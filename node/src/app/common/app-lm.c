/*
    _/_/_/    _/_/_/    _/_/_/  _/_/_/  _/      _/    _/_/_/  _/    _/  _/_/_/_/
   _/    _/    _/    _/          _/    _/_/    _/  _/        _/    _/  _/
  _/_/_/      _/      _/_/      _/    _/  _/  _/  _/  _/_/  _/_/_/_/  _/_/_/
 _/    _/    _/          _/    _/    _/    _/_/  _/    _/  _/    _/  _/
_/    _/  _/_/_/  _/_/_/    _/_/_/  _/      _/    _/_/_/  _/    _/  _/
    (C)2015 RisingHF, all rights reserved.

Description: LoRaMac-Node application file

*/
#include "app-lm.h"
#include "board.h"
#include "LoRaMac.h"
#include "LoRaMac-api-v3.h"

lm_callback_t lm_callback_g;
#define LOAD_VALUE      0x6
static uint8_t DevEui[] = {
    0x46, 0x4c, 0x45, 0x58, 0x01, 0x01, 0x00, 0x00
};
static uint8_t AppEui[] = {
    0x46, 0x4c, 0x45, 0x58, 0x01, 0x01, 0x00, 0x00
};
static uint8_t NwkSKey[] = {
    0x6c, 0x6f, 0x72, 0x61, 0x6e, 0x6f, 0x64, 0x65,
    0x46, 0x4c, 0x45, 0x58, 0x01, 0x01, 0x00, 0x00
};
static uint8_t AppSKey[] = {
    0x6c, 0x6f, 0x72, 0x61, 0x6e, 0x6f, 0x64, 0x65,
    0x46, 0x4c, 0x45, 0x58, 0x01, 0x01, 0x00, 0x00
};
static uint8_t AppKey[] = {
    0x6c, 0x6f, 0x72, 0x61, 0x6e, 0x6f, 0x64, 0x65,
    0x46, 0x4c, 0x45, 0x58, 0x01, 0x01, 0x00, 0x00
};
static uint32_t DevAddr;
//static LoRaMacEvent_t LoRaMacEvents;
static LoRaMacCallbacks_t LoRaMacCallbacks;
LoRaMacRxInfo RxInfo;
LoRaMode_t Mode = ABP;
uint8_t FlashArray[41] = {0};
extern uint8_t loramac_join_flag;
extern uint8_t loramac_send_flag;
void OnMacEvent( LoRaMacEventFlags_t *flags, LoRaMacEventInfo_t *info )
{
    switch( info->Status ){
        case LORAMAC_EVENT_INFO_STATUS_OK:
            break;
        case LORAMAC_EVENT_INFO_STATUS_RX2_TIMEOUT:
            break;
        case LORAMAC_EVENT_INFO_STATUS_ERROR:
            break;
        default:
            break;
    }

    if( flags->Bits.JoinAccept == 1 ){
#ifdef MODE_OTA
        memset(FlashArray, 0x00, sizeof(FlashArray));
        DevAddr = LoRaMacGetDevAddr();
        memcpy(FlashArray, FLASH_HEAD, 4);
        FlashArray[4] = ( DevAddr ) & 0xFF;
        FlashArray[5] = ( DevAddr >> 8 ) & 0xFF;
        FlashArray[6] = ( DevAddr >> 16 ) & 0xFF;
        FlashArray[7] = ( DevAddr >> 24 ) & 0xFF;
        LoRaMacGetNwkSKey(NwkSKey);
        memcpy(FlashArray + 8, NwkSKey, sizeof(NwkSKey));
        LoRaMacGetAppSKey(AppSKey);
        memcpy(FlashArray + 24, AppSKey, sizeof(AppSKey));
        //Flash_If_Write(FlashArray, (uint8_t *)USBD_DFU_APP_DEFAULT_ADD, 40*4);
        LoRaMacInitNwkIds( 0x000000, DevAddr, NwkSKey, AppSKey );
        loramac_join_flag = 1;
        loramac_send_flag = 1;
        //memset(FlashArray, 0x00, sizeof(FlashArray));
        //Flash_If_Read((uint8_t *)USBD_DFU_APP_DEFAULT_ADD, FlashArray, 40);
#endif
    }

    if( info->TxAckReceived == true ){
        if(lm_callback_g!=NULL){
            lm_callback_g(LM_STA_ACK_RECEIVED, &RxInfo);
        }
    }else if( flags->Bits.Rx == 1 ){
        RxInfo.size = info->RxBufferSize;
        memcpy(RxInfo.buf, info->RxBuffer, RxInfo.size);
        RxInfo.rssi = info->RxRssi;
        RxInfo.snr = info->RxSnr;
        RxInfo.win = flags->Bits.RxSlot+1;
        RxInfo.port = info->RxPort;
        if(RxInfo.size>0){
            if(lm_callback_g!=NULL){
                lm_callback_g(LM_STA_RXDONE, &RxInfo);
            }
        }else{
            if(lm_callback_g!=NULL){
                lm_callback_g(LM_STA_CMD_RECEIVED, &RxInfo);
            }
        }
    }else{
        if(lm_callback_g!=NULL){
            lm_callback_g(LM_STA_TXDONE, NULL);
        }
    }
}

