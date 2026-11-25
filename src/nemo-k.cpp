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

#include <Preferences.h>
Preferences prefs;
String preferencesSsid;
String preferencesPass;
String preferencesHostname;
String preferencesSrvhost;
uint16_t preferencesSrvport;
String preferencesSrvtoken;
String preferencesKey;
int preferencesDelay;

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
    nemokPrint(preferencesSsid);

    unsigned int WAITING_TIME_MS = 60000;
    unsigned int wifi_timeout = millis() + WAITING_TIME_MS;

    WiFi.mode(WIFI_STA);
    WiFi.begin(preferencesSsid, preferencesPass);

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
    sigK.safeDelay(delayMs);
#else
    delay(delayMs);
#endif
}

void setup () {
#ifdef NEED_SERIAL
    Serial.begin(115200);
#endif
#ifdef NEED_LED
    pinMode(LED_BUILTIN, OUTPUT);
#endif

    prefs.begin("nemok");

    preferencesSsid = prefs.getString("ssid", "nemok");
    preferencesPass = prefs.getString("pass", "nemok");
    preferencesHostname = prefs.getString("hostname", "sensor");
    preferencesSrvhost = prefs.getString("srvhost", "nemok.org");
    int32_t preferencesSrvportLong = prefs.getInt("srvport", 3000);
    preferencesSrvport = static_cast<uint16_t>(preferencesSrvportLong);
    Serial.println(preferencesSrvportLong);
    Serial.println(preferencesSrvport);
    preferencesSrvtoken = prefs.getString("srvtoken");
    preferencesKey = prefs.getString("key", "undefined");
    preferencesDelay = prefs.getInt("delay", 1000);

    #ifdef NEED_LIB_ESPSIGK
    sigK.configure(preferencesHostname, preferencesSsid, preferencesPass, &wiFiClient);
    sigK.setPrintDebugSerial(true);

    if (preferencesSrvhost.length() > 0) {
        sigK.setServerHost(preferencesSrvhost);
        nemokPrint("Setting server host to:");
        nemokPrint(preferencesSrvhost);
    }

    if (preferencesSrvport > 0) {
        sigK.setServerPort(preferencesSrvport);
        nemokPrint("Setting server port to:");
        nemokPrint(preferencesSrvport);
    }

    if (preferencesSrvtoken.length() > 0) {
        sigK.setServerToken(preferencesSrvtoken);
    }

    sigK.begin();

    blink_led(BLINK_LENGTH_SHORT, 3);
    #endif

    nemokPrint("Starting sensor setup");
    sensorSetup();
    nemokPrint("Sensor setup finished");
}

void loop () {
    nemokPrint("Starting sensor loop");
    sensorLoop();
    nemokPrint("Sensor loop finished");

    #ifdef NEED_LED
    digitalWrite(LED_BUILTIN, HIGH);
    #endif

    if (preferencesDelay > 0) {
        nemokDelay(preferencesDelay / 2);
        delay(BLINK_LENGTH_SHORT);
    }

    #ifdef NEED_LED
    digitalWrite(LED_BUILTIN, LOW);
    #endif

    if (preferencesDelay > 0) {
        nemokDelay(preferencesDelay / 2);
        delay(BLINK_LENGTH_SHORT);
    }

    #ifdef NEED_LIB_ESPSIGK
    sigK.handle();
    #endif

    nemokPrint("Loop finished");
}
