#!/bin/sh

# Iterate over each device in /dev/dri/
for device in /dev/dri/*; do
    sudo chmod o+rw "$device"
    echo "Update permissions for device $device"
done
