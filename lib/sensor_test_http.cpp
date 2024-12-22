#define STATUS_CODE_SENSOR_TEST_HTTP_PASSED 1
#define STATUS_CODE_SENSOR_TEST_HTTP_FAILED 2

void report_status(int status_code, bool report_forever = false) {
    do {
        Serial.print("Status code: ");
        Serial.print(status_code);

        blink_led(BLINK_LENGTH_LONG, 1);
        blink_led(BLINK_LENGTH_SHORT, status_code);

        Serial.println("\t(halted)");
    } while (report_forever);
}

void sensorSetup() {
    if (! wifi_initialise()) {
        Serial.println("Could not initialise WiFi, restarting...");
        ESP.restart();
    }

}

void sensorLoop() {
    HTTPClient http;

    http.begin(wiFiClient, (const char *) "http://signalk.org");
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
}
