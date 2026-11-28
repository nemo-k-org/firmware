// Configuration web server (port 80) with mDNS advertisement.
// Included from src/nemo-k.cpp.
//
// Coexistence with EspSigK:
// * EspSigK runs its own web server on port 8080 with a global named `server`,
//   so do not reuse that name here.
// * There is only one mDNS responder (the global `MDNS` from ESP8266mDNS).
//   EspSigK calls MDNS.begin(hostname) and adds an http/tcp service for port
//   8080. We reuse that responder instead of starting a second one, and give
//   our service a distinct instance name, because the responder refuses a
//   second service with the same name, service and protocol.
// * EspSigK never calls MDNS.update(), which the responder needs in order to
//   finish probing and start answering. webServerHandle() calls it for both.

#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

#define NEMOK_WEB_SERVER_PORT 80
#define NEMOK_WEB_SERVER_ADMIN_USER "admin"
#define NEMOK_WEB_SERVER_REALM "Nemo-K"
#define NEMOK_WEB_SERVER_MAX_VALUE_LENGTH 64

ESP8266WebServer nemokWebServer(NEMOK_WEB_SERVER_PORT);

bool webServerAuthenticate() {
    if (nemokPrefs.get(NemokPreferences::ADMPASS).length() == 0) {
        return true;
    }

    if (nemokWebServer.authenticate(NEMOK_WEB_SERVER_ADMIN_USER, nemokPrefs.get(NemokPreferences::ADMPASS).c_str())) {
        return true;
    }

    nemokWebServer.requestAuthentication(BASIC_AUTH, NEMOK_WEB_SERVER_REALM);
    return false;
}

String webServerHtmlEscape(const String& value) {
    String escaped;
    escaped.reserve(value.length());
    for (unsigned int n = 0; n < value.length(); n++) {
        char c = value[n];
        switch (c) {
            case '&': escaped += F("&amp;"); break;
            case '<': escaped += F("&lt;"); break;
            case '>': escaped += F("&gt;"); break;
            case '"': escaped += F("&quot;"); break;
            case '\'': escaped += F("&#39;"); break;
            default: escaped += c;
        }
    }
    return escaped;
}

bool webServerIsInteger(const String& value) {
    unsigned int start = (value.length() > 0 && value[0] == '-') ? 1 : 0;
    unsigned int digits = value.length() - start;
    if (digits < 1 || digits > 9) return false;
    for (unsigned int n = start; n < value.length(); n++) {
        if (!isDigit(value[n])) return false;
    }
    return true;
}

void webServerAppendField(String& page, const char* name, const String& value, const char* inputType) {
    page += F("<p><label>");
    page += name;
    page += F("<br><input type=\"");
    page += inputType;
    page += F("\" name=\"");
    page += name;
    page += F("\" value=\"");
    page += webServerHtmlEscape(value);
    page += F("\"></label></p>");
}

void webServerSendForm(const String& message) {
    String page;
    page.reserve(2048);

    page += F("<!DOCTYPE html><html><head><meta charset=\"utf-8\">"
              "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">"
              "<title>Nemo-K</title></head><body><h1>");
    page += webServerHtmlEscape(nemokPrefs.get(NemokPreferences::HOSTNAME));
    page += F("</h1>");

    if (message.length() > 0) {
        page += F("<p><b>");
        page += message;
        page += F("</b></p>");
    }

    page += F("<form method=\"post\" action=\"/\">");

    for (int n = 0; n < NemokPreferences::STRING_KEY_COUNT; n++) {
        NemokPreferences::StringKey key = static_cast<NemokPreferences::StringKey>(n);
        if (!nemokPrefs.isDefined(key)) continue;

        webServerAppendField(page, NemokPreferences::keyName(key), nemokPrefs.get(key), "text");
    }

    for (int n = 0; n < NemokPreferences::INT_KEY_COUNT; n++) {
        NemokPreferences::IntKey key = static_cast<NemokPreferences::IntKey>(n);
        if (!nemokPrefs.isDefined(key)) continue;

        webServerAppendField(page, NemokPreferences::keyName(key), String(nemokPrefs.get(key)), "number");
    }

    page += F("<p><input type=\"submit\" value=\"Save\"></p></form>"
              "<p>Restart the sensor to take the new values into use.</p>"
              "</body></html>");

    nemokWebServer.send(200, "text/html", page);
}

