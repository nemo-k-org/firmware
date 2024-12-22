.PHONY: build_wemosd1
build_wemosd1:
	pio run --environment wemosd1

.PHONY: build_esp01
build_esp01:
	pio run --environment esp01

.PHONY: upload_wemosd1
upload_wemosd1:
	pio run --environment wemosd1 -t upload

.PHONY: upload_esp01
upload_esp01:
	pio run --environment esp01 -t upload

.PHONY: monitor
monitor:
	pio device monitor

.PHONY: clean
clean:
	rm -fR .pio/
