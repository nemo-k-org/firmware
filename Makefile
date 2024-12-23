.PHONY: build
build:
	pio run --environment esp01

.PHONY: upload
upload:
	pio run --environment esp01 -t upload

.PHONY: monitor
monitor:
	pio device monitor

.PHONY: clean
clean:
	rm -fR .pio/
