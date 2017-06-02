/*
    _/_/_/    _/_/_/    _/_/_/  _/_/_/  _/      _/    _/_/_/  _/    _/  _/_/_/_/
   _/    _/    _/    _/          _/    _/_/    _/  _/        _/    _/  _/
  _/_/_/      _/      _/_/      _/    _/  _/  _/  _/  _/_/  _/_/_/_/  _/_/_/
 _/    _/    _/          _/    _/    _/    _/_/  _/    _/  _/    _/  _/
_/    _/  _/_/_/  _/_/_/    _/_/_/  _/      _/    _/_/_/  _/    _/  _/
    (C)2015 RisingHF, all rights reserved.

Description: Application LoRaWAN stack

*/
#ifndef __APP_LM_H
#define __APP_LM_H

#include <stdint.h>

#define AT_MODULE_PORT				(9)

#define AESKEY_LEN                  (16)

typedef enum{
        ABP,
        OTA,
}LoRaMode_t;

typedef enum{
	IDLE,
	BUSY,
	DONE,
}LoRaStatus_t;

typedef enum{
	CONFIRMED,
	UNCONFIRMED
}LoRaFrameType_t;

typedef enum{
	INVALID,
	RECEIVED,
	UNRECEIVED,
}LoRaTxAckReceived_t;

typedef enum{
    LORAMAC_NWKSKEY=0,
    LORAMAC_APPSKEY=1,
    LORAMAC_APPKEY=2,
}LoRaMacKey_t;

typedef struct{
    int16_t rssi;
    uint8_t snr;
    uint8_t win;
    uint8_t port;
    uint16_t size;
    uint8_t buf[256];
}LoRaMacRxInfo;

typedef enum{
    LM_STA_TXDONE,
    LM_STA_RXDONE,
    LM_STA_RXTIMEOUT,
    LM_STA_ACK_RECEIVED,
    LM_STA_ACK_UNRECEIVED,
    LM_STA_CMD_RECEIVED,
}lm_evt_t;

typedef void (*lm_callback_t) (lm_evt_t lm_evt, void *msg);

void app_lm_init(lm_callback_t cb);
void app_lm_para_init(void);
uint8_t app_lm_join(void);
int app_lm_send( LoRaFrameType_t type, uint8_t *buf, int size, int retry);
uint8_t app_lm_join(void);
uint32_t app_lm_get_devaddr(void);
uint8_t app_lm_get_mode(void);
void app_lm_set_mode(LoRaMode_t mode);

int LoRaMacSetKey(LoRaMacKey_t key_type, uint8_t *key);
int LoRaMacGetKey(LoRaMacKey_t key_type, uint8_t *key);

#endif
