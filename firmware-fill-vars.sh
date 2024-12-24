if [ -z $NEMOK_SENSOR_TEST_HTTP ]; then
    export NEMOK_SENSOR_TEST_HTTP=0
fi

if [ -z $NEMOK_SENSOR_TEMP_DS18B20 ]; then
    export NEMOK_SENSOR_TEMP_DS18B20=0
fi

if [ -z $NEMOK_SENSOR_TEMP_MAX6675 ]; then
    export NEMOK_SENSOR_TEMP_MAX6675=0
fi

if [ -z $NEMOK_SIGNALK_SERVER_PORT ]; then
    export NEMOK_SIGNALK_SERVER_PORT=0
fi

if [ -z $NEMOK_SENSOR_DELAY ]; then
    export NEMOK_SENSOR_DELAY=0
fi

selected_count=0
for required in $NEMOK_SENSOR_TEST_HTTP $NEMOK_SENSOR_TEMP_DS18B20 $NEMOK_SENSOR_TEMP_MAX6675; do
    if [ $required -eq 1 ]; then
        selected_count=`expr $selected_count + 1`
    fi
done

if [ $selected_count -lt 1 ]; then
    echo "Error: No firmware type selected"
    exit 1
fi

if [ $selected_count -gt 1 ]; then
    echo "Error: More than one firmware type selected"
    exit 1
fi
