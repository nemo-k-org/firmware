# Nemo-K firmware

The goal of the Nemo-K project is to give DIY people with little or no
programming skills and easy tools to create [SignalK](http://signalk.org/)
sensors.

This repository contains the Nemo-K firmware. Its basic charastertics are:
 * [PlatformIO](https://platformio.org/) build framework
 * ESP-01S with at least 1 Mb RAM (ESP8266) microcontroller (MCU)
 * [EspSigK](https://github.com/mplattu/EspSigK) SignalK client library
 * The firmware flavour and parameters are written to a LittleFS partition which is read by [vshymanskyy/Preferences](https://github.com/vshymanskyy/Preferences) library

## Inputs and outputs

NemoK firmware uses
* build-time symbols to select the sensor type to build
* configuration variables to store settings of an invidivual sensor

The build-time symbols have common prefix `NEMOK_SENSOR_`. These symbols are
listed below by each sensor.

The configuration variable names refer to Preferences variables. Each variable
is stored to LittleFS in a separate file. The filesystem is created in the
web frontend but the values can be later manipulated by the sensor firmware.
This approach allows one to update the firmware only.

The sensor firmware/Preferences library stores these settings using following
path scheme: `/nvs/nemok/[variable name]`. In other words the variable `hostname`
is stored at `/nvs/nemok/hostname`. The EspSigK library utilises Preferences
as well and it uses path `/nvs/EspSigK/`.

The maximum length of Preferences variable name is 8, which explains the
brevity of the names.

### Common configuration variables

Following configuration variables are used in almost all sensors:

* `hostname` - sensor hostname
* `admpass` - password for accessing sensor configuration web server (leave empty for
   no authentication)
* `delay` - delay between sending sensor reading to server (milliseconds)
* `key` - SignalK data key for sensor reading (e.g. `environment.outside.temperature`)
* `ssid` - the WiFi SSID of your yacht
* `ssidpass` - the passkey to aforementioned WiFi
* `srvhost` - IP or DNS address to your SignalK server
* `srvport` - port number of your SignalK server
* `srvtoken` - SignalK server token

### Status codes

The MCU reports its status (typically an error condition) by blinking
the internal LED. The status code starts with long (1 sec) blink following
the status code (number of short 100 ms blinks).

Status codes are following:
 1. HTTP test passed
 2. HTTP test failed (HTTP status code is reported in serial output)

### Sensor configuration web server

Each sensor has a configuration web server, which can be accessed with URL
`http://[hostname].local`. So if the value of configuration variable `hostname`
is `enginetemp`, you can access the configuration web server with url
`http://enginetemp.local` while being in the same wireless network as the
sensor.

If the variable `admpass` is set, the password to access the server is that value.
The username is always `admin`.

If the variable `admpass` is an empty string, no username or password is required
to access the configuration web server.

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

* Required configuration variables:
   * `ssid`
   * `ssidpass`

#### Temperature sensor DS18B20: `NEMOK_SENSOR_TEMP_DS18B20`

This sensor uses DS18B20 sensor which sends temperature using
OneWire protocol. The sensor measures -55°C - +125°C and the
accuracy is ±0,5°C from -10°C - +85°C. The measuring range is suitable
for example to engine, engine room or salon temperatures.

* Configuration variables:
   * `hostname`
   * `admpass`
   * `ssid`
   * `ssidpass`
   * `srvhost`
   * `srvport`
   * `srvtoken`
   * `key`
   * `delay`

#### Temperature sensor MAX6675: `NEMOK_SENSOR_TEMP_MAX6675`

This sensor uses MAX6675 chip and type-K thermocouple to measure
temperatures. MAX6675 has 12-bit 0,25°C resolution 0 - 1024°C. The
measuring range suits e.g. for exhaust pipes.

* Configuration variables:
   * `hostname`
   * `admpass`
   * `ssid`
   * `ssidpass`
   * `srvhost`
   * `srvport`
   * `srvtoken`
   * `key`
   * `delay`

#### Location/GNSS sensor NEO 6M v2: `NEMOK_SENSOR_GNSS_NEO6MV2`

This sensor uses NEO 6M v2 breakout board to get geolocation
from GNSS systems supported by the chip.

* Configuration variables:
   * `hostname`
   * `admpass`
   * `ssid`
   * `ssidpass`
   * `srvhost`
   * `srvport`
   * `srvtoken`
   * `key`
   * `delay`

If configuration variable `key` is set, the sensor will update keys for selected
vessel. For example configuration `key=flybridge` results e.g. following
keys sent to the server:
   * `vessels.flybridge.navigation.gnss.satellites`
   * `vessels.flybridge.navigation.gnss.antennaAltitude`
   * ...

If `key` is not set, following keys are updated:
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
     equal with the value of sensor variable `srvport`)
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
     equal with the value of sensor variable `srvport`)
   * Description: e.g. `NEO6` (this is just to describe what this connection is for)
   * Receive Input on this Port: `checked`
   * Output on this port: `unchecked`
1. OK > Ok

In the setup above no SignalK server is required as the GPS sensor sends NMEA-0183 stream directly
to OpenCPN.

* Configuration variables:
   * `hostname`
   * `admpass`
   * `ssid`
   * `ssidpass`
   * `srvhost` (Destination of the UDP data, not necessarily a SignalK server)
   * `srvport` (Destination of the UDP data, not necessarily a SignalK server)

#### Tank Level Sensor Using ADS1115 AD Converter `NEMOK_SENSOR_ADC_TANKLEVEL`

This sensor reads resistance (0-300 ohm) from standard tank gauges. Any sensor
sending readings in this scale can and must be calibrated, since the readings may
easily be ±10 ohms.

Here are two examples of gauges known to work:
* Wema black water gauge signal is 240-30 ohm (0-100%, US standard)
* Wema water/fuel gauge signal is 0-180 ohm (0-100%, European standard)

* Configuration variables:
   * `hostname`
   * `admpass`
   * `ssid`
   * `ssidpass`
   * `srvhost`
   * `srvport`
   * `srvtoken`
   * `key` - SignalK data key for sensor reading (e.g. `tanks.freshWater.currentLevel`)
   * `delay`
   * `adctank0` - Resistance in ohms when the tank is empty (0%), see calibration
      instructions below
   * `adctank1` - Resistance in ohms when the tank is full (100%), see calibration
      instructions below

**Important! Read this before connecting the gauge to the sensor!** In case your gauge is
connected to the electrical ground (e.g. your tank is made of conductive material) identify
whether either of the gauge wires is connected to the ground. Keep the gauge installed to
the tank while making following measurements with a multimeter.
1. Measure connectivity between either of the gauge wires and the electrical ground which you
   are going to use to power the NemoK sensor.
1. Measure potential difference (voltage) between either of the gauge wires and the electrical
   ground which you are going to use to power the NemoK sensor.

In the best possible (and most typical) scenario there is no connectivity and no potential
difference. If this is your result, you can connect the gauge wires to the NemoK sensor either
way.

If you observe voltage between either of the wires and power ground, you have to find the cause
before proceeding.

In case of connectivity (a measurable resistance) between gauge wires and ground you should
try to find which of the wires is directly connected to the ground. The manufacturer may have
marked this wire as black.
* The gauge ground wire should have zero ohms resistance regardless the position of the
  gauge float when connected to the ground.
* The resistance between the gauge signal wire and ground should alter when the position of the
  gauge float is changed.

If you have identified gauge ground wire make sure you connect this wire to SEND GND screw
terminal.

**Calibration**
1. For best possible results take the gauge out from the tank so you can adjust the float
   manually.
1. Start by using resistance values given by the gauge manufacturer. If your gauge should give
   240 ohms with level 0% and 30 ohms with level 100%, set `adctank0` to 240
   and `adctank1` to 30.
1. After measuring the sensor (see above) connect the gauge to the NemoK level sensor.
1. Power up the NemoK sensor and observe in the SignalK server Data Browser view the values
   the sensor sends. The path specified by `key` (e.g. `tanks.freshWater.currentLevel`)
   should stay between 0.00 - 1.00. The real measured resistance can be read from a debug key
   (in this example `tanks.freshWater.currentLevel.debug.resistance`). You can use this
   value to set `adctank0` and `adctank1`.

## Building

To build all firmwares locally:
* PlatformIO installed: `make all`
* Docker: ``docker run -v `pwd`:/opt/build --name pio-build infinitecoding/platformio-for-ci:latest /opt/build/ci-make-all.sh``

To build all firmwares on GitHub:
* Run workflow [build.yaml](https://github.com/nemo-k-org/firmware/actions/workflows/build.yaml) manually
