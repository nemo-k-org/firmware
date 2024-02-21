#include "../lib/tool_selector.cpp"

#ifdef NEED_WIFI
#include <ESP8266WiFi.h>
#endif

#ifdef NEED_HTTP_CLIENT
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#endif

#define STATUS_CODE_SENSOR_TEST_HTTP_PASSED 1
#define STATUS_CODE_SENSOR_TEST_HTTP_FAILED 2

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

void report_status(int status_code, bool report_forever = false) {
    do {
        Serial.print("Status code: ");
        Serial.print(status_code);

        blink_led(BLINK_LENGTH_LONG, 1);
        blink_led(BLINK_LENGTH_SHORT, status_code);

        Serial.println("\t(halted)");
    } while (report_forever);
}

#ifdef NEED_WIFI
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
#endif

void setup () {
    Serial.begin(115200);
    pinMode(LED_BUILTIN, OUTPUT);

    #ifdef NEED_WIFI
    if (! wifi_initialise()) {
        Serial.println("Could not initialise WiFi, restarting...");
        ESP.restart();
    }
    #endif
}

void loop () {
    #ifdef NEMOK_SENSOR_TEST_HTTP
    WiFiClient client;
    HTTPClient http;

    http.begin(client, (const char *) "http://signalk.org");
    int httpResponseCode = http.GET();

    if (httpResponseCode == 200) {
        report_status(STATUS_CODE_SENSOR_TEST_HTTP_PASSED, true);
    }
    else {
        Serial.print("HTTP GET to http://signalk.org returned a response code: ");
        Serial.print(httpResponseCode);
        Serial.println(", trying again");
        report_status(STATUS_CODE_SENSOR_TEST_HTTP_FAILED, true);
    }
    #endif
}
