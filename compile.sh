# modify esp32 platform.txt from g++11 to gcc+17

if [ ! -d hardware ]; then
    mkdir -p hardware/esp32-patched
    cd hardware/esp32-patched
    git clone https://github.com/espressif/arduino-esp32 -b 2.0.9 --depth=1 esp32
    cd esp32
    patch -p1 < ../../../core_esp32.patch
    ./tools/get.py
    cd ../../../

    sed  s#PWD#$PWD#g arduino-cli.yaml.orig > arduino-cli.yaml

    cd hardware/esp32-patched/esp32/libraries
    git clone https://github.com/facchinm/ArduinoBLE.git
    cd ArduinoBLE/
    git checkout 4e4a7cef7718f6787ef89c24aa3ffd0fd2a9ced3
    cd ..
    git clone https://github.com/pennam/Arduino_ESP32_OTA.git
    cd Arduino_ESP32_OTA/
    git checkout e27f822406986354197a09516edaaeea3ed18e79
    cd ..
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
