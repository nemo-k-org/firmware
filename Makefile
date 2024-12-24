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
