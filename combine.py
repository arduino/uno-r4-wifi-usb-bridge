#!/usr/bin/env python

import sys;

booloaderData = open("UNOR4USBBridge/build/esp32-patched.esp32.arduino_unor4wifi_usb_bridge/UNOR4USBBridge.ino.bootloader.bin", "rb").read()
partitionData = open("UNOR4USBBridge/build/esp32-patched.esp32.arduino_unor4wifi_usb_bridge/UNOR4USBBridge.ino.partitions.bin", "rb").read()
bootApp = open("boot/boot_app0.bin", "rb").read()
appData = open("UNOR4USBBridge/build/esp32-patched.esp32.arduino_unor4wifi_usb_bridge/UNOR4USBBridge.ino.bin", "rb").read()
spiffsData = open("spiffs/spiffs.bin", "rb").read()
certsData = open("UNOR4USBBridge/build/esp32-patched.esp32.arduino_unor4wifi_usb_bridge/x509_crt_bundle", "rb").read()

# Offset   Size     Name
# 0x000000 0x008000 bootloader
# 0x008000 0x001000 partitions
# 0x009000 0x002000 boot_app/otadata
# 0x00B000 0x045000 certs
# 0x050000 0x190000 app0
# 0x1E0000 0x190000 app1
# 0x370000 0x080000 spiffs
# 0x3F0000 0x010000 nvs

# calculate the output binary size included nvs
outputSize = 0x400000

# allocate and init to 0xff
outputData = bytearray(b'\xff') * outputSize

# copy data: bootloader, partitions, app
for i in range(0, len(booloaderData)):
	outputData[0x0000 + i] = booloaderData[i]

for i in range(0, len(partitionData)):
	outputData[0x8000 + i] = partitionData[i]

for i in range(0, len(bootApp)):
    outputData[0x9000 + i] = bootApp[i]

for i in range(0, len(certsData)):
    outputData[0xB000 + i] = certsData[i]

for i in range(0, len(appData)):
    outputData[0x50000 + i] = appData[i]

for i in range(0, len(spiffsData)):
    outputData[0x370000 + i] = spiffsData[i]

outputFilename = "UNOR4USBBridge/build/esp32-patched.esp32.arduino_unor4wifi_usb_bridge/S3-ALL.bin"

# write out
with open(outputFilename,"w+b") as f:
	f.seek(0)
	f.write(outputData)
	f.close

outputFilename = "UNOR4USBBridge/build/esp32-patched.esp32.arduino_unor4wifi_usb_bridge/S3-BOOT-APP.bin"

# write out
with open(outputFilename,"w+b") as f:
	f.seek(0)
	f.write(outputData[:0x1E0000])
	f.close

outputFilename = "UNOR4USBBridge/build/esp32-patched.esp32.arduino_unor4wifi_usb_bridge/S3-APP.bin"

# write out
with open(outputFilename,"w+b") as f:
	f.seek(0)
	f.write(outputData[0xE000:0x1E0000])
	f.close