void app_lm_init(lm_callback_t cb)
{
    
    // Initialize LoRaMac device unique ID
    DevEui[7] = LOAD_VALUE;
    AppEui[7] = LOAD_VALUE;
    //BoardGetUniqueId( DevEui );

    LoRaMacCallbacks.MacEvent = OnMacEvent;
    LoRaMacCallbacks.GetBatteryLevel = NULL;
    LoRaMacInit( &LoRaMacCallbacks );

    // Random seed initialization
    srand1( RAND_SEED );
    // Choose a random device address
    // NwkID = 0
    // NwkAddr rand [0, 33554431]
    
    DevAddr = 0x01010000;//jason
#if 1
    //DevAddr = randr( 0, 0x01FFFFFF );
    DevAddr = DevAddr | LOAD_VALUE;
    NwkSKey[15] = LOAD_VALUE;
    AppSKey[15] = LOAD_VALUE;
    AppKey[15] = LOAD_VALUE;
#endif
    LoRaMacInitNwkIds( 0x000000, DevAddr, NwkSKey, AppSKey );
    LoRaMacSetAdrOn( true );
    LoRaMacTestSetDutyCycleOn(false);

    lm_callback_g = cb;
}
//flash format
/*
XXXX XXXX XXXXXXXXXXXXXXXX XXXXXXXXXXXXXXXX
FLEX DevAddr NwkSKey AppSKey
*/

void app_lm_para_init(void)
{
    app_lm_set_mode(OTA);
    if (app_lm_get_mode() == OTA)
    {
        memset(FlashArray, 0x00, sizeof(FlashArray));
        //Flash_If_Read((uint8_t *)USBD_DFU_APP_DEFAULT_ADD, FlashArray, 40);
        if (strncmp((char *)FlashArray, FLASH_HEAD, 4) == 0)
        {
          DevAddr = ( uint32_t )FlashArray[4];
          DevAddr |= ( ( uint32_t )FlashArray[5] << 8 );
          DevAddr |= ( ( uint32_t )FlashArray[6] << 16 );
          DevAddr |= ( ( uint32_t )FlashArray[7] << 24 );
          memcpy(NwkSKey, FlashArray+8, sizeof(NwkSKey));
          memcpy(AppSKey, FlashArray+24,sizeof(AppSKey));
          LoRaMacInitNwkIds( 0x000000, DevAddr, NwkSKey, AppSKey );
          loramac_join_flag = 1;
          //loramac_send_flag = 1;
        }
    }
}

int app_lm_send( LoRaFrameType_t type, uint8_t *buf, int size, int retry)
{
	int sendFrameStatus;

	if(size == 0 || buf == 0){
		return -3;
	}

	if(type == UNCONFIRMED){
		sendFrameStatus = LoRaMacSendFrame( AT_MODULE_PORT, buf, size );
	}else{
		if(retry <= 0){
			retry = 3;
		}
		sendFrameStatus = LoRaMacSendConfirmedFrame( AT_MODULE_PORT, buf, size, retry );
	}

    switch( sendFrameStatus )
    {
	case 1: // LoRaMac is Busy
		return -1;
    case 2:
    case 3: // LENGTH_PORT_ERROR
    case 4: // MAC_CMD_ERROR
    case 5: // NO_FREE_CHANNEL
    case 6:
    	return -2;
    default:
        break;
    }
    return 0;
}

uint8_t app_lm_join(void)
{
    uint8_t status = 0;
    Flash_If_Erase_Page((uint32_t)USBD_DFU_APP_DEFAULT_ADD);
    status = LoRaMacJoinReq(DevEui, AppEui, AppKey);
    
    return status;
}

uint32_t app_lm_get_devaddr(void)
{
    return DevAddr;
}

void app_lm_set_mode(LoRaMode_t mode)
{
    Mode = mode;
}

uint8_t app_lm_get_mode(void)
{
    return Mode;
}
