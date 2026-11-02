#include <Arduino.h>
#include <ADS1115_WE.h>
#include <Wire.h>

#define NEMOK_SENSOR_ADC_TANKLEVEL_DIVIDER_RESISTANCE 100.0

#ifndef NEMOK_SENSOR_KEY
#error "NEMOK_SENSOR_KEY is not defined"
#else
static_assert(sizeof("" NEMOK_SENSOR_KEY) > 1,
              "NEMOK_SENSOR_KEY must not be empty");
#endif

ADS1115_WE adc = ADS1115_WE(0x48);
String nemoKSensorKeyDebugResistance;

void sensorSetup()
{
    Wire.begin(0, 2);
    delay(1000);

    if (!adc.init())
    {
        Serial.println("ADS1115 not connected!");
        send_error_message(1);
        ESP.restart();
    }

    adc.setVoltageRange_mV(ADS1115_RANGE_4096);
    adc.setCompareChannels(ADS1115_COMP_1_GND);
    adc.setMeasureMode(ADS1115_CONTINUOUS);

    nemoKSensorKeyDebugResistance = String(NEMOK_SENSOR_KEY) + ".debug.resistance";
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
    float percentage = (resistance - NEMOK_SENSOR_ADC_TANKLEVEL_0) / (NEMOK_SENSOR_ADC_TANKLEVEL_100 - NEMOK_SENSOR_ADC_TANKLEVEL_0);
    return percentage;
}

void sensorLoop()
{
    float resistance = readResistance();
    float percentage = calculatePercentage(resistance);

    if (strcmp(NEMOK_SENSOR_KEY, "") != 0)
        sigK.sendDelta(NEMOK_SENSOR_KEY, percentage);
    sigK.sendDelta(nemoKSensorKeyDebugResistance, resistance);
}
