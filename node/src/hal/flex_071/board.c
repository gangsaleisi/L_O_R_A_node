/*
    _/_/_/    _/_/_/    _/_/_/  _/_/_/  _/      _/    _/_/_/  _/    _/  _/_/_/_/
   _/    _/    _/    _/          _/    _/_/    _/  _/        _/    _/  _/
  _/_/_/      _/      _/_/      _/    _/  _/  _/  _/  _/_/  _/_/_/_/  _/_/_/
 _/    _/    _/          _/    _/    _/    _/_/  _/    _/  _/    _/  _/
_/    _/  _/_/_/  _/_/_/    _/_/_/  _/      _/    _/_/_/  _/    _/  _/
    (C)2015 RisingHF, all rights reserved.

Description: loramac-node board level functions

*/
#include "board.h"

static bool McuInitialized = false;
Adc_t Adc;
I2c_t I2c;
Gpio_t Led;

static void BoardUnusedIoInit( void );

#define HSI_TIMEOUT_VALUE          ((uint32_t)0x1FFFF)  /* 100 ms */
#define PLL_TIMEOUT_VALUE          ((uint32_t)0x1FFFF)  /* 100 ms */
#define CLOCKSWITCH_TIMEOUT_VALUE  ((uint32_t)0x1FFFF)  /* 5 s    */

#define UART_FIFO_TX_SIZE                                8
#define UART_FIFO_RX_SIZE                                256

uint8_t UartTxBuffer[UART_FIFO_TX_SIZE];
uint8_t UartRxBuffer[UART_FIFO_RX_SIZE];
extern void MX_GPIO_Init(void);
extern void MX_DMA_Init(void);
extern void MX_USART2_UART_Init(void);
void SystemClock_Config(void)
{
    uint32_t tickstart;

    RCC->APB1ENR |= (RCC_APB1ENR_PWREN);
    while(PWR->CSR & PWR_CSR_VOSF);
    PWR->CR = (PWR->CR & ~(PWR_CR_VOS)) | PWR_CR_VOS_0;
    while(PWR->CSR & PWR_CSR_VOSF);

    RCC->CR |= RCC_CR_HSION | RCC_CR_HSIDIVEN;
    tickstart = 0;
    while ((RCC->CR & (RCC_CR_HSIRDY |RCC_CR_HSIDIVF)) != (RCC_CR_HSIRDY |RCC_CR_HSIDIVF)) { /* (4) */
        tickstart++;
        if (tickstart > HSI_TIMEOUT_VALUE) {
            while(1);
            //return;
        }
    }
    RCC->CFGR |= RCC_CFGR_PLLSRC_HSI | RCC_CFGR_PLLMUL8 | RCC_CFGR_PLLDIV2; // 16MHz
    //RCC->CFGR |= RCC_CFGR_PLLSRC_HSI | RCC_CFGR_PLLMUL8 | RCC_CFGR_PLLDIV2; // 32MHz

    RCC->CR |= RCC_CR_PLLON;
    tickstart = 0;
    while ((RCC->CR & RCC_CR_PLLRDY)  == 0) {
        tickstart++;
        if (tickstart > PLL_TIMEOUT_VALUE) {
            while(1);
        }
    }
    RCC->CFGR |= RCC_CFGR_SW_PLL;
    tickstart = 0;
    while ((RCC->CFGR & RCC_CFGR_SWS_PLL)  == 0) {
        tickstart++;
        if (tickstart > CLOCKSWITCH_TIMEOUT_VALUE) {
            while(1);
        }
    }
}

void BoardInitPeriph( void )
{
#ifdef MP9800
    Mcp98Init();
#endif
}

void BoardInitMcu( void )
{
    if( McuInitialized == false )
    {
        SystemClock_Config();
        SystemCoreClockUpdate();
        
        GpioInit( &Led, LED, PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, 1 );
        GpioWrite( &Led, 0 );
        // Disable Systick
        SysTick->CTRL  &= ~SysTick_CTRL_TICKINT_Msk;    // Systick IRQ off
        SCB->ICSR |= SCB_ICSR_PENDSTCLR_Msk;            // Clear SysTick Exception pending flag

        SpiInit( &SX1276.Spi, RADIO_MOSI, RADIO_MISO, RADIO_SCLK, NC );
        SX1276IoInit( );

        TimerSetLowPowerEnable( true );
        RtcInit( );

        BoardUnusedIoInit( );
        //FifoInit( &Uart1.FifoTx, UartTxBuffer, UART_FIFO_TX_SIZE );
        //FifoInit( &Uart1.FifoRx, UartRxBuffer, UART_FIFO_RX_SIZE );
        // Configure your terminal for 8 Bits data (7 data bit + 1 parity bit), no parity and no flow ctrl
        //UartInit( &Uart1, UART_2, UART_TX, UART_RX );
        //UartConfig( &Uart1, RX_TX, 9600, UART_8_BIT, UART_1_STOP_BIT, NO_PARITY, NO_FLOW_CTRL );

        
        McuInitialized = true;
    }
#ifdef MCP9800
    I2cInit( &I2c, I2C_SCL, I2C_SDA );
#endif
    AdcInit( &Adc, BAT_LEVEL_PIN );
    Flash_If_Init();
#if USE_DEBUGGER
    /* Enable debug under stop mode */
    RCC->APB2ENR |= RCC_APB2ENR_DBGMCUEN;
    DBGMCU->CR |= DBGMCU_CR_DBG_STOP | DBGMCU_CR_DBG_STANDBY | DBGMCU_CR_DBG_SLEEP;
    DBGMCU->APB1FZ |= DBGMCU_APB1_FZ_DBG_RTC_STOP;
#endif
}

void BoardDeInitMcu( void )
{
#ifdef MCP9800
    I2cDeInit( &I2c );
#endif
    AdcDeInit( &Adc );
    Flash_If_DeInit();
    SpiDeInit( &SX1276.Spi );
    SX1276IoDeInit( );
    //UartDeInit( &Uart1 );
    McuInitialized = false;

#ifndef USE_DEBUGGER
    {
        Gpio_t Swdio;
        Gpio_t Swclk;
        GpioInit( &Swdio, SWDIO, PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
        GpioInit( &Swclk, SWCLK, PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
    }
#endif
}

void BoardGetUniqueId( uint8_t *id )
{
    id[0] = ( ( *( uint32_t* )ID1 )+ ( *( uint32_t* )ID3 ) ) >> 24;
    id[1] = ( ( *( uint32_t* )ID1 )+ ( *( uint32_t* )ID3 ) ) >> 16;
    id[2] = ( ( *( uint32_t* )ID1 )+ ( *( uint32_t* )ID3 ) ) >> 8;
    id[3] = ( ( *( uint32_t* )ID1 )+ ( *( uint32_t* )ID3 ) );
    id[4] = ( ( *( uint32_t* )ID2 ) ) >> 24;
    id[5] = ( ( *( uint32_t* )ID2 ) ) >> 16;
    id[6] = ( ( *( uint32_t* )ID2 ) ) >> 8;
    id[7] = ( ( *( uint32_t* )ID2 ) );
}

uint16_t BoardGetPowerSupply( void )
{
    return 0xFFFF;
}

uint8_t BoardMeasureBatterieLevel( void )
{
    return 254;
}

static void BoardUnusedIoInit( void )
{

}
static uint8_t IrqNestLevel = 0;

void BoardDisableIrq( void )
{
    __disable_irq( );
    IrqNestLevel++;
}

void BoardEnableIrq( void )
{
    IrqNestLevel--;
    if( IrqNestLevel == 0 )
    {
        __enable_irq( );
    }
}

UART_HandleTypeDef huart2;
DMA_HandleTypeDef hdma_usart2_rx;
DMA_HandleTypeDef hdma_usart2_tx;
USART_RECEIVETYPE UsartType1; 
uint16_t eui[4] = {0};
void GetDevEui(void)
{
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_USART2_UART_Init();
  HAL_UART_Receive_DMA(&huart2, UsartType1.usartDMA_rxBuf, RECEIVELEN);  
  __HAL_UART_ENABLE_IT(&huart2, UART_IT_IDLE);
  while(1)
  {

    if(UsartType1.receive_flag)//如果产生了空闲中断  
    {
      UsartType1.receive_flag=0;//清零标记  
      if(strncmp(UsartType1.usartDMA_rxBuf, FLASH_HEAD, 4) == 0)
      {
        eui[0] = ( uint16_t )UsartType1.usartDMA_rxBuf[5];
        eui[0] |= ( uint16_t )UsartType1.usartDMA_rxBuf[4] << 8;
        eui[1] = ( uint16_t )UsartType1.usartDMA_rxBuf[7];
        eui[1] |= ( uint16_t )UsartType1.usartDMA_rxBuf[6] << 8;
        eui[2] = ( uint16_t )UsartType1.usartDMA_rxBuf[9];
        eui[2] |= ( uint16_t )UsartType1.usartDMA_rxBuf[8] << 8;
        eui[3] = ( uint16_t )UsartType1.usartDMA_rxBuf[11];
        eui[3] |= ( uint16_t )UsartType1.usartDMA_rxBuf[10] << 8;
        SendData("OK\n", 3);
        
        //break;
      }
      else
      {
        SendData("ERROR,add FLEX before ID\n", 25);
      }
    }

  }
}
