# Nemo-K firmware

The goal of the Nemo-K project is to give DIY people with little or no
programming skills and easy tools to create [SignalK](http://signalk.org/)
sensors.

This repository contains the Nemo-K firmware. Its basic charastertics are:
 * [PlatformIO](https://platformio.org/) build framework
 * [Wemos D1 mini (ESP8266)](https://www.wemos.cc/en/latest/d1/d1_mini.html) microcontroller (MCU)
 * [EspSigK](https://github.com/mplattu/EspSigK) SignalK client library
 * The firmware flavour and parameters are set in compile time and hard-coded into firmware

## Inputs and outputs

### Common environment variables

* `NEMOK_WIFI_SSID` - the WiFi SSID of your yacht
* `NEMOK_WIFI_PASS` - the passkey to aforementioned WiFi
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

Required parameters: `NEMOK_WIFI_SSID`, `NEMOK_WIFI_PASS`
