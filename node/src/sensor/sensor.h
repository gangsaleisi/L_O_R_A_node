#ifndef __SENSOR_H__
#define __SENSOR_H__

//MCP9700A
#define TEMP_COEF       (10)
#define TEMP_0_VOL      (500)
#define ADC_MAX_VALUE    4095
#define VREFINT_CAL           ( *( uint16_t* )0x1FF80078 )
#define FACTORY_POWER_SUPPLY             3000 // mV

//TMP006
#if defined( TMP006 )
#define MCP_I2C_ADDRESS     0x40 //1001000
#define SENSOR_REG      0x00
#define TEMP_REG        0x01
#define CONF_REG        0x02
#define MAUN_ID_REG     0xfe
#define DEV_REG         0xff
//MCP9800
#else defined (MCP9800)
#define MCP_I2C_ADDRESS     0x48 //1001000
#define TEMP_REG        0x00
#define CONF_REG        0x01
#define TEMP_HYST_REG   0x10
#define TEMP_LIMT_REG   0x11
#endif


uint16_t Mcp97SensorAdc( void );
uint8_t Mcp98Write( uint8_t addr, uint8_t data );
uint8_t Mcp98WriteBuffer( uint8_t addr, uint8_t *data, uint8_t size );
uint8_t Mcp98Read( uint8_t addr, uint8_t *data );
uint8_t Mcp98ReadBuffer( uint8_t addr, uint8_t *data, uint8_t size );
void Mcp98SetDeviceAddr( uint8_t addr );
uint8_t Mcp98GetDeviceAddr( void );
void Mcp98Check( void );
float Mcp98SensorI2c( void );
#endif