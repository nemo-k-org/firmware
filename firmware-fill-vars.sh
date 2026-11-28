#!/bin/bash
# Sourced by firmware-build.sh and firmware-upload.sh.
# Defaults unset sensor selectors to 0 and ensures exactly one is selected.

SENSOR_VARS=(
    NEMOK_SENSOR_TEST_HTTP
    NEMOK_SENSOR_TEMP_DS18B20
    NEMOK_SENSOR_TEMP_MAX6675
    NEMOK_SENSOR_GNSS_NEO6MV2
    NEMOK_SENSOR_GNSS_NEO6MV2_UDP
    NEMOK_SENSOR_ADC_TANKLEVEL
)

selected_count=0
for var in "${SENSOR_VARS[@]}"; do
    export "${var}=${!var:-0}"
    if [[ "${!var}" == "1" ]]; then
        selected_count=$((selected_count + 1))
    fi
done

if [[ $selected_count -lt 1 ]]; then
    echo "Error: No firmware type selected" >&2
    exit 1
fi

if [[ $selected_count -gt 1 ]]; then
    echo "Error: More than one firmware type selected" >&2
    exit 1
fi
