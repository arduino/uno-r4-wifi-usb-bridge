#!/bin/bash
#set -e

SCRIPTPATH="$( cd "$(dirname "$0")" ; pwd -P )"

cd "$SCRIPTPATH"

./bin/unor4wifi-reboot-linux64
if [ "$?" -ne 0 ]; then
    echo "Cannot put the board in ESP mode. (via 'unor4wifi-reboot')"
    exit 1
fi

echo "Start flashing firmware"
sleep 1
./bin/espflash write-bin -b 115200 0x0 firmware/UNOR4-WIFI-S3-0.2.0-rc1.bin