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
#include <time.h>
#include "board.h"
#include "rtc-board.h"

/*!
 * RTC Time base in us
 */
#define RTC_ALARM_TIME_BASE                             122.07

/*!
 * MCU Wake Up Time
 */
#define MCU_WAKE_UP_TIME                                3400

/*!
 * \brief Start the Rtc Alarm (time base 1s)
 */
static void RtcStartWakeUpAlarm( uint32_t timeoutValue );

/*!
 * \brief Read the MCU internal Calendar value
 *
 * \retval Calendar value
 */
static uint64_t RtcGetCalendarValue( void );

/*!
 * \brief Clear the RTC flags and Stop all IRQs
 */
static void RtcClearStatus( void );

/*!
 * \brief Indicates if the RTC is already Initalized or not
 */
static bool RtcInitalized = false;

/*!
 * \brief Flag to indicate if the timestamps until the next event is long enough
 * to set the MCU into low power mode
 */
static bool RtcTimerEventAllowsLowPower = false;

/*!
 * \brief Flag to disable the LowPower Mode even if the timestamps until the
 * next event is long enough to allow Low Power mode
 */
static bool LowPowerDisableDuringTask = false;

/*!
 * Keep the value of the RTC timer when the RTC alarm is set
 */
static uint64_t RtcTimerContext = 0;

/*!
 * Number of seconds in a minute
 */
static const uint8_t SecondsInMinute = 60;

/*!
 * Number of seconds in an hour
 */
static const uint16_t SecondsInHour = 3600;

/*!
 * Number of seconds in a day
 */
static const uint32_t SecondsInDay = 86400;

/*!
 * Number of hours in a day
 */
static const uint8_t HoursInDay = 24;

/*!
 * Number of days in a standard year
 */
static const uint16_t DaysInYear = 365;

/*!
 * Number of days in a leap year
 */
static const uint16_t DaysInLeapYear = 366;

/*!
 * Number of days in a century
 */
static const double DaysInCentury = 36524.219;

/*!
 * Number of days in each month on a normal year
 */
