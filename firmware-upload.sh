#!/bin/sh

. ./firmware-fill-vars.sh

pio run --environment esp01 -t upload
exit $?