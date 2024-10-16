#!/bin/bash

# Check if /dev/dri/ exists and is not empty
if [ -d /dev/dri ] && [ "$(ls -A /dev/dri)" ]; then
    # Iterate over each device in /dev/dri/
    for device in /dev/dri/*; do
        sudo chmod o+rw "$device"
        echo "Updated permissions for device $device"
    done
    acpp-info -l
    clinfo -l
fi

# Check if any arguments are provided
if [ $# -eq 0 ]; then
    set -- acpp-info
fi

# Execute the command
exec "$@"