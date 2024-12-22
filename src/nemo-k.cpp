#include <Arduino.h>
#include "../lib/tool_selector.cpp"

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
WiFiClient wiFiClient;
#include <ESP8266HTTPClient.h>

// ESP01S blue led is connected to GPIO 1 (pin 2)
#define LED_BUILTIN 1

#ifdef NEED_LIB_ESPSIGK
#include <EspSigK.h>
EspSigK sigK(NEMOK_SENSOR_HOSTNAME, NEMOK_WIFI_SSID, NEMOK_WIFI_PASS, &wiFiClient);
#endif

#define BLINK_LENGTH_SHORT 100
#define BLINK_LENGTH_LONG 1000

void blink_led(int length, int times) {
    for (int n=0; n < times; n++) {
        digitalWrite(LED_BUILTIN, HIGH);
        yield();
        delay(length);
        digitalWrite(LED_BUILTIN, LOW);
        yield();
        delay(length);
    }
}

bool wifi_initialise() {
    Serial.print("Connecting to WiFi...");
    Serial.print(NEMOK_WIFI_SSID);

    unsigned int WAITING_TIME_MS = 60000;
    unsigned int wifi_timeout = millis() + WAITING_TIME_MS;

    WiFi.mode(WIFI_STA);
    WiFi.begin(NEMOK_WIFI_SSID, NEMOK_WIFI_PASS);

    while (WiFi.status() != WL_CONNECTED) {
        blink_led(BLINK_LENGTH_SHORT, 1);
        Serial.print(".");
        if (wifi_timeout < millis()) {
            Serial.println("Timeout");
            return false;
        }
    }

    Serial.println(WiFi.localIP());

    return true;
}

#if NEMOK_SENSOR_TEST_HTTP > 0
#include "../lib/sensor_test_http.cpp"
#elif NEMOK_SENSOR_TEMP_MAX6675 > 0
#include "../lib/sensor_temp_max6675.cpp"
#endif

void setup () {
    //Serial.begin(115200);
    pinMode(LED_BUILTIN, OUTPUT);

    #ifdef NEED_LIB_ESPSIGK
    if (strcmp(NEMOK_SIGNALK_SERVER_HOST, "") != 0) {
        sigK.setServerHost(NEMOK_SIGNALK_SERVER_HOST);
    }

    #if NEMOK_SIGNALK_SERVER_PORT > 0
    sigK.setServerPort(NEMOK_SIGNALK_SERVER_PORT);
    #endif

    if (strcmp(NEMOK_SIGNALK_SERVER_TOKEN, "") != 0) {
        sigK.setServerToken(NEMOK_SIGNALK_SERVER_TOKEN);
    }

    sigK.begin();

    blink_led(BLINK_LENGTH_SHORT, 3);
    #endif

    sensorSetup();
}

void loop () {
    sensorLoop();

    digitalWrite(LED_BUILTIN, HIGH);
    #if NEMOK_SENSOR_DELAY > 0
    sigK.safeDelay(NEMOK_SENSOR_DELAY / 2);
    delay(BLINK_LENGTH_SHORT);
    #endif

    digitalWrite(LED_BUILTIN, LOW);
    #if NEMOK_SENSOR_DELAY > 0
    sigK.safeDelay(NEMOK_SENSOR_DELAY / 2);
    #else
    delay(BLINK_LENGTH_SHORT);
    #endif

    #ifdef NEED_LIB_ESPSIGK
    sigK.handle();
    #endif
}
