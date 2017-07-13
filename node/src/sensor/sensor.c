#include "board.h"
#include "sensor.h"
#include "math.h"  
/*
MCP9700A:
        Vout = Tc * Ta + V0
Tc:temperature coefficient 
Ta:ambient temperature
V0:sensor output vlotage at 0
mcp9700A:tc=10,v0=500
*/
uint8_t default_config[2] = {0x75, 0x00};
uint8_t powerdown_config[2] = {0x05, 0x00};
float get_sensor_value()
{
    float vamb = 0.0;
    uint8_t i = 3;
#if defined (MCP9700)
    uint16_t vout = Mcp97SensorAdc();
    vamb = ( float )( vout - TEMP_0_VOL ) /( float )TEMP_COEF;

#elif defined( TMP006 )
#if 0
      vamb += (float)randr(-1, 11)/10 + 32.0;
#else
    //Tmp006Write(CONF_REG, default_config, 2);
    vamb = Tmp006SensorI2c();
    if (vamb == 0)
      vamb = 25.0;
#endif
    Tmp006Write(CONF_REG, powerdown_config, 2);
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
#elif defined( TMP006 )
uint8_t tempBuf[3];
float Tobj;
float Tmp006SensorI2c( void )
{
    uint16_t Vobj_Read;
    float Vobj;  
    float Tdie;  
    
    float S,Vos,fVobj;  
    float S0 = 6.4*pow(10,-14);  
    float a1 = 1.75*pow(10,-3);  
    float a2 = -1.678*pow(10,-5);  
    float Tref = 298.15;  
    float b0 = -2.94*pow(10,-5);  
    float b1 = -5.7*pow(10,-7);  
    float b2 = 4.63*pow(10,-9);  
    float c2 = 13.4; 
    
    
    Tmp006Read( MAUN_ID_REG, tempBuf, 2 );
    if(tempBuf[0] == 0 || tempBuf[1] == 0)
      return 0;
    Tmp006Write( CONF_REG, default_config, 2 );
    do
    {
      Tmp006Read( CONF_REG, tempBuf, 2 );
    }while((tempBuf[1] & 0x80) != 0x80);
    
    Tmp006Read( TEMP_REG, tempBuf, 2 );
    Tdie = ((tempBuf[0] << 8) +  tempBuf[1]) * 0.0078125 + 273.15;
    
    Tmp006Read( SENSOR_REG, tempBuf, 2 );
    Vobj_Read  = (tempBuf[0] << 8) + tempBuf[1];
    if(Vobj_Read >= 0x8000)  
    {
        Vobj = -(0xffff - Vobj_Read)*156.25;  
    }

    else
    {
        Vobj = Vobj_Read*156.25;   
    }
    Vobj *= pow(10,-9);  
    S = S0*(1 + a1*(Tdie - Tref) + a2*(Tdie-Tref)*(Tdie - Tref));  
    Vos = b0 + b1*(Tdie - Tref) + b2*(Tdie - Tref)*(Tdie - Tref);  
    
    fVobj = Vobj - Vos + c2*(Vobj - Vos)*(Vobj - Vos);  
    Tobj = sqrt(sqrt(pow(Tdie,4) + fVobj/S));  
    Tobj -= 273.15;  
    return( Tobj );
}

#endif