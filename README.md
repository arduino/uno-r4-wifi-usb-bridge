# Arduino UNO R4 WiFi USB Bridge firmware

This firmware uses [arduino-esp32](https://github.com/espressif/arduino-esp32/)

## Update submodules

```
git submodule update --init --depth 1 --no-single-branch
```

## Get the toolchain

```
cd hardware/esp32-patched/esp32/tools
./get.py
```

## Build the firmware

```
./compile.sh
```
Running the compile script will:

1. Compile the firmware using `arduino-cli`
2. Export binaries in the build directory inside the `UNOR4USBBridge` folder 

The `compile.sh` script will produce a bunch of binary files that can be flashed using [esptool](https://github.com/espressif/esptool/releases) from the build directory:

```
esptool --chip esp32s3 --port "/dev/ttyACM0" --baud 921600  --before default_reset --after hard_reset write_flash  -z --flash_mode dio --flash_freq 80m --flash_size 4MB 0x0 "UNOR4USBBridge.ino.bootloader.bin" 0x8000 "UNOR4USBBridge.ino.partitions.bin" 0xe000 "../../../boot/boot_app0.bin" 0x10000 "UNOR4USBBridge.ino.bin"
```

## Package a single binary

```
./export.sh
```

The `export.sh` script will take care of generating a single binary blob including the TLS certificates bundle. Running the export script will:

1. Invoke the `compile.sh` script
2. Generate the certificate bundle using this [script](certificates/gen_crt_bundle.py) and this [certificates list](certificates/certificates.pem)
3. Combine everything in a single binary blob that can be flashed from address 0x0 using [espflash](https://github.com/esp-rs/espflash/releases)

```
espflash write-bin -b 115200 0x0 S3.bin
```

## Update your board

To flash the firmware the board needs to be in `ESP download` mode. This can be done [manually](unor4wifi-updater#option-2) or using the [unor4wifi-updater](unor4wifi-updater) script.

Alternatively you can also use the `download.sh` script to update the firmware using the `arduino-cli`. Also in this case the `download.sh` script
should be invoked after putting the board in `ESP download` mode.
