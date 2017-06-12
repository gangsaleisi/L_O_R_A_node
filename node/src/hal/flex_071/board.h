/*
    _/_/_/    _/_/_/    _/_/_/  _/_/_/  _/      _/    _/_/_/  _/    _/  _/_/_/_/
   _/    _/    _/    _/          _/    _/_/    _/  _/        _/    _/  _/
  _/_/_/      _/      _/_/      _/    _/  _/  _/  _/  _/_/  _/_/_/_/  _/_/_/
 _/    _/    _/          _/    _/    _/    _/_/  _/    _/  _/    _/  _/
_/    _/  _/_/_/  _/_/_/    _/_/_/  _/      _/    _/_/_/  _/    _/  _/
    (C)2015 RisingHF, all rights reserved.

Description: loramac-node board dependent definitions

*/
#ifndef __BOARD_H__
#define __BOARD_H__

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "stm32l0xx.h"
#include "stm32l0xx_hal.h"
#include "utilities.h"
#include "timer.h"
#include "delay.h"
#include "gpio.h"
#include "adc.h"
#include "spi.h"
#include "i2c.h"
#include "flash.h"
#include "radio.h"
#include "sx1276/sx1276.h"
#include "rtc-board.h"
#include "timer-board.h"
#include "sx1276-board.h"

/*!
 * NULL definition
 */
#ifndef NULL
    #define NULL                                    ( ( void * )0 )
#endif

/*!
 * Generic definition
 */
#ifndef SUCCESS
#define SUCCESS                                     1
#endif

#ifndef FAIL
#define FAIL                                        0
#endif

/*!
 * Unique Devices IDs register set ( STM32L1xxx )
 */
#define         ID1                                 ( 0x1FF80050 )
#define         ID2                                 ( 0x1FF80054 )
#define         ID3                                 ( 0x1FF80064 )

/*!
 * Random seed generated using the MCU Unique ID
 */
#define RAND_SEED                                   ( ( *( uint32_t* )ID1 ) ^ \
                                                      ( *( uint32_t* )ID2 ) ^ \
                                                      ( *( uint32_t* )ID3 ) )

/*!
 * Board MCU pins definitions
 */
#if defined(STM32L052xx)
#define RADIO_RESET                                 PB_11

#define RADIO_DIO_0                                 PB_10
#define RADIO_DIO_1                                 PB_2
#define RADIO_DIO_2                                 PB_0
#define RADIO_DIO_3                                 PB_1
#define RADIO_DIO_4                                 NC
#define RADIO_DIO_5                                 NC

#define RADIO_ANT_SWITCH_RXTX1	                    PA_1
#define RADIO_ANT_SWITCH_RXTX2                      PA_2

#elif defined(STM32L071xx)

#define RADIO_RESET                                 PB_1

#define RADIO_DIO_0                                 PA_8
#define RADIO_DIO_1                                 PA_9
#define RADIO_DIO_2                                 PA_10
#define RADIO_DIO_3                                 PA_11
#define RADIO_DIO_4                                 PA_12
#define RADIO_DIO_5                                 NC

#define RADIO_ANT_SWITCH_RXTX1	                    PB_4
#define RADIO_ANT_SWITCH_RXTX2                      PB_5
#endif

#define BAT_LEVEL_PIN                               PA_1
#define BAT_LEVEL_CHANNEL                           ADC_CHANNEL_1

#define I2C_SCL                                     PB_6
#define I2C_SDA                                     PB_7

#define RADIO_MOSI                                  PA_7
#define RADIO_MISO                                  PA_6
#define RADIO_SCLK                                  PA_5
#define RADIO_NSS                                   PA_4

#define SWDIO                                       PA_13
#define SWCLK                                       PA_14

#define LED                                         PA_0
/*!
 * \brief Initializes the target board peripherals.
 */
void BoardInitMcu( void );

/*!
 * \brief Initializes the boards peripherals.
 */
void BoardInitPeriph( void );

/*!
 * \brief De-initializes the target board peripherals to decrease power
 *        consumption.
 */
void BoardDeInitMcu( void );

/*!
 * \brief Measure the Battery level
 *
 * \retval value  battery level ( 0: very low, 254: fully charged )
 */
uint8_t BoardMeasureBatterieLevel( void );

/*!
 * \brief Gets the board 64 bits unique ID
 *
 * \param [IN] id Pointer to an array that will contain the Unique ID
 */
void BoardGetUniqueId( uint8_t *id );

void MX_GPIO_Init(void);
void MX_GPIO_DeInit(void);

float get_sensor_value( void );
extern uint16_t Flash_If_Init(void);
extern uint16_t Flash_If_Erase(uint32_t Add);
extern uint16_t Flash_If_Erase_Page(uint32_t Add);
extern uint16_t Flash_If_Write(uint8_t *src, uint8_t *dest, uint32_t Len);
extern uint8_t *Flash_If_Read(uint8_t *src, uint8_t *dest, uint32_t Len);
extern uint16_t Flash_If_DeInit(void);
#endif // __BOARD_H__