void webServerHandleRoot() {
    if (!webServerAuthenticate()) return;

    webServerSendForm("");
}

void webServerHandleSave() {
    if (!webServerAuthenticate()) return;

    bool writeFailed = false;
    bool tooLongValues = false;

    String invalidFields;

    for (int n = 0; n < NemokPreferences::STRING_KEY_COUNT; n++) {
        NemokPreferences::StringKey key = static_cast<NemokPreferences::StringKey>(n);
        const char* name = NemokPreferences::keyName(key);
        if (!nemokPrefs.isDefined(key) || !nemokWebServer.hasArg(name)) continue;

        String value = nemokWebServer.arg(name);
        if (value.length() > NEMOK_WEB_SERVER_MAX_VALUE_LENGTH) {
            tooLongValues = true;
            continue;
        }

        if (value != nemokPrefs.get(key)) {
            if (!nemokPrefs.set(key, value)) writeFailed = true;
        }
    }

    for (int n = 0; n < NemokPreferences::INT_KEY_COUNT; n++) {
        NemokPreferences::IntKey key = static_cast<NemokPreferences::IntKey>(n);
        const char* name = NemokPreferences::keyName(key);
        if (!nemokPrefs.isDefined(key) || !nemokWebServer.hasArg(name)) continue;

        String value = nemokWebServer.arg(name);
        value.trim();
        if (!webServerIsInteger(value)) {
            if (invalidFields.length() > 0) invalidFields += F(", ");
            invalidFields += name;
            continue;
        }

        int32_t intValue = value.toInt();
        if (intValue != nemokPrefs.get(key)) {
            if (!nemokPrefs.set(key, intValue)) writeFailed = true;
        }
    }

    String message;
    if (tooLongValues) {
        message += F("Error: some values were too long. These values were not saved. ");
    }

    if (writeFailed) {
        message += F("Error: some values could not be written. ");
    }
    else if (invalidFields.length() > 0) {
        message += F("Saved, except these fields which are not valid integers: ");
        message += invalidFields;
    }
    else {
        message += F("Saved.");
    }

    webServerSendForm(message);
}

void webServerHandleNotFound() {
    if (!webServerAuthenticate()) return;

    nemokWebServer.send(404, "text/plain", "404: Not found");
}

void webServerSetupMdns() {
    if (!MDNS.isRunning()) {
        if (!MDNS.begin(nemokPrefs.get(NemokPreferences::HOSTNAME).c_str())) {
            nemokPrint("Error starting mDNS responder");
            return;
        }
    }

    String instanceName = nemokPrefs.get(NemokPreferences::HOSTNAME) + "-config";
    if (MDNS.addService(instanceName.c_str(), "http", "tcp", NEMOK_WEB_SERVER_PORT)) {
        nemokPrint("mDNS: web server advertised as http://" + nemokPrefs.get(NemokPreferences::HOSTNAME) + ".local/");
    }
    else {
        nemokPrint("Error adding mDNS service for the web server");
    }
}

void webServerSetup() {
    nemokWebServer.on("/", HTTP_GET, webServerHandleRoot);
    nemokWebServer.on("/", HTTP_POST, webServerHandleSave);
    nemokWebServer.onNotFound(webServerHandleNotFound);
    nemokWebServer.begin();
    nemokPrint("Web server started on port 80");

    webServerSetupMdns();
}

void webServerHandle() {
    nemokWebServer.handleClient();
    MDNS.update();
}
