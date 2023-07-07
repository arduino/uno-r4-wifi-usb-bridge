# Arduino UNO R4 WiFi USB Bridge firmware

This firmware uses [arduino-esp32](https://github.com/espressif/arduino-esp32/releases/tag/2.0.9)

## Building

```
./compile.sh
```
Running the compile script will:

1. Download the [arduino-esp32 v2.0.9 core](https://github.com/espressif/arduino-esp32/releases/tag/2.0.9) and build tools
2. Apply needed [patches](core_esp32.patch)
3. Compile the firmware using arduino-cli
4. Export binaries in the build directory inside the `UNOR4USBBridge` folder 

The `compile.sh` script will produce a bunch of binary files that can be flashed using [esptool](https://github.com/espressif/esptool/releases) from the build directory:

```
esptool --chip esp32s3 --port "/dev/ttyACM0" --baud 921600  --before default_reset --after hard_reset write_flash  -z --flash_mode dio --flash_freq 80m --flash_size 4MB 0x0 "UNOR4USBBridge.ino.bootloader.bin" 0x8000 "UNOR4USBBridge.ino.partitions.bin" 0xe000 "../../../boot/boot_app0.bin" 0x10000 "UNOR4USBBridge.ino.bin"
```

## Packaging

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

## Flashing

To flash the firmware the board needs to be in `ESP download` mode. This can be done [manually](unor4wifi-updater#option-2) or using the [unor4wifi-updater](unor4wifi-updater) script.

