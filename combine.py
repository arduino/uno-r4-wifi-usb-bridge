#!/usr/bin/env python

import sys;

booloaderData = open("UNOR4USBBridge/build/esp32.esp32.esp32s3/UNOR4USBBridge.ino.bootloader.bin", "rb").read()
partitionData = open("UNOR4USBBridge/build/esp32.esp32.esp32s3/UNOR4USBBridge.ino.partitions.bin", "rb").read()
bootApp = open("boot/boot_app0.bin", "rb").read()
appData = open("UNOR4USBBridge/build/esp32.esp32.esp32s3/UNOR4USBBridge.ino.bin", "rb").read()
certsData = open("x509_crt_bundle", "rb").read()

# 0x000000 bootloader
# 0x008000 partitions
# 0x00E000 boot_app
# 0x010000 app
# 0x3C0000 certs

# calculate the output binary size, app offset 
outputSize = 0x3C0000 + len(certsData)
if (outputSize % 1024):
	outputSize += 1024 - (outputSize % 1024)

# allocate and init to 0xff
outputData = bytearray(b'\xff') * outputSize

# copy data: bootloader, partitions, app
for i in range(0, len(booloaderData)):
	outputData[0x0000 + i] = booloaderData[i]

for i in range(0, len(partitionData)):
	outputData[0x8000 + i] = partitionData[i]

for i in range(0, len(bootApp)):
        outputData[0xE0000 + i] = bootApp[i]

for i in range(0, len(appData)):
        outputData[0x10000 + i] = appData[i]

for i in range(0, len(certsData)):
	outputData[0x3C0000 + i] = certsData[i]


outputFilename = "S3.bin"
if (len(sys.argv) > 1):
	outputFilename = sys.argv[1]

# write out
with open(outputFilename,"w+b") as f:
	f.seek(0)
	f.write(outputData)
