.PHONY=build
build:
	pio run --environment wemosd1

.PHONY=upload
upload:
	pio run --environment wemosd1 -t upload

.PHONY=monitor
monitor:
	pio device monitor

.PHONY=clean
clean:
	rm -fR .pio/
