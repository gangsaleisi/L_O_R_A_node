/*
    _/_/_/    _/_/_/    _/_/_/  _/_/_/  _/      _/    _/_/_/  _/    _/  _/_/_/_/
   _/    _/    _/    _/          _/    _/_/    _/  _/        _/    _/  _/
  _/_/_/      _/      _/_/      _/    _/  _/  _/  _/  _/_/  _/_/_/_/  _/_/_/
 _/    _/    _/          _/    _/    _/    _/_/  _/    _/  _/    _/  _/
_/    _/  _/_/_/  _/_/_/    _/_/_/  _/      _/    _/_/_/  _/    _/  _/
    (C)2015 RisingHF, all rights reserved.

Description: MCU RTC timer and low power modes management

*/
#include <math.h>
#include "board.h"
#include "timer-board.h"

/*!
 * Hardware Time base in us
 */
#define HW_TIMER_TIME_BASE                              100 //us

/*!
 * Hardware Timer tick counter
 */
volatile uint64_t TimerTickCounter = 1;

/*!
 * Saved value of the Tick counter at the start of the next event
 */
uint64_t TimerTickCounterContext = 0;

/*!
 * Value trigging the IRQ
 */
volatile uint64_t TimeoutCntValue = 0;

/*!
 * Increment the Hardware Timer tick counter
 */
void TimerIncrementTickCounter( void );

/*!
 * Counter used for the Delay operations
 */
volatile uint32_t TimerDelayCounter = 0;

/*!
 * Retunr the value of the counter used for a Delay
 */
uint32_t TimerHwGetDelayValue( void );

/*!
 * Increment the value of TimerDelayCounter
 */
void TimerIncrementDelayCounter( void );


void TimerHwInit( void )
{

}

void TimerHwDeInit( void )
{

}

uint32_t TimerHwGetMinimumTimeout( void )
{

    return 0;

}

void TimerHwStart( uint32_t val )
{

}

void TimerHwStop( void )
{

}

void TimerHwDelayMs( uint32_t delay )
{

}

uint64_t TimerHwGetElapsedTime( void )
{
    return 0;
}

uint64_t TimerHwGetTimerValue( void )
{
    return 0;
}

TimerTime_t TimerHwGetTime( void )
{
    return 0;
}

uint32_t TimerHwGetDelayValue( void )
{
    return 0;
}

void TimerIncrementTickCounter( void )
{

}

void TimerIncrementDelayCounter( void )
{

}

/*!
 * Timer IRQ handler
 */
void TIM2_IRQHandler( void )
{

}

/*!
 * Timer IRQ handler
 */
void TIM3_IRQHandler( void )
{

}

void TimerHwEnterLowPowerStopMode( void )
{

}
