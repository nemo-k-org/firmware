#!/bin/bash

# shellcheck source=firmware-fill-vars.sh
. "$(dirname "$0")/firmware-fill-vars.sh"

pio run --environment esp01 -v -t upload
exit $?
