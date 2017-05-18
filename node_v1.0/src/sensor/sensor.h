#ifndef __SENSOR_H__
#define __SENSOR_H__

//MCP9700A
#define TEMP_COEF       (10)
#define TEMP_0_VOL      (500)
#define ADC_MAX_VALUE    4095
#define VREFINT_CAL           ( *( uint16_t* )0x1FF80078 )
#define FACTORY_POWER_SUPPLY             3300 // mV

//MCP9800
#define MCP_I2C_ADDRESS     0x48 //1001000
#define TEMP_REG        0x00
#define CONF_REG        0x01
#define TEMP_HYST_REG   0x10
#define TEMP_LIMT_REG   0x11


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