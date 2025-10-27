# Nemo-K firmware

The goal of the Nemo-K project is to give DIY people with little or no
programming skills and easy tools to create [SignalK](http://signalk.org/)
sensors.

This repository contains the Nemo-K firmware. Its basic charastertics are:
 * [PlatformIO](https://platformio.org/) build framework
 * ESP-01S with at least 1 Mb RAM (ESP8266) microcontroller (MCU)
 * [EspSigK](https://github.com/mplattu/EspSigK) SignalK client library
 * The firmware flavour and parameters are set in compile time and hard-coded into firmware

## Inputs and outputs

### Common environment variables

* `NEMOK_SENSOR_HOSTNAME` - sensor hostname
* `NEMOK_SENSOR_DELAY` - delay between sending sensor reading to server (milliseconds)
* `NEMOK_SENSOR_KEY` - SignalK data key for sensor reading (e.g. `environment.outside.temperature`)
* `NEMOK_WIFI_SSID` - the WiFi SSID of your yacht
* `NEMOK_WIFI_PASS` - the passkey to aforementioned WiFi
* `NEMOK_SIGNALK_SERVER_HOST` - IP or DNS address to your SignalK server
* `NEMOK_SIGNALK_SERVER_PORT` - port number of your SignalK server
* `NEMOK_SIGNALK_SERVER_TOKEN` - SignalK server token
* One (and only one) of the `NEMOK_SENSOR_*` variables (see below)

### Status codes

The MCU reports its status (typically an error condition) by blinking
the internal LED. The status code starts with long (1 sec) blink following
the status code (number of short 100 ms blinks).

Status codes are following:
 1. HTTP test passed
 2. HTTP test failed (HTTP status code is reported in serial output)

## Sensor types

Set one of these to `1` to choose sensor to build

#### Test sensor: `NEMOK_SENSOR_TEST_HTTP`

With this test sensor you can test building and uploading the firmware
to your MCU. When executed, the sensor tries to connect to your WiFi and 
make HTTP connection to `http://signalk.org`.

Test progress:
 1. When started, the MCU tries to connect to the given WiFi and
    get IPv4 address from the DHCP server. While reaching the network
    and DHCP server the built-in led blinks quickly.

    If the network connection does not succeed in 60 seconds the
    MCU restarts.
 2. The MCU makes HTTP GET to `http://signalk.org`.
 3. The response code is reported as status code (see "Status codes"
    above).
    
    If the WiFi of your yacht should have an internet connection
    you should get status code 1. If no internet connection is not
    available, both codes 1 and 2 are acceptable results.

* Required parameters:
   * `NEMOK_WIFI_SSID`
   * `NEMOK_WIFI_PASS`

#### Temperature sensor DS18B20: `NEMOK_SENSOR_TEMP_DS18B20`

This sensor uses DS18B20 sensor which sends temperature using
OneWire protocol. The sensor measures -55°C - +125°C and the
accuracy is ±0,5°C from -10°C - +85°C. The measuring range is suitable
for example to engine, engine room or salon temperatures.

* Required parameters:
   * `NEMOK_SENSOR_HOSTNAME`
   * `NEMOK_WIFI_SSID`
   * `NEMOK_WIFI_PASS`
* Optional parameters:
   * `NEMOK_SIGNALK_SERVER_HOST`
   * `NEMOK_SIGNALK_SERVER_PORT`
   * `NEMOK_SIGNALK_SERVER_TOKEN`
   * `NEMOK_SENSOR_DELAY`

#### Temperature sensor MAX6675: `NEMOK_SENSOR_TEMP_MAX6675`

This sensor uses MAX6675 chip and type-K thermocouple to measure
temperatures. MAX6675 has 12-bit 0,25°C resolution 0 - 1024°C. The
measuring range suits e.g. for exhaust pipes.

* Required parameters:
   * `NEMOK_SENSOR_HOSTNAME`
   * `NEMOK_WIFI_SSID`
   * `NEMOK_WIFI_PASS`
* Optional parameters:
   * `NEMOK_SIGNALK_SERVER_HOST`
   * `NEMOK_SIGNALK_SERVER_PORT`
   * `NEMOK_SIGNALK_SERVER_TOKEN`
   * `NEMOK_SENSOR_DELAY`

#### Location/GNSS sensor NEO 6M v2: `NEMOK_SENSOR_GNSS_NEO6MV2`

This sensor uses NEO 6M v2 breakout board to get geolocation
from GNSS systems supported by the chip.

* Required parameters:
   * `NEMOK_SENSOR_HOSTNAME`
   * `NEMOK_WIFI_SSID`
   * `NEMOK_WIFI_PASS`
* Optional parameters:
   * `NEMOK_SIGNALK_SERVER_HOST`
   * `NEMOK_SIGNALK_SERVER_PORT`
   * `NEMOK_SIGNALK_SERVER_TOKEN`
   * `NEMOK_SENSOR_DELAY`

If `NEMOK_SENSOR_KEY` is set, the sensor will update keys for selected
vessel. For example `NEMOK_SENSOR_KEY=flybridge` results e.g. following
keys sent to the server:
   * `vessels.flybridge.navigation.gnss.satellites`
   * `vessels.flybridge.navigation.gnss.antennaAltitude`
   * ...

If `NEMOK_SENSOR_KEY` is not set, following keys are updated:
   * `navigation.gnss.satellites`
   * `navigation.gnss.antennaAltitude`
   * `navigation.gnss.differentialAge`
   * `navigation.position`
   * `navigation.speedOverGround`
   * `navigation.gnss.horizontalDilution`
   * `navigation.headingTrue`

#### Location/GNSS sensor NEO 6M v2: `NEMOK_SENSOR_GNSS_NEO6MV2_UDP`

This sensor uses NEO 6M v2 breakout board to get geolocation
from GNSS systems supported by the chip. Instead of sending location
data directly to SignalK server like `NEMOK_SENSOR_GNSS_NEO6MV2`, it
sends raw NMEA-0183 data from the GPS unit to the given host/port.

This firmware can be used to send raw GPS data to SignalK server. In
this scenario the SignalK server must be configured to read NMEA-0183
data from UDP port:
1. Server > Data Connections
1. Add
1. Example configuration:
   * Data type: `NMEA 0183`
   * Enabled: `Yes`
   * Data logging: `No`
   * ID: e.g. `NEO6` (this string will used as a data source, see e.g. Data Browser view)
   * NMEA 0183 Source: `UDP`
   * Port: e.g. `4123` (this should be an unused port in the server and the value should
     equal with `NEMOK_SIGNALK_SERVER_PORT` parameter)
   * Validate checksum: `Yes`
   * Append checksum: `No`
   * Remove NULL characters: `No`
   * Ignore Sentences: leave empty
   * Override timestamps: `No`
1. Apply

The firmware can also be used to send data to any other device which is able to
process NMEA-0183 over UDP. Here is an example configuration for OpenCPN:
1. Settings > Connections
1. Add new connection
1. Example configuration:
   * `Network`
   * Network protocol: `UDP`
   * Data protocol: `NMEA 0183`
   * Address: `0.0.0.0`
   * DataPort: e.g. `4123` (this should be an unused port in the workstation and the value should
     equal with `NEMOK_SIGNALK_SERVER_PORT` parameter)
   * Description: e.g. `NEO6` (this is just to describe what this connection is for)
   * Receive Input on this Port: `checked`
   * Output on this port: `unchecked`
1. OK > Ok

In the setup above no SignalK server is required as the GPS sensor sends NMEA-0183 stream directly
to OpenCPN.

* Required parameters:
   * `NEMOK_SENSOR_HOSTNAME`
   * `NEMOK_WIFI_SSID`
   * `NEMOK_WIFI_PASS`
   * `NEMOK_SIGNALK_SERVER_HOST` (Destination of the UDP data, not necessary a SignalK server)
   * `NEMOK_SIGNALK_SERVER_PORT` (Destination of the UDP data, not necessary a SignalK server)
