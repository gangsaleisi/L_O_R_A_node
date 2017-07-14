/*
    _/_/_/    _/_/_/    _/_/_/  _/_/_/  _/      _/    _/_/_/  _/    _/  _/_/_/_/
   _/    _/    _/    _/          _/    _/_/    _/  _/        _/    _/  _/
  _/_/_/      _/      _/_/      _/    _/  _/  _/  _/  _/_/  _/_/_/_/  _/_/_/
 _/    _/    _/          _/    _/    _/    _/_/  _/    _/  _/    _/  _/
_/    _/  _/_/_/  _/_/_/    _/_/_/  _/      _/    _/_/_/  _/    _/  _/
    (C)2015 RisingHF, all rights reserved.

Description: LoRaWAW Class A/C Example

*/
#include "board.h"
#include "radio.h"
#include "LoRaMac.h"
#include "app-lm.h"
#include "LoRaMac-api-v3.h"
#include "sensor.h"

#define APP_PORT                                        (2)
#define APP_DATA_SIZE                                   (28)

#define APP_TX_DUTYCYCLE                                (90000000)     // min
#define APP_TX_DUTYCYCLE_RND                            (1500000)   // us

extern void SendData(uint8_t *pdata, uint16_t Length);

typedef enum{
    SYS_STA_IDLE,       // report status in period
    SYS_STA_TX,
    SYS_STA_WAIT,
}sys_sta_t;

static TimerEvent_t ReportTimer;
volatile bool ReportTimerEvent = false;
extern Gpio_t Led;

uint8_t loramac_evt_flag = 0;
uint8_t loramac_join_flag = 0;
LoRaMacRxInfo *loramac_rx_info;
lm_evt_t loramac_evt;

sys_sta_t sys_sta = SYS_STA_IDLE;

uint8_t AppData[APP_DATA_SIZE];

void OnReportTimerEvent( void )
{
    ReportTimerEvent = true;
    
}

void app_lm_cb (lm_evt_t evt, void *msg)
{
    switch(evt){
    case LM_STA_TXDONE:
    case LM_STA_RXDONE:
    case LM_STA_RXTIMEOUT:
    case LM_STA_ACK_RECEIVED:
    case LM_STA_ACK_UNRECEIVED:
    case LM_STA_CMD_RECEIVED:
        loramac_rx_info = msg;
        loramac_evt = evt;
        loramac_evt_flag = 1;
        break;
    }
}
extern uint8_t global_local[];
extern uint8_t global_obj[];
extern uint8_t global_manu[];
uint8_t powerdown_config[2] = {0x05, 0x00};
int main( void )
{
    BoardInitMcu( );
    BoardInitPeriph( );
    
    
    app_lm_init(app_lm_cb);
#ifdef MODE_OTA
    app_lm_para_init();
#endif
    Tmp006Write( CONF_REG, powerdown_config, 2 );
    while(1)
    {
      sprintf((char *)AppData, "%f\t%x%x\t%x%x\t%x%x\n", (double)get_sensor_value(), 
              global_manu[0], global_manu[1], global_local[0], global_local[1],global_obj[0],global_obj[1]);
      Tmp006Write( CONF_REG, powerdown_config, 2 );
      AppData[27] = '\n';
      SendData(AppData, 28);
      DelayMs( 1000 );
    }
    /* Uncomment below line to enable class C mode */
#ifdef CLASS_TYPE_C
    LoRaMacSetDeviceClass(CLASS_C);
#endif
    TimerInit( &ReportTimer, OnReportTimerEvent );
    ReportTimerEvent = true;
#ifndef USE_DEBUGGER
    DelayMs( 10000 );
#endif
    while( 1 )
    {
        switch(sys_sta){
        case SYS_STA_IDLE:
            if(ReportTimerEvent == true){
                ReportTimerEvent = false;
                sys_sta = SYS_STA_TX;
            }
            break;
        case SYS_STA_TX:
            // Toggle LED 1
            GpioWrite( &Led, 1 );
            DelayMs( 5 );
            GpioWrite( &Led, 0 );
            if( loramac_join_flag == 0 && app_lm_get_mode() == OTA){
              app_lm_join();
              sys_sta = SYS_STA_WAIT;
            }
            else
            {
              sprintf((char *)AppData, "%8.2f", (double)get_sensor_value());
              //jason
              //if( app_lm_send(UNCONFIRMED, AppData, APP_DATA_SIZE, 0) == 0 ){
              if( app_lm_send(CONFIRMED, AppData, APP_DATA_SIZE, 0) == 0 ){
              sys_sta = SYS_STA_WAIT;
              }else{
                  sys_sta = SYS_STA_IDLE;
                  ReportTimerEvent = false;
                  TimerStop( &ReportTimer );
                  TimerSetValue( &ReportTimer, APP_TX_DUTYCYCLE + randr( -APP_TX_DUTYCYCLE_RND, APP_TX_DUTYCYCLE_RND ) );
                  TimerStart( &ReportTimer );
              }
            }
            break;
        case SYS_STA_WAIT:

            break;
        }

        if(loramac_evt_flag == 1){
            __disable_irq();
            loramac_evt_flag = 0;
            __enable_irq();

            sys_sta = SYS_STA_IDLE;

            ReportTimerEvent = false;
            TimerSetValue( &ReportTimer, APP_TX_DUTYCYCLE + randr( -APP_TX_DUTYCYCLE_RND, APP_TX_DUTYCYCLE_RND ) );
            TimerStart( &ReportTimer );
        }

        TimerLowPowerHandler( );
    }
}

