#include <Arduino.h>
#include <WiFiUdp.h>

#define NEO6MV2_UDP_PORT 3000

WiFiUDP Udp;

char _neo6mv2udp_inputBuffer[512];
unsigned int _neo6mv2udp_inputBufferIndex;

void sensorSetup()
{
    Serial.begin(9600);

    if (! wifi_initialise()) {
        Serial.println("Could not initialise WiFi, restarting...");
        ESP.restart();
    }

    Udp.begin(NEO6MV2_UDP_PORT);
}

void sensorLoop()
{
    while (Serial.available() > 0) {
        char c = Serial.read();

        if (c == '\n') {
            _neo6mv2udp_inputBuffer[_neo6mv2udp_inputBufferIndex] = '\n';
            _neo6mv2udp_inputBuffer[_neo6mv2udp_inputBufferIndex + 1] = '\0';
            Udp.beginPacket(NEMOK_SIGNALK_SERVER_HOST, NEMOK_SIGNALK_SERVER_PORT);
            Udp.write(_neo6mv2udp_inputBuffer);
            Udp.endPacket();
            _neo6mv2udp_inputBufferIndex = 0;
        } else {
            if (_neo6mv2udp_inputBufferIndex < sizeof(_neo6mv2udp_inputBuffer) - 2) {
                _neo6mv2udp_inputBuffer[_neo6mv2udp_inputBufferIndex++] = c;
            }
        }
    }
}
