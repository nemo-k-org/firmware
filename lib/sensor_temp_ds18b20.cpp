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
        if (nemokPrefs.get(NemokPreferences::KEY).length() > 0) {
            sigK.sendDelta(nemokPrefs.get(NemokPreferences::KEY), tempKelvin);
        }
    }
}
