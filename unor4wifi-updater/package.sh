#!/bin/bash

function package_windows() {
    mkdir -p unor4wifi-update-windows
    mkdir -p unor4wifi-update-windows/bin
    mkdir -p unor4wifi-update-windows/firmware

    wget https://github.com/esp-rs/espflash/releases/download/v2.0.0/espflash-x86_64-pc-windows-msvc.zip -O /tmp/espflash-x86_64-pc-windows-msvc.zip
    unzip /tmp/espflash-x86_64-pc-windows-msvc.zip -d unor4wifi-update-windows/bin

    wget https://github.com/bcmi-labs/uno-r4-wifi-usb-bridge/releases/download/0.1.0/unor4wifi-reboot-windows.zip -O /tmp/unor4wifi-reboot-windows.zip
    unzip /tmp/unor4wifi-reboot-windows.zip -d unor4wifi-update-windows/bin

    cp update_windows.bat unor4wifi-update-windows/update.bat
    wget https://github.com/bcmi-labs/uno-r4-wifi-usb-bridge/releases/download/0.2.0/S3.bin -O unor4wifi-update-windows/firmware/UNOR4-WIFI-S3-0.2.0-rc1.bin

    zip -r unor4wifi-update-windows.zip unor4wifi-update-windows

}

function package_linux() {
    mkdir -p unor4wifi-update-linux
    mkdir -p unor4wifi-update-linux/bin
    mkdir -p unor4wifi-update-linux/firmware

    wget https://github.com/esp-rs/espflash/releases/download/v2.0.0/espflash-x86_64-unknown-linux-gnu.zip -O /tmp/espflash-x86_64-unknown-linux-gnu.zip
    unzip /tmp/espflash-x86_64-unknown-linux-gnu.zip -d unor4wifi-update-linux/bin

    wget https://github.com/bcmi-labs/uno-r4-wifi-usb-bridge/releases/download/0.1.0/unor4wifi-reboot-linux64.zip -O /tmp/unor4wifi-reboot-linux64.zip
    unzip /tmp/unor4wifi-reboot-linux64.zip -d unor4wifi-update-linux/bin

    cp update_linux.sh unor4wifi-update-linux/update.sh
    wget https://github.com/bcmi-labs/uno-r4-wifi-usb-bridge/releases/download/0.2.0/S3.bin -O unor4wifi-update-linux/firmware/UNOR4-WIFI-S3-0.2.0-rc1.bin

    zip -r unor4wifi-update-linux.zip unor4wifi-update-linux

}


function package_macos() {
    mkdir -p unor4wifi-update-macos
    mkdir -p unor4wifi-update-macos/bin
    mkdir -p unor4wifi-update-macos/firmware

    wget https://github.com/esp-rs/espflash/releases/download/v2.0.0/espflash-x86_64-apple-darwin.zip -O /tmp/espflash-x86_64-apple-darwin.zip
    unzip /tmp/espflash-x86_64-apple-darwin.zip -d unor4wifi-update-macos/bin

    wget https://github.com/bcmi-labs/uno-r4-wifi-usb-bridge/releases/download/0.1.0/unor4wifi-reboot-macos.zip -O /tmp/unor4wifi-reboot-macos.zip
    unzip /tmp/unor4wifi-reboot-macos.zip -d unor4wifi-update-macos/bin

    cp update_mac.command unor4wifi-update-macos/update.command
    wget https://github.com/bcmi-labs/uno-r4-wifi-usb-bridge/releases/download/0.2.0/S3.bin -O unor4wifi-update-macos/firmware/UNOR4-WIFI-S3-0.2.0-rc1.bin

    zip -r unor4wifi-update-macos.zip unor4wifi-update-macos

}


# Cleanup before starting
rm -rf unor4wifi-update-linux*
rm -rf unor4wifi-update-windows*
rm -rf unor4wifi-update-macos*

package_macos
package_linux
package_windows





