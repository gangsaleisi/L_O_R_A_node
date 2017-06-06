/*
  ______                              _
 / _____)             _              | |
( (____  _____ ____ _| |_ _____  ____| |__
 \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 _____) ) ____| | | || |_| ____( (___| | | |
(______/|_____)_|_|_| \__)_____)\____)_| |_|
    (C)2016 Semtech

Description: Board UART driver implementation

License: Revised BSD License, see LICENSE.TXT file include in the project

Maintainer: Miguel Luis and Gregory Cristian
*/
#include "board.h"

#include "uart-board.h"

UART_HandleTypeDef UartHandle;

extern DMA_HandleTypeDef hdma_usart2_rx;
extern UART_HandleTypeDef huart2;
void UartMcuDeInit( Uart_t *obj )
{
    __HAL_RCC_USART2_FORCE_RESET( );
    __HAL_RCC_USART2_RELEASE_RESET( );
    __HAL_RCC_USART2_CLK_DISABLE( );

    GpioInit( &obj->Tx, obj->Tx.pin, PIN_ANALOGIC, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
    GpioInit( &obj->Rx, obj->Rx.pin, PIN_ANALOGIC, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
}

uint8_t UartMcuPutChar( Uart_t *obj, uint8_t data )
{
	HAL_UART_Transmit(&UartHandle, "11", 2, 5);
	#if 0
    BoardDisableIrq( );
    TxData = data;

    if( IsFifoFull( &obj->FifoTx ) == false )
    {
        FifoPush( &obj->FifoTx, TxData );

        // Trig UART Tx interrupt to start sending the FIFO contents.
        __HAL_UART_ENABLE_IT( &UartHandle, UART_IT_TC );

        BoardEnableIrq( );
        return 0; // OK
    }
    BoardEnableIrq( );
		#endif
    return 1; // Busy
}

extern uint8_t rx_buffer[21];
extern uint8_t tx_buffer[21];
#define BUFF_SIXE_1       20
void my_printf(uint8_t *buff)
{
  HAL_UART_Transmit(&UartHandle, buff, strlen((char const *)buff), 5);
}

void HAL_UART_TxCpltCallback( UART_HandleTypeDef *handle )
{
#if 0
    if( IsFifoEmpty( &Uart1.FifoTx ) == false )
    {
        TxData = FifoPop( &Uart1.FifoTx );
        //  Write one byte to the transmit data register
        HAL_UART_Transmit_IT( &UartHandle, &TxData, 1 );
    }

    if( Uart1.IrqNotify != NULL )
    {
        Uart1.IrqNotify( UART_NOTIFY_TX );
    }
#endif
}
extern uint8_t receive_flag;
void HAL_UART_RxCpltCallback( UART_HandleTypeDef *handle )
{
#if 0
    if( IsFifoFull( &Uart1.FifoRx ) == false )
    {
        // Read one byte from the receive data register
        FifoPush( &Uart1.FifoRx, RxData );
    }

    if( Uart1.IrqNotify != NULL )
    {
        Uart1.IrqNotify( UART_NOTIFY_RX );
    }
#endif
    receive_flag = 1;
    memcpy1(tx_buffer, rx_buffer, sizeof(rx_buffer));
    HAL_UART_Receive_DMA( &huart2, rx_buffer, BUFF_SIXE_1 );
}

void HAL_UART_ErrorCallback( UART_HandleTypeDef *handle )
{
   // HAL_UART_Receive_IT( &UartHandle, &RxData, 1 );
}
#if 0
void USART2_IRQHandler( void )
{
    HAL_UART_IRQHandler( &UartHandle );
}
#endif