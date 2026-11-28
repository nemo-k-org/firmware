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

ESP8266WebServer nemokWebServer(NEMOK_WEB_SERVER_PORT);

// HTTP basic authentication: user "admin", password from preferencesAdminPass.
// An empty password disables authentication. Call at the start of every handler;
// if it returns false, the 401 response has already been sent.
bool webServerAuthenticate() {
    if (preferencesAdminPass.length() == 0) {
        return true;
    }

    if (nemokWebServer.authenticate(NEMOK_WEB_SERVER_ADMIN_USER, preferencesAdminPass.c_str())) {
        return true;
    }

    nemokWebServer.requestAuthentication(BASIC_AUTH, NEMOK_WEB_SERVER_REALM);
    return false;
}

void webServerHandleRoot() {
    if (!webServerAuthenticate()) return;

    nemokWebServer.send(200, "text/html",
        "<!DOCTYPE html><html><head><meta charset=\"utf-8\"><title>Nemo-K</title></head>"
        "<body><h1>Hello world</h1></body></html>");
}

void webServerHandleNotFound() {
    if (!webServerAuthenticate()) return;

    nemokWebServer.send(404, "text/plain", "404: Not found");
}

void webServerSetupMdns() {
    if (!MDNS.isRunning()) {
        if (!MDNS.begin(preferencesHostname.c_str())) {
            nemokPrint("Error starting mDNS responder");
            return;
        }
    }

    String instanceName = preferencesHostname + "-config";
    if (MDNS.addService(instanceName.c_str(), "http", "tcp", NEMOK_WEB_SERVER_PORT)) {
        nemokPrint("mDNS: web server advertised as http://" + preferencesHostname + ".local/");
    }
    else {
        nemokPrint("Error adding mDNS service for the web server");
    }
}

void webServerSetup() {
    nemokWebServer.on("/", HTTP_GET, webServerHandleRoot);
    nemokWebServer.onNotFound(webServerHandleNotFound);
    nemokWebServer.begin();
    nemokPrint("Web server started on port 80");

    webServerSetupMdns();
}

void webServerHandle() {
    nemokWebServer.handleClient();
    MDNS.update();
}
