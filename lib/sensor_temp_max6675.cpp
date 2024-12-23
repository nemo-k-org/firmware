#include <max6675.h>

int spiDO = 3;  // ESP01 GPIO 3 (pin 7)
int spiCS = 2;  // ESP01 GPIO 2 (pin 3)
int spiCLK = 0; // ESP01 GPIO 0 (pin 5)

MAX6675 max6675(spiCLK, spiCS, spiDO);

void sensorSetup() {
    // This sensor does not need setup
}

void sensorLoop() {
    float tempCelsius = max6675.readCelsius();
    float tempKelvin = tempCelsius + 273.15;
    if (strcmp(NEMOK_SENSOR_KEY, "") != 0) {
        sigK.sendDelta(NEMOK_SENSOR_KEY, tempKelvin);
    }
}
