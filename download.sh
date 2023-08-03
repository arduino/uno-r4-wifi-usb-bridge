#!/bin/bash

arduino-cli compile -e --config-file arduino-cli.yaml  --fqbn=esp32-patched:esp32:esp32s3:JTAGAdapter=default,PSRAM=disabled,FlashMode=qio,FlashSize=4M,LoopCore=1,EventsCore=1,USBMode=default,CDCOnBoot=default,MSCOnBoot=default,DFUOnBoot=default,UploadMode=default,PartitionScheme=huge_app,CPUFreq=240,UploadSpeed=921600,DebugLevel=none,EraseFlash=none UNOR4USBBridge -u -p  /dev/ttyACM0 -v

