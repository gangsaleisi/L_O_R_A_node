#ifndef __SENSOR_H__
#define __SENSOR_H__

//MCP9700A
#define TEMP_COEF       (10)
#define TEMP_0_VOL      (500)
#define ADC_MAX_VALUE    4095
#define VREFINT_CAL           ( *( uint16_t* )0x1FF80078 )
#define FACTORY_POWER_SUPPLY             3000 // mV

#if defined( TMP006 )
#define MCP_I2C_ADDRESS     0x80 //1001000
#define SENSOR_REG      0x00
#define TEMP_REG        0x01
#define CONF_REG        0x02
#define MAUN_ID_REG     0xfe
#define DEV_REG         0xff
//MCP9800
#elif defined (MCP9800)
#define MCP_I2C_ADDRESS     0x48 //1001000
#define TEMP_REG        0x00
#define CONF_REG        0x01
#define TEMP_HYST_REG   0x10
#define TEMP_LIMT_REG   0x11
#endif

uint16_t Mcp97SensorAdc( void );
uint8_t Tmp006Write( uint16_t addr, uint8_t *data,  uint16_t Size);
uint8_t Tmp006Read( uint16_t addr, uint8_t *data,  uint16_t Size);
float Tmp006SensorI2c( void );
#endif