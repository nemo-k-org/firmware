#include <TinyGPSPlus.h>

TinyGPSPlus gps;

String _neo6mv2_satellites;
String _neo6mv2_altitude;
String _neo6mv2_differentialAge;
String _neo6mv2_position;
String _neo6mv2_speedOverGround;
String _neo6mv2_horizontalDilution;
String _neo6mv2_headingTrue;

const int _neo6mv2_locationJsonBufferSize = 1024;
char _neo6mv2_locationJsonBuffer[_neo6mv2_locationJsonBufferSize];

void sensorSetup()
{
    Serial.begin(9600);

    String vesselPrefix = "";
    if (strcmp(NEMOK_SENSOR_KEY, "") != 0) {
        vesselPrefix = "vessels." + String(NEMOK_SENSOR_KEY) + ".";
    }

    _neo6mv2_satellites = vesselPrefix + "navigation.gnss.satellites";
    _neo6mv2_altitude = vesselPrefix + "navigation.gnss.antennaAltitude";
    _neo6mv2_differentialAge = vesselPrefix + "navigation.gnss.differentialAge";
    _neo6mv2_position = vesselPrefix + "navigation.position";
    _neo6mv2_speedOverGround = vesselPrefix + "navigation.speedOverGround";
    _neo6mv2_horizontalDilution = vesselPrefix + "navigation.gnss.horizontalDilution";
    _neo6mv2_headingTrue = vesselPrefix + "navigation.headingTrue";
}

void sensorLoop()
{
    while (Serial.available() > 0) {
        if (gps.encode(Serial.read())) {
            if (gps.satellites.isValid()) {
                sigK.addDeltaValue(_neo6mv2_satellites, static_cast<int>(gps.satellites.value()));
            }

            if (gps.location.isValid()) {
                uint32_t ageInSeconds = gps.location.age() / 1000;
                sigK.addDeltaValue(_neo6mv2_differentialAge, static_cast<int>(ageInSeconds));

                if (gps.altitude.isValid()) {
                    sigK.addDeltaValue(_neo6mv2_altitude, gps.altitude.meters());

                    snprintf(_neo6mv2_locationJsonBuffer, _neo6mv2_locationJsonBufferSize,
                        "{\"longitude\":%.12f,\"latitude\":%.12f,\"altitude\":%.1f}",
                        gps.location.lng(), gps.location.lat(), gps.altitude.meters()
                    );
                } else {
                    snprintf(_neo6mv2_locationJsonBuffer, _neo6mv2_locationJsonBufferSize,
                        "{\"latitude\":%.12f,\"longitude\":%.12f}",
                        gps.location.lat(), gps.location.lng()
                    );
                }
                sigK.addDeltaValue(_neo6mv2_position, static_cast<const char *>(_neo6mv2_locationJsonBuffer), true);
            }

            if (gps.speed.isValid()) {
                sigK.addDeltaValue(_neo6mv2_speedOverGround, gps.speed.mps());
            }

            if (gps.hdop.isValid()) {
                sigK.addDeltaValue(_neo6mv2_horizontalDilution, gps.hdop.hdop());
            }

            if (gps.course.isValid()) {
                double courseInRadians = gps.course.value() * 0.01745329;
                sigK.addDeltaValue(_neo6mv2_headingTrue, courseInRadians);
            }

            sigK.sendDelta();
        }
    }
}
