#include "board.h"
#include "sensor.h"

/*
MCP9700A:
        Vout = Tc * Ta + V0
Tc:temperature coefficient 
Ta:ambient temperature
V0:sensor output vlotage at 0
mcp9700A:tc=10,v0=500
*/
float get_sensor_value()
{
    float vamb = 0.0;
#if defined (MCP9700)
    uint16_t vout = Mcp97SensorAdc();
    vamb = ( float )( vout - TEMP_0_VOL ) /( float )TEMP_COEF;

#elif defined (MCP9800)
    vamb = Mcp98SensorI2c();

#else
    //todo
#endif
    return ( vamb ? vamb : 0);
}
#if defined (MCP9700)
extern Adc_t Adc;

uint16_t GetAdcVref()
{
    uint16_t vdiv = 0;
    uint16_t voltage = 0;

    vdiv = AdcReadChannel( &Adc, ADC_CHANNEL_VREFINT);
    voltage = FACTORY_POWER_SUPPLY * ( ( float )VREFINT_CAL / ( float )vdiv );

    return voltage;
}
uint16_t Mcp97SensorAdc()
{
    uint16_t vdiv = 0;
    uint16_t voltage = 0;
    uint16_t vref = 0;

    vref = GetAdcVref();
    vdiv = AdcReadChannel( &Adc, BAT_LEVEL_CHANNEL);
    voltage = vref * ( ( float )vdiv / ( float )ADC_MAX_VALUE );

    return voltage;
}
#elif defined (MCP9800)
extern I2c_t I2c;
static uint8_t I2cDeviceAddr = 0;

uint8_t Mcp98Init( void )
{
    Mcp98SetDeviceAddr( MCP_I2C_ADDRESS );
    
    return SUCCESS;
}

uint8_t Mcp98Write( uint8_t addr, uint8_t data )
{
    return Mcp98WriteBuffer( addr, &data, 1 );
}

uint8_t Mcp98WriteBuffer( uint8_t addr, uint8_t *data, uint8_t size )
{
    return I2cWriteBuffer( &I2c, I2cDeviceAddr << 1, addr, data, size );
}

uint8_t Mcp98Read( uint8_t addr, uint8_t *data )
{
    return Mcp98ReadBuffer( addr, data, 1 );
}

uint8_t Mcp98ReadBuffer( uint8_t addr, uint8_t *data, uint8_t size )
{
    return I2cReadBuffer( &I2c, I2cDeviceAddr << 1, addr, data, size );
}

void Mcp98SetDeviceAddr( uint8_t addr )
{
    I2cDeviceAddr = addr;
}

uint8_t Mcp98GetDeviceAddr( void )
{
    return I2cDeviceAddr;
}

void Mcp98Check()
{
    uint8_t conReg = 0;
    //Mcp98Write( CONF_REG, 0x02);
    Mcp98Read( CONF_REG, &conReg );
}

float Mcp98SensorI2c( void )
{
    uint8_t tempBuf[2];
    uint8_t msb = 0, lsb = 0;
    bool negSign = false;
    uint8_t val = 0;
    float temperature = 0;

    //Mcp98Check();
    
    Mcp98ReadBuffer( TEMP_REG, tempBuf, 2 );

    msb = tempBuf[0];
    lsb = tempBuf[1];

    if( msb > 0x7F )
    {
        val = ~( ( msb << 8 ) + lsb ) + 1;      // 2¡¯s complement
        msb = val >> 8;
        lsb = val & 0x00F0;
        negSign = true;
    }

    if( negSign == true )
    {
        temperature = 0 - ( msb + ( float )( ( lsb >> 4 ) / 16.0 ) );
    }
    else
    {
        temperature = msb + ( float )( ( lsb >> 4 ) / 16.0 );
    }


    return( temperature );
}

#endif