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
Gpio_t Led;
Gpio_t UnUsed;
I2C_HandleTypeDef hi2c1;
UART_HandleTypeDef huart2;
USART_RECEIVETYPE UsartType1; 
DMA_HandleTypeDef hdma_usart2_rx;
DMA_HandleTypeDef hdma_usart2_tx;
static void BoardUnusedIoInit( void );

static void MX_GPIO_Init(void);
#define UART_FIFO_TX_SIZE                                8
#define UART_FIFO_RX_SIZE                                256
uint8_t UartTxBuffer[UART_FIFO_TX_SIZE];
uint8_t UartRxBuffer[UART_FIFO_RX_SIZE];
extern void MX_DMA_Init(void);
extern void MX_USART2_UART_Init(void);

static void BoardUnusedIoInit( void );
#ifdef TMP006
static void MX_GPIO_Init(void);
#endif
#define HSI_TIMEOUT_VALUE          ((uint32_t)0x1FFFF)  /* 100 ms */
#define PLL_TIMEOUT_VALUE          ((uint32_t)0x1FFFF)  /* 100 ms */
#define CLOCKSWITCH_TIMEOUT_VALUE  ((uint32_t)0x1FFFF)  /* 5 s    */

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

}

void BoardInitMcu( void )
{
    if( McuInitialized == false )
    {
        SystemClock_Config();
        SystemCoreClockUpdate();
        
        GpioInit( &Led, LED, PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, 1 );
        
        // Disable Systick
        SysTick->CTRL  &= ~SysTick_CTRL_TICKINT_Msk;    // Systick IRQ off
        SCB->ICSR |= SCB_ICSR_PENDSTCLR_Msk;            // Clear SysTick Exception pending flag

        SpiInit( &SX1276.Spi, RADIO_MOSI, RADIO_MISO, RADIO_SCLK, NC );
        SX1276IoInit( );

        TimerSetLowPowerEnable( true );
        RtcInit( );

        BoardUnusedIoInit( );
       
        
        McuInitialized = true;
    }
    MX_GPIO_Init();
    MX_DMA_Init();
    MX_USART2_UART_Init();
    HAL_UART_Receive_DMA(&huart2, UsartType1.usartDMA_rxBuf, RECEIVELEN);  
    __HAL_UART_ENABLE_IT(&huart2, UART_IT_IDLE);
#if defined( TMP006 )
    
    MX_I2C1_Init();
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
#if defined( TMP006 )
    MX_I2C1_DeInit();
#endif
    AdcDeInit( &Adc );
    Flash_If_DeInit();
    SpiDeInit( &SX1276.Spi );
    SX1276IoDeInit( );

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
    GpioInit( &UnUsed, UNUSED_1, PIN_INPUT, PIN_OPEN_DRAIN, PIN_NO_PULL, 0 );
    GpioInit( &UnUsed, UNUSED_2, PIN_INPUT, PIN_OPEN_DRAIN, PIN_NO_PULL, 0 );
    GpioInit( &UnUsed, UNUSED_3, PIN_INPUT, PIN_OPEN_DRAIN, PIN_NO_PULL, 0 );
    GpioInit( &UnUsed, UNUSED_4, PIN_INPUT, PIN_OPEN_DRAIN, PIN_NO_PULL, 0 );
    GpioInit( &UnUsed, UNUSED_5, PIN_INPUT, PIN_OPEN_DRAIN, PIN_NO_PULL, 0 );
    GpioInit( &UnUsed, UNUSED_6, PIN_INPUT, PIN_OPEN_DRAIN, PIN_NO_PULL, 0 );
    GpioInit( &UnUsed, UNUSED_7, PIN_INPUT, PIN_OPEN_DRAIN, PIN_NO_PULL, 0 );
    GpioInit( &UnUsed, UNUSED_8, PIN_INPUT, PIN_OPEN_DRAIN, PIN_NO_PULL, 0 );
    GpioInit( &UnUsed, UNUSED_9, PIN_INPUT, PIN_OPEN_DRAIN, PIN_NO_PULL, 0 );
    GpioInit( &UnUsed, UNUSED_10, PIN_INPUT, PIN_OPEN_DRAIN, PIN_NO_PULL, 0 );
    GpioInit( &UnUsed, UNUSED_11, PIN_INPUT, PIN_OPEN_DRAIN, PIN_NO_PULL, 0 );
    GpioInit( &UnUsed, UNUSED_12, PIN_INPUT, PIN_OPEN_DRAIN, PIN_NO_PULL, 0 );
    GpioInit( &UnUsed, UNUSED_13, PIN_INPUT, PIN_OPEN_DRAIN, PIN_NO_PULL, 0 );
}

static void MX_GPIO_Init(void)
{
  __HAL_RCC_GPIOA_CLK_ENABLE();
  /* GPIO Ports Clock Enable */
  #ifdef TMP006
  __HAL_RCC_GPIOB_CLK_ENABLE();
  #endif

}
