#include <Arduino.h>
#include <ADS1115_WE.h>
#include <Wire.h>

#define NEMOK_SENSOR_ADC_TANKLEVEL_DIVIDER_RESISTANCE 100.0
#define NEMOK_SENSOR_ADC_TANKLEVEL_GPIO_SDA 2
#define NEMOK_SENSOR_ADC_TANKLEVEL_GPIO_SDL 0

ADS1115_WE adc = ADS1115_WE(0x48);
String nemoKSensorKey;
String nemoKSensorKeyDebugResistance;

float nemoKADCTanklevel0;
float nemoKADCTanklevel100;

void sensorSetup()
{
    Wire.begin(NEMOK_SENSOR_ADC_TANKLEVEL_GPIO_SDA, NEMOK_SENSOR_ADC_TANKLEVEL_GPIO_SDL);
    delay(1000);

    if (!adc.init())
    {
        nemokPrint("ADS1115 not connected!");
        send_error_message(1);
        ESP.restart();
    }

    adc.setVoltageRange_mV(ADS1115_RANGE_4096);
    adc.setCompareChannels(ADS1115_COMP_1_GND);
    adc.setMeasureMode(ADS1115_CONTINUOUS);

    nemoKSensorKeyDebugResistance = preferencesKey + ".debug.resistance";

    nemoKADCTanklevel0 = prefs.getInt("adctank0");
    nemoKADCTanklevel100 = prefs.getInt("adctank1");
}

float readChannel(ADS1115_MUX channel)
{
    float voltage = 0.0;
    adc.setCompareChannels(channel);
    voltage = adc.getResult_V();
    return voltage;
}

float readResistance()
{
    float voltage_vcc = readChannel(ADS1115_COMP_1_GND);
    float divider_v = readChannel(ADS1115_COMP_0_GND);

    float resistance = (NEMOK_SENSOR_ADC_TANKLEVEL_DIVIDER_RESISTANCE * divider_v) / (voltage_vcc - divider_v);

    return resistance;
}

float calculatePercentage(float resistance)
{
    float percentage = (resistance - nemoKADCTanklevel0) / (nemoKADCTanklevel100 - nemoKADCTanklevel0);
    return percentage;
}

void sensorLoop()
{
    float resistance = readResistance();
    float percentage = calculatePercentage(resistance);

    nemokPrint("resistance:");
    nemokPrint(resistance);
    nemokPrint("percentage:");
    nemokPrint(percentage);

    if (preferencesKey.length() > 0)
        sigK.sendDelta(preferencesKey, percentage);
    sigK.sendDelta(nemoKSensorKeyDebugResistance, resistance);
}
