FIRMWARE_BIN:=.pio/build/esp01/firmware.bin

dist:
	mkdir dist

dist/firmware_adc_tanklevel.bin: lib/sensor_adc_tanklevel.cpp lib/tool_selector.cpp src/nemo-k.cpp platformio.ini dist
	NEMOK_SENSOR_ADC_TANKLEVEL=1 ./firmware-build.sh
	cp $(FIRMWARE_BIN) dist/firmware_adc_tanklevel.bin

dist/firmware_gnss_neo6mv2_udp.bin: lib/sensor_gnss_neo6mv2_udp.cpp lib/tool_selector.cpp src/nemo-k.cpp platformio.ini dist
	NEMOK_SENSOR_GNSS_NEO6MV2_UDP=1 ./firmware-build.sh
	cp $(FIRMWARE_BIN) dist/firmware_gnss_neo6mv2_udp.bin

dist/firmware_gnss_neo6mv2.bin: lib/sensor_gnss_neo6mv2.cpp lib/tool_selector.cpp src/nemo-k.cpp platformio.ini dist
	NEMOK_SENSOR_GNSS_NEO6MV2=1 ./firmware-build.sh
	cp $(FIRMWARE_BIN) dist/firmware_gnss_neo6mv2.bin

dist/firmware_temp_ds18b20.bin: lib/sensor_temp_ds18b20.cpp lib/tool_selector.cpp src/nemo-k.cpp platformio.ini dist
	NEMOK_SENSOR_TEMP_DS18B20=1 ./firmware-build.sh
	cp $(FIRMWARE_BIN) dist/firmware_temp_ds18b20.bin

dist/firmware_temp_max6675.bin: lib/sensor_temp_max6675.cpp lib/tool_selector.cpp src/nemo-k.cpp platformio.ini dist
	NEMOK_SENSOR_TEMP_MAX6675=1 ./firmware-build.sh
	cp $(FIRMWARE_BIN) dist/firmware_temp_max6675.bin

dist/firmware_test_http.bin: lib/sensor_test_http.cpp lib/tool_selector.cpp src/nemo-k.cpp platformio.ini dist
	NEMOK_SENSOR_TEST_HTTP=1 ./firmware-build.sh
	cp $(FIRMWARE_BIN) dist/firmware_test_http.bin

all: dist/firmware_adc_tanklevel.bin \
	dist/firmware_gnss_neo6mv2_udp.bin \
	dist/firmware_gnss_neo6mv2.bin	\
	dist/firmware_temp_ds18b20.bin \
	dist/firmware_temp_max6675.bin \
	dist/firmware_test_http.bin

.PHONY: build
build:
	./firmware-build.sh

.PHONY: upload
upload:
	./firmware-upload.sh

.PHONY: monitor
monitor:
	pio device monitor

.PHONY: clean
clean:
	rm -fR .pio/
	rm -fR dist/

