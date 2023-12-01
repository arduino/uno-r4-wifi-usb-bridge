# modify esp32 platform.txt from g++11 to gcc+17

if [ ! -d hardware ]; then
    mkdir -p hardware/esp32-patched
    cd hardware/esp32-patched
    git clone https://github.com/pennam/arduino-esp32 -b unor4wifi --depth=1 esp32
    cd esp32
    ./tools/get.py
    cd ../../../

    sed  s#PWD#$PWD#g arduino-cli.yaml.orig > arduino-cli.yaml

    cd hardware/esp32-patched/esp32/libraries
    git clone https://github.com/arduino-libraries/ArduinoBLE.git -b 1.3.6
    git clone https://github.com/arduino-libraries/Arduino_ESP32_OTA.git -b 0.2.0
    git clone https://github.com/pennam/BOSSA.git
    cd BOSSA
    git checkout 43a8feff9dfd07109d25cb2005279dd92e81d8fc
    cd ..
    git clone https://github.com/arduino-libraries/Arduino_DebugUtils.git
    cd ../../../../

    ln -s $(pwd)/hardware/esp32-patched/esp32/esptool hardware/esp32-patched/esp32/tools/esptool
    ln -s $(pwd)/hardware/esp32-patched/esp32/xtensa-esp32s3-elf hardware/esp32-patched/esp32/tools/xtensa-esp32s3-elf
fi

arduino-cli compile -e --config-file arduino-cli.yaml --fqbn=esp32-patched:esp32:arduino_unor4wifi_usb_bridge:JTAGAdapter=default,PSRAM=disabled,FlashMode=qio,FlashSize=4M,LoopCore=1,EventsCore=1,USBMode=default,CDCOnBoot=default,MSCOnBoot=default,DFUOnBoot=default,UploadMode=default,PartitionScheme=unor4wifi,CPUFreq=240,UploadSpeed=921600,DebugLevel=none,EraseFlash=none -v UNOR4USBBridge
