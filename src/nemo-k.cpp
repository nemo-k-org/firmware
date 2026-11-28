#include <Arduino.h>
#include "../lib/tool_selector.cpp"

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
WiFiClient wiFiClient;
#include <ESP8266HTTPClient.h>

// ESP01S blue led is connected to GPIO 1 (pin 2)
#ifdef NEED_LED
#define LED_BUILTIN 2
#endif

#include "../lib/nemok_preferences.cpp"

#ifdef NEED_LIB_ESPSIGK
#include <EspSigK.h>
EspSigK sigK;
#endif

#define BLINK_LENGTH_SHORT 100
#define BLINK_LENGTH_LONG 1000

void blink_led(int length, int times) {
    for (int n=0; n < times; n++) {
#ifdef NEED_LED
        digitalWrite(LED_BUILTIN, HIGH);
#endif
        yield();
        delay(length);
#ifdef NEED_LED
        digitalWrite(LED_BUILTIN, LOW);
#endif
        yield();
        delay(length);
    }
}

void nemokPrint(const char * message) {
#ifdef NEED_SERIAL
    Serial.println(message);
#endif
}

void nemokPrint(String message) {
#ifdef NEED_SERIAL
    Serial.println(message);
#endif
}

void nemokPrint(int message) {
#ifdef NEED_SERIAL
    Serial.println(message);
#endif
}

void nemokPrint(double message) {
#ifdef NEED_SERIAL
    Serial.println(message);
#endif
}

void nemokPrint(const IPAddress& ip) {
#ifdef NEED_SERIAL
    char text[16];
    snprintf(text, sizeof(text), "%u.%u.%u.%u",
             ip[0], ip[1], ip[2], ip[3]);
    Serial.println(text);
#endif
}

void send_error_message(int error) {
    nemokPrint("Error: ");
    nemokPrint(error);

    int led_short = BLINK_LENGTH_SHORT;
    int led_long = BLINK_LENGTH_SHORT * 3;

    blink_led(led_short, 3);
    blink_led(led_long, 3);
    blink_led(led_short, 3);

    delay(BLINK_LENGTH_LONG);
    blink_led(BLINK_LENGTH_LONG, error);
    delay(BLINK_LENGTH_LONG);
}

bool wifi_initialise() {
    nemokPrint("Connecting to WiFi...");
    nemokPrint(nemokPrefs.get(NemokPreferences::SSID));

    unsigned int WAITING_TIME_MS = 60000;
    unsigned int wifi_timeout = millis() + WAITING_TIME_MS;

    WiFi.mode(WIFI_STA);
    WiFi.begin(nemokPrefs.get(NemokPreferences::SSID), nemokPrefs.get(NemokPreferences::SSIDPASS));

    while (WiFi.status() != WL_CONNECTED) {
        blink_led(BLINK_LENGTH_SHORT, 1);
        nemokPrint(".");
        if (wifi_timeout < millis()) {
            nemokPrint("Timeout");
            return false;
        }
    }

    nemokPrint(WiFi.localIP());

    return true;
}

#include "../lib/web_server.cpp"

#if NEMOK_SENSOR_TEST_HTTP > 0
#include "../lib/sensor_test_http.cpp"
#elif NEMOK_SENSOR_TEMP_DS18B20 > 0
#include "../lib/sensor_temp_ds18b20.cpp"
#elif NEMOK_SENSOR_TEMP_MAX6675 > 0
#include "../lib/sensor_temp_max6675.cpp"
#elif NEMOK_SENSOR_GNSS_NEO6MV2 > 0
#include "../lib/sensor_gnss_neo6mv2.cpp"
#elif NEMOK_SENSOR_GNSS_NEO6MV2_UDP > 0
#include "../lib/sensor_gnss_neo6mv2_udp.cpp"
#elif NEMOK_SENSOR_ADC_TANKLEVEL > 0
#include "../lib/sensor_adc_tanklevel.cpp"
#endif

void nemokDelay(unsigned long delayMs) {
#ifdef NEED_LIB_ESPSIGK
    sigK.safeDelay(delayMs, webServerHandle);
#else
    unsigned long start = millis();
    while (millis() - start < delayMs) {
        webServerHandle();
        yield();
    }
#endif
}

void setup () {
#ifdef NEED_SERIAL
    Serial.begin(115200);
#endif
#ifdef NEED_LED
    pinMode(LED_BUILTIN, OUTPUT);
#endif

    nemokPrefs.begin();

    #ifdef NEED_LIB_ESPSIGK
    sigK.configure(
        nemokPrefs.get(NemokPreferences::HOSTNAME),
        nemokPrefs.get(NemokPreferences::SSID),
        nemokPrefs.get(NemokPreferences::SSIDPASS),
        &wiFiClient
    );
    sigK.setPrintDebugSerial(true);

    const String& srvhost = nemokPrefs.get(NemokPreferences::SRVHOST);
    if (srvhost.length() > 0) {
        sigK.setServerHost(srvhost);
        nemokPrint("Setting server host to:");
        nemokPrint(srvhost);
    }

    int32_t srvport = nemokPrefs.get(NemokPreferences::SRVPORT);
    if (srvport > 0) {
        sigK.setServerPort(static_cast<uint16_t>(srvport));
        nemokPrint("Setting server port to:");
        nemokPrint(srvport);
    }

    const String& srvtoken = nemokPrefs.get(NemokPreferences::SRVTOKEN);
    if (srvtoken.length() > 0) {
        sigK.setServerToken(srvtoken);
    }

    sigK.begin();

    blink_led(BLINK_LENGTH_SHORT, 3);
    #endif

    nemokPrint("Starting sensor setup");
    sensorSetup();
    nemokPrint("Sensor setup finished");

    webServerSetup();
}

void loop () {
    nemokPrint("Starting sensor loop");
    sensorLoop();
    nemokPrint("Sensor loop finished");

    int32_t loopDelay = nemokPrefs.get(NemokPreferences::DELAY);

    #ifdef NEED_LED
    digitalWrite(LED_BUILTIN, HIGH);
    #endif

    if (loopDelay > 0) {
        nemokDelay(loopDelay / 2);
        delay(BLINK_LENGTH_SHORT);
    }

    #ifdef NEED_LED
    digitalWrite(LED_BUILTIN, LOW);
    #endif

    if (loopDelay > 0) {
        nemokDelay(loopDelay / 2);
        delay(BLINK_LENGTH_SHORT);
    }

    #ifdef NEED_LIB_ESPSIGK
    sigK.handle();
    #endif

    webServerHandle();

    nemokPrint("Loop finished");
}
