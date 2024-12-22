#include <OneWire.h>
#include <DallasTemperature.h>

#define ONEWIRE_PIN 2

OneWire oneWire(ONEWIRE_PIN);
DallasTemperature sensors(&oneWire);

void sensorSetup() {
    sensors.begin();
}

void sensorLoop() {
    sensors.requestTemperatures();

    float tempCelsius = sensors.getTempCByIndex(0);
    if (tempCelsius != DEVICE_DISCONNECTED_C) {
        float tempKelvin = tempCelsius + 273.15;
        if (strcmp(NEMOK_SENSOR_KEY, "") != 0) {
            sigK.sendDelta(NEMOK_SENSOR_KEY, tempKelvin);
        }
    }
}