static const uint8_t DaysInMonth[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

/*!
 * Number of days in each month on a leap year
 */
static const uint8_t DaysInMonthLeapYear[] = { 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

/*!
 * Hold the previous year value to detect the turn of a century
 */
static uint8_t PreviousYear = 0;

/*!
 * Century counter
 */
static uint8_t Century = 0;


#define RTC_TR_RESERVED_MASK    ((uint32_t)0x007F7F7F)
#define RTC_DR_RESERVED_MASK    ((uint32_t)0x00FFFF3F)
#define RTC_RSF_MASK            ((uint32_t)0xFFFFFF5F)
#define SYNCHRO_TIMEOUT          ((uint32_t) 0x00008000)

typedef struct
{
  uint8_t RTC_Hours;
  uint8_t RTC_Minutes;
  uint8_t RTC_Seconds;
  uint8_t RTC_Month;
  uint8_t RTC_Date;
  uint8_t RTC_Year;
}RTC_TimeTypeDef;

void RTC_WaitForSynchro(void)
{
    __IO uint32_t synchrocounter = 0;
    uint32_t synchrostatus = 0x00;

    /* Disable the write protection for RTC registers */
    RTC->WPR = 0xCA;
    RTC->WPR = 0x53;

    /* Clear RSF flag */
    RTC->ISR &= (uint32_t)RTC_RSF_MASK;

    /* Wait the registers to be synchronised */
    do
    {
        synchrostatus = RTC->ISR & RTC_ISR_RSF;
        synchrocounter++;
    } while((synchrocounter != SYNCHRO_TIMEOUT) && (synchrostatus == 0x00));

    /* Enable the write protection for RTC registers */
    RTC->WPR = 0xFF;
}

void RtcInit( void )
{
    if( RtcInitalized == false )
    {
        /** DBP enable */
        RCC->APB1ENR |= RCC_APB1ENR_PWREN;
        PWR->CR |= PWR_CR_DBP;

        /** reset RTC */
        RCC->CSR |= RCC_CSR_RTCRST;
        RCC->CSR &= ~RCC_CSR_RTCRST;

        /** enable LSE */
        RCC->CSR |= RCC_CSR_LSEON; /* (1) */
        while((RCC->CSR & RCC_CSR_LSERDY)!=RCC_CSR_LSERDY) /* (2) */
        {
        /* add time out here for a robust application */
        }

        /** Enable RTC, use LSE as clock */
        RCC->CSR = (RCC->CSR & ~RCC_CSR_RTCSEL) | RCC_CSR_RTCEN | RCC_CSR_RTCSEL_0; /* (5) */

        //RCC->APB1ENR &=~ RCC_APB1ENR_PWREN; /* (7) */

        /* Configure RTC */
        /* (7) Write access for RTC regsiters */
        /* (8) Disable alarm A to modify it */
        /* (9) Wait until it is allow to modify alarm A value */
        /* (10) Modify alarm A mask to have an interrupt each 1Hz */
        /* (11) Enable alarm A and alarm A interrupt */
        /* (12) Disable write access */
        RTC->WPR = 0xCA; /* (7) */
        RTC->WPR = 0x53; /* (7) */
        RTC->CR &=~ RTC_CR_ALRAE; /* (8) */
        while((RTC->ISR & RTC_ISR_ALRAWF) != RTC_ISR_ALRAWF) /* (9) */
        {
        /* add time out here for a robust application */
        }
        RTC->ALRMAR = 0; /* (10) */
        RTC->CR = RTC_CR_ALRAIE | RTC_CR_ALRAE; /* (11) */
        RTC->WPR = 0xFE; /* (12) */
        RTC->WPR = 0x64; /* (12) */

        /* Configure exti and nvic for RTC IT */
        /* (13) unmask line 17 */
        /* (14) Rising edge for line 17 */
        /* (15) Set priority */
        /* (16) Enable RTC_IRQn */
        EXTI->IMR |= EXTI_IMR_IM17; /* (13) */
        EXTI->RTSR |= EXTI_RTSR_TR17; /* (14) */
        NVIC_SetPriority(RTC_IRQn, 0); /* (15) */
        NVIC_EnableIRQ(RTC_IRQn); /* (16) */

        /* RTC init mode */
        /* Configure RTC */
        /* (1) Write access for RTC registers */
        /* (2) Enable init phase */
        /* (3) Wait until it is allow to modify RTC register values */
        /* (4) set prescaler, 40kHz/64 => 625 Hz, 625Hz/625 => 1Hz */
        /* (5) Set time to 0 */
        /* (6) Disable init phase */
        /* (7) Disable write access for RTC registers */
        RTC->WPR = 0xCA; /* (1) */
        RTC->WPR = 0x53; /* (1) */
        RTC->ISR |= RTC_ISR_INIT; /* (2) */
        while((RTC->ISR & RTC_ISR_INITF)!=RTC_ISR_INITF) /* (3) */
        {
        /* add time out here for a robust application */
        }
        RTC->PRER = 0x00010001; /* (4) */
        RTC->TR = 0; /* (5) */
        RTC->DR = 0x00002101;
        RTC->CR = (RTC->CR & ~RTC_CR_FMT);
        RTC->ISR &=~ RTC_ISR_INIT; /* (6) */
        RTC->WPR = 0xFE; /* (7) */
        RTC->WPR = 0x64; /* (7) */

        RtcInitalized = true;
    }
}

void RtcStopTimer( void )
{
    RtcClearStatus( );
}

uint32_t RtcGetMinimumTimeout( void )
{
    return( (uint32_t)ceil( 10 * RTC_ALARM_TIME_BASE ) );
}

void RtcSetTimeout( uint32_t timeout )
{
    uint32_t timeoutValue = 0;

    timeoutValue = timeout;

    if( timeoutValue < ( 10 * RTC_ALARM_TIME_BASE ) )
    {
        timeoutValue = (uint32_t)(10.0 * RTC_ALARM_TIME_BASE);
    }

    if( timeoutValue < 55000 )
    {
        // we don't go in Low Power mode for delay below 50ms (needed for LEDs)
        RtcTimerEventAllowsLowPower = false;
    }
    else
    {
        RtcTimerEventAllowsLowPower = true;
    }

    if( ( LowPowerDisableDuringTask == false ) && ( RtcTimerEventAllowsLowPower == true ) )
    {
        timeoutValue = timeoutValue - MCU_WAKE_UP_TIME;
    }

    RtcStartWakeUpAlarm( timeoutValue );
}


uint32_t RtcGetTimerElapsedTime( void )
{
    uint64_t CalendarValue = 0;

    CalendarValue = RtcGetCalendarValue( );

    return( ( uint32_t )( ceil ( ( ( CalendarValue - RtcTimerContext ) + 2 ) * RTC_ALARM_TIME_BASE ) ) );
}

uint64_t RtcGetTimerValue( void )
{
    uint64_t CalendarValue = 0;

    CalendarValue = RtcGetCalendarValue( );

    return (uint64_t)( ( CalendarValue + 2 ) * RTC_ALARM_TIME_BASE );
}

static void RtcClearStatus( void )
{
    RTC->WPR = 0xCA; /* (7) */
    RTC->WPR = 0x53; /* (7) */
    RTC->CR &=~ RTC_CR_ALRAE; /* (8) */
    while((RTC->ISR & RTC_ISR_ALRAWF) != RTC_ISR_ALRAWF) /* (9) */
    {
        /* add time out here for a robust application */
    }
    RTC->CR &= ~(RTC_CR_ALRAIE | RTC_CR_ALRAE); /* (11) */
    RTC->WPR = 0xFE; /* (12) */
    RTC->WPR = 0x64; /* (12) */
}

void RtcEnterLowPowerStopMode( void )
{
    if( ( LowPowerDisableDuringTask == false ) && ( RtcTimerEventAllowsLowPower == true ) )
    {
        /** disable irq */
        __disable_irq( );

        /** Deinit board */
        BoardDeInitMcu( );

        /* Enable PWR clock */
        RCC->APB1ENR |= (uint32_t)(RCC_APB1ENR_PWREN);

        /* Set SLEEPDEEP bit of Cortex System Control Register */
        SCB->SCR |= SCB_SCR_SLEEPDEEP_Msk;

        /* Clear WakeUp flag before enter sleep mode */
        PWR->CR |= (uint32_t)(PWR_CR_CWUF);

        /* Disable the Power Voltage Detector */
        PWR->CR &= (uint32_t)(~PWR_CR_PVDE);

        /* Set MCU in ULP (Ultra Low Power) */
        PWR->CR |= PWR_CR_ULP;

        /*Enable fast wakeUp*/
        PWR->CR |= (uint32_t)(PWR_CR_FWU);

        /* Enter Stop mode(not standby mode) when mcu enters deepsleep */
        PWR->CR &= (uint32_t)(~PWR_CR_PDDS);

        /* Regulator is in low power mode */
        PWR->CR |= PWR_CR_LPSDSR;

        /* Request Wait For Interrupt */
        __WFI();

        /** enable irq */
        __enable_irq( );

        /* Reset SLEEPDEEP bit of Cortex System Control Register */
        SCB->SCR &= (uint32_t)~((uint32_t)SCB_SCR_SLEEPDEEP_Msk);
    }
}

#define HSI_TIMEOUT_VALUE          ((uint32_t)0x1FFFF)  /* 100 ms */
#define PLL_TIMEOUT_VALUE          ((uint32_t)0x1FFFF)  /* 100 ms */
#define CLOCKSWITCH_TIMEOUT_VALUE  ((uint32_t)0x1FFFF) /* 5 s    */

void RtcRecoverMcuStatus( void )
{
    uint32_t tickstart;

    if( TimerGetLowPowerEnable( ) == true )
    {
        if( ( LowPowerDisableDuringTask == false ) && ( RtcTimerEventAllowsLowPower == true ) )
        {
            /** disable irq */
            __disable_irq( );

            /** reconfigure system clock */
            /** Enable HSI, wait until it is ready. */
            RCC->CR |= RCC_CR_HSION | RCC_CR_HSIDIVEN; /* (3) */
            tickstart = 0;
            while ((RCC->CR & (RCC_CR_HSIRDY |RCC_CR_HSIDIVF)) != (RCC_CR_HSIRDY |RCC_CR_HSIDIVF)) { /* (4) */
                tickstart++;
                if (tickstart > HSI_TIMEOUT_VALUE) {
                    while(1);
                    //return;
                }
            }

            /** Enable PLL */
            RCC->CFGR |= RCC_CFGR_PLLSRC_HSI | RCC_CFGR_PLLMUL8 | RCC_CFGR_PLLDIV2; /* (5) */
            RCC->CR |= RCC_CR_PLLON;
            tickstart = 0;
            while ((RCC->CR & RCC_CR_PLLRDY)  == 0) {
                tickstart++;
                if (tickstart > PLL_TIMEOUT_VALUE) {
                    while(1);
                    //return;
                }
            }

            /** Switch system clock to PLL */
            RCC->CFGR |= RCC_CFGR_SW_PLL;
            tickstart = 0;
            while ((RCC->CFGR & RCC_CFGR_SWS_PLL)  == 0) {
                tickstart++;
                if (tickstart > CLOCKSWITCH_TIMEOUT_VALUE) {
                    while(1);
                    //return;
                }
            }

            /* Enable PWR Clock */
            RCC->APB1ENR |= (RCC_APB1ENR_PWREN);

            /* WeakUp flag must be cleared before enter sleep again */
            PWR->CR |= (uint32_t)(PWR_CR_CWUF);
            while( (PWR->CSR & PWR_CSR_WUF) == PWR_CSR_WUF );

            /* Enable the Power Voltage Detector */
            PWR->CR |= (uint32_t)(PWR_CR_PVDE);

            /* Exit MCU in ULP (Ultra Low Power) */
            PWR->CR &= (uint32_t)~PWR_CR_ULP;

            //RCC->APB1ENR &= (uint32_t)(~RCC_APB1ENR_PWREN);

            /** reinit board */
            BoardInitMcu();

            /** enable irq */
            __enable_irq( );
        }
    }
}

/*!
* \brief RTC IRQ Handler on the RTC Alarm
*/
void RTC_IRQHandler( void )
{
    /* Check alarm A flag */
    if((RTC->ISR & (RTC_ISR_ALRAF)) == (RTC_ISR_ALRAF))
    {
        RtcRecoverMcuStatus( );

        TimerIrqHandler( );

        RTC->ISR &=~ RTC_ISR_ALRAF; /* clear flag */
        EXTI->PR |= EXTI_PR_PR17; /* clear exti line 17 flag */
    }
    else
    {
        /**error*/
        while(1);
    }
}

void BlockLowPowerDuringTask( bool status )
{
    if( status == true )
    {
        RtcRecoverMcuStatus( );
    }
    LowPowerDisableDuringTask = status;
}

void RtcDelayMs( uint32_t delay )
{
    uint64_t delayValue = 0;
    uint64_t timeout = 0;

    delayValue = ( uint64_t )( delay * 1000 );

    // Wait delay ms
    timeout = RtcGetTimerValue( );
    while( ( ( RtcGetTimerValue( ) - timeout ) ) < delayValue )
    {
        __NOP( );
    }
}

static uint8_t Bcd2ToByte(uint8_t Value)
{
    uint8_t tmp = 0;
    tmp = ((uint8_t)(Value & (uint8_t)0xF0) >> (uint8_t)0x4) * 10;
    return (tmp + (Value & (uint8_t)0x0F));
}

static uint8_t ByteToBcd2(uint8_t Value)
{
    uint8_t bcdhigh = 0;

    while (Value >= 10)
    {
        bcdhigh++;
        Value -= 10;
    }

    return  ((uint8_t)(bcdhigh << 4) | Value);
}

void RTC_GetTime(RTC_TimeTypeDef* RTC_TimeStruct)
{
    uint32_t tmpreg;

    /* Get the RTC_TR register */
    tmpreg = (uint32_t)(RTC->TR & RTC_TR_RESERVED_MASK);

    /* Fill the structure fields with the read parameters */
    RTC_TimeStruct->RTC_Hours = (uint8_t)((tmpreg & (RTC_TR_HT | RTC_TR_HU)) >> 16);
    RTC_TimeStruct->RTC_Minutes = (uint8_t)((tmpreg & (RTC_TR_MNT | RTC_TR_MNU)) >>8);
    RTC_TimeStruct->RTC_Seconds = (uint8_t)(tmpreg & (RTC_TR_ST | RTC_TR_SU));

    /* Get the RTC_TR register */
    tmpreg = (uint32_t)(RTC->DR & RTC_DR_RESERVED_MASK);

    /* Fill the structure fields with the read parameters */
    RTC_TimeStruct->RTC_Year = (uint8_t)((tmpreg & (RTC_DR_YT | RTC_DR_YU)) >> 16);
    RTC_TimeStruct->RTC_Month = (uint8_t)((tmpreg & (RTC_DR_MT | RTC_DR_MU)) >> 8);
    RTC_TimeStruct->RTC_Date = (uint8_t)(tmpreg & (RTC_DR_DT | RTC_DR_DU));

    RTC_TimeStruct->RTC_Hours = Bcd2ToByte(RTC_TimeStruct->RTC_Hours);
    RTC_TimeStruct->RTC_Minutes = Bcd2ToByte(RTC_TimeStruct->RTC_Minutes);
    RTC_TimeStruct->RTC_Seconds = Bcd2ToByte(RTC_TimeStruct->RTC_Seconds);
    RTC_TimeStruct->RTC_Year = Bcd2ToByte(RTC_TimeStruct->RTC_Year);
    RTC_TimeStruct->RTC_Month = Bcd2ToByte(RTC_TimeStruct->RTC_Month);
    RTC_TimeStruct->RTC_Date = Bcd2ToByte(RTC_TimeStruct->RTC_Date);
}


static void RtcStartWakeUpAlarm( uint32_t timeoutValue )
{
    uint32_t tmpreg = 0;

    uint16_t rtcSeconds = 0;
    uint16_t rtcMinutes = 0;
    uint16_t rtcHours = 0;
    uint16_t rtcDays = 0;

    uint8_t rtcAlarmSeconds = 0;
    uint8_t rtcAlarmMinutes = 0;
    uint8_t rtcAlarmHours = 0;
    uint16_t rtcAlarmDays = 0;

    RTC_TimeTypeDef RTC_TimeStruct;

    RtcClearStatus( );

    RtcTimerContext = RtcGetCalendarValue( );
    RTC_GetTime( &RTC_TimeStruct );

    timeoutValue = (uint32_t)((double)timeoutValue / RTC_ALARM_TIME_BASE);

    if( timeoutValue > 2160000 ) // 25 "days" in tick
    {                            // drastically reduce the computation time
        rtcAlarmSeconds = RTC_TimeStruct.RTC_Seconds;
        rtcAlarmMinutes = RTC_TimeStruct.RTC_Minutes;
        rtcAlarmHours = RTC_TimeStruct.RTC_Hours;
        rtcAlarmDays = 25 + RTC_TimeStruct.RTC_Date;  // simply add 25 days to current date and time

        if( ( RTC_TimeStruct.RTC_Year == 0 ) || ( RTC_TimeStruct.RTC_Year % 4 == 0 ) )
        {
            if( rtcAlarmDays > DaysInMonthLeapYear[ RTC_TimeStruct.RTC_Month - 1 ] )
            {
                rtcAlarmDays = rtcAlarmDays % DaysInMonthLeapYear[ RTC_TimeStruct.RTC_Month - 1];
            }
        }
        else
        {
            if( rtcAlarmDays > DaysInMonth[ RTC_TimeStruct.RTC_Month - 1 ] )
            {
                rtcAlarmDays = rtcAlarmDays % DaysInMonth[ RTC_TimeStruct.RTC_Month - 1];
            }
        }
    }
    else
    {
        rtcSeconds = ( timeoutValue % SecondsInMinute ) + RTC_TimeStruct.RTC_Seconds;
        rtcMinutes = ( ( timeoutValue / SecondsInMinute ) % SecondsInMinute ) + RTC_TimeStruct.RTC_Minutes;
        rtcHours = ( ( timeoutValue / SecondsInHour ) % HoursInDay ) + RTC_TimeStruct.RTC_Hours;
        rtcDays = ( timeoutValue / SecondsInDay ) + RTC_TimeStruct.RTC_Date;

        rtcAlarmSeconds = ( rtcSeconds ) % 60;
        rtcAlarmMinutes = ( ( rtcSeconds / 60 ) + rtcMinutes ) % 60;
        rtcAlarmHours   = ( ( ( ( rtcSeconds / 60 ) + rtcMinutes ) / 60 ) + rtcHours ) % 24;
        rtcAlarmDays    = ( ( ( ( ( rtcSeconds / 60 ) + rtcMinutes ) / 60 ) + rtcHours ) / 24 ) + rtcDays;

        if( ( RTC_TimeStruct.RTC_Year == 0 ) || ( RTC_TimeStruct.RTC_Year % 4 == 0 ) )
        {
            if( rtcAlarmDays > DaysInMonthLeapYear[ RTC_TimeStruct.RTC_Month - 1 ] )
            {
                rtcAlarmDays = rtcAlarmDays % DaysInMonthLeapYear[ RTC_TimeStruct.RTC_Month - 1 ];
            }
        }
        else
        {
            if( rtcAlarmDays > DaysInMonth[ RTC_TimeStruct.RTC_Month - 1 ] )
            {
                rtcAlarmDays = rtcAlarmDays % DaysInMonth[ RTC_TimeStruct.RTC_Month - 1 ];
            }
        }
    }

    tmpreg = (((uint32_t)ByteToBcd2(rtcAlarmHours) << 16) | \
              ((uint32_t)ByteToBcd2(rtcAlarmMinutes) << 8) | \
              ((uint32_t)ByteToBcd2(rtcAlarmSeconds)) | \
              ((uint32_t)ByteToBcd2(rtcAlarmDays) << 24));

    /* Disable the write protection for RTC registers */
    RTC->WPR = 0xCA;
    RTC->WPR = 0x53;

    /* Configure the Alarm register */
    RTC->ALRMAR = (uint32_t)tmpreg;

    /* Enable the write protection for RTC registers */
    RTC->WPR = 0xFF;

    /* Wait for RTC APB registers synchronisation */
    RTC_WaitForSynchro( );

    RTC->WPR = 0xCA;
    RTC->WPR = 0x53;
    RTC->CR = RTC_CR_ALRAIE | RTC_CR_ALRAE;
    RTC->WPR = 0xFE;
    RTC->WPR = 0x64;
}

uint64_t RtcGetCalendarValue( void )
{
    uint64_t calendarValue = 0;
    uint8_t i = 0;

    RTC_TimeTypeDef RTC_TimeStruct;
    RTC_GetTime( &RTC_TimeStruct );

    if( ( PreviousYear == 99 ) && ( RTC_TimeStruct.RTC_Year == 0 ) )
    {
        Century++;
    }
    PreviousYear = RTC_TimeStruct.RTC_Year;

    // century
    for( i = 0; i < Century; i++ )
    {
        calendarValue += ( uint64_t )( DaysInCentury * SecondsInDay );
    }

    // years
    for( i = 0; i < RTC_TimeStruct.RTC_Year; i++ )
    {
        if( ( i == 0 ) || ( i % 4 == 0 ) )
        {
            calendarValue += DaysInLeapYear * SecondsInDay;
        }
        else
        {
            calendarValue += DaysInYear * SecondsInDay;
        }
    }

    // months
    if( ( RTC_TimeStruct.RTC_Year == 0 ) || ( RTC_TimeStruct.RTC_Year % 4 == 0 ) )
    {
        for( i = 0; i < ( RTC_TimeStruct.RTC_Month - 1 ); i++ )
        {
            calendarValue += DaysInMonthLeapYear[i] * SecondsInDay;
        }
    }
    else
    {
        for( i = 0;  i < ( RTC_TimeStruct.RTC_Month - 1 ); i++ )
        {
            calendarValue += DaysInMonth[i] * SecondsInDay;
        }
    }

    // days
    calendarValue += ( ( uint32_t )RTC_TimeStruct.RTC_Seconds +
                      ( ( uint32_t )RTC_TimeStruct.RTC_Minutes * SecondsInMinute ) +
                      ( ( uint32_t )RTC_TimeStruct.RTC_Hours * SecondsInHour ) +
                      ( ( uint32_t )( RTC_TimeStruct.RTC_Date * SecondsInDay ) ) );

    return( calendarValue );
}
