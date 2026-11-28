# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project

Nemo-K firmware: PlatformIO/Arduino firmware for ESP-01S (ESP8266, 1 MB, `esp01_1m` board) SignalK sensors. One source tree produces several firmware flavours, one per sensor type. The README is end-user documentation (sensor wiring, configuration variables, calibration) and should be kept in sync when sensors or variables change.

## Commands

There are no tests or linters; "building" means compiling with PlatformIO.

- Build all flavours into `dist/firmware_<sensor>.bin`: `make all`
- Build all in Docker (as CI does): ``docker run -v `pwd`:/opt/build --name pio-build infinitecoding/platformio-for-ci:latest /opt/build/ci-make-all.sh``
- Build one flavour: `NEMOK_SENSOR_TEMP_DS18B20=1 ./firmware-build.sh` (output: `.pio/build/esp01/firmware.bin`)
- Upload one flavour to a connected MCU: `NEMOK_SENSOR_TEMP_DS18B20=1 ./firmware-upload.sh`
- Serial monitor (115200): `make monitor`
- Clean: `make clean`

CI ([.github/workflows/build.yaml](.github/workflows/build.yaml)) runs `ci-make-all.sh` on manual dispatch or pushes to the `github-actions` branch and uploads `dist/` as an artifact.

Local per-device upload scripts named `upload_sensor_*` are gitignored (they hold WiFi credentials); never commit them.

## Architecture

**Flavour selection is compile-time.** Exactly one `NEMOK_SENSOR_*` env var must be `1`. [firmware-fill-vars.sh](firmware-fill-vars.sh) defaults unset vars to `0` and refuses zero or multiple selections; [platformio.ini](platformio.ini) passes each env var through as a `-D` build flag via `${sysenv.…}`.

**Single translation unit via `#include` of .cpp files.** [src/nemo-k.cpp](src/nemo-k.cpp) is the only compiled source. It includes:
1. [lib/tool_selector.cpp](lib/tool_selector.cpp) — maps the selected sensor to feature macros `NEED_LIB_ESPSIGK`, `NEED_LED`, `NEED_SERIAL`, which gate the EspSigK client, LED blinking, and serial output throughout `nemo-k.cpp`.
2. Exactly one `lib/sensor_*.cpp`, chosen by an `#if/#elif` chain. Each sensor file implements `sensorSetup()` and `sensorLoop()` and directly uses globals defined earlier in `nemo-k.cpp` (`prefs`, `sigK`, `wiFiClient`, `preferencesKey`, `preferencesSrvhost`, `nemokPrint()`, `blink_led()`, `send_error_message()`, `wifi_initialise()`, …). Include order therefore matters.

**Configuration web server.** [lib/web_server.cpp](lib/web_server.cpp) runs an `ESP8266WebServer` on port 80 in every flavour (`nemokWebServer`; EspSigK has its own global `server` on port 8080, so avoid that name). `handleClient()` is called from `loop()` and inside `nemokDelay()`, so code that blocks for a long time makes the server unresponsive. The server is advertised over mDNS as `<hostname>.local`. There is only one global `MDNS` responder, shared with EspSigK: reuse it if it is already running, and keep calling `MDNS.update()` (EspSigK never calls it).

`setup()` reads common settings from Preferences, configures/starts EspSigK if needed, then calls `sensorSetup()`. `loop()` calls `sensorLoop()`, waits `delay` ms (blinking the LED), and calls `sigK.handle()`.

**Runtime configuration lives in LittleFS, not in the firmware.** Settings are read with the `vshymanskyy/Preferences` library under namespace `nemok` (stored as `/nvs/nemok/<name>`; EspSigK uses `/nvs/EspSigK/`). The filesystem is created by a separate web frontend, so firmware can be updated without losing settings. Preference names are limited to 8 characters. Common names: `hostname`, `ssid`, `pass`, `srvhost`, `srvport`, `srvtoken`, `key`, `delay`; sensors may read their own (e.g. `adctank0`/`adctank1`). Older code used build-time `NEMOK_*` symbols for these values — those are obsolete.

SignalK values are sent in SI units (e.g. temperatures in Kelvin, tank levels as 0–1 ratios).

## Adding a sensor type

Touch all of these together:
- New `lib/sensor_<name>.cpp` with `sensorSetup()`/`sensorLoop()`
- `#elif` branch in [src/nemo-k.cpp](src/nemo-k.cpp) and feature macros in [lib/tool_selector.cpp](lib/tool_selector.cpp)
- `-DNEMOK_SENSOR_<NAME>=${sysenv.…}` in [platformio.ini](platformio.ini), plus any `lib_deps`
- Default-to-0 block and the selection-count loop in [firmware-fill-vars.sh](firmware-fill-vars.sh)
- `dist/firmware_<name>.bin` target and `all` entry in [Makefile](Makefile)
- Sensor section in [README.md](README.md) listing its configuration variables
