# UNO R4 WiFi updater script

This updater script can be used to automatically put the UNO R4 WiFi in `ESP download` mode and upload the latest firmware.

The script uses [unor4wifi-reboot](unor4wifi-reboot) to put the board in `ESP download` mode through an HID message; and [espflash](https://github.com/esp-rs/espflash/releases) to flash the firmware bundle.

# How to use the script

## On Windows
* Extract the zip file
* Disconnect all USB gadgets from your PC
* Connect your UNO R4 WiFi
* Double click on update.bat
* Follow instructions on the terminal window
* Disconnect and connect your UNO R4 WiFi from your PC

On windows `espflash `do not list the board as `Espressif USB JTAG/serial debug unit` but only as generic `COMX` port.
Check the `COMX` properties from the device manager to be sure what to select.

![image](https://github.com/pennam/UnoR4WiFiUpdate/assets/20436476/7c219bc1-d68e-4c61-8b43-ffffbba4955c)  ![image](https://github.com/pennam/UnoR4WiFiUpdate/assets/20436476/ff2dbb8f-2d6c-4d6c-aab0-5002bd1a11d9) 

If Windows  is overprotective click on `More info` and then `Run anyway`

![image](https://github.com/pennam/UnoR4WiFiUpdate/assets/20436476/dbd20f28-c3c5-4194-b509-f728ac6dc0df)

## On Linux
* Extract the zip file
* Disconnect all USB gadgets from your PC
* Connect your UNO R4 WiFi
* From the terminal emulator run `./update.sh`
* Follow instructions on the terminal window
* Disconnect and connect your UNO R4 WiFi from your PC

On some distro may be required to run the script with `sudo`.

## On MacOS
* Extract the zip file
* Disconnect all USB gadgets from your PC
* Connect your UNO R4 WiFi
* Double click on update.command
* Follow instructions on the terminal window
* Disconnect and connect your UNO R4 WiFi from your PC

If MacOS complains about the developer run this commands from the root folder of the extracted .zip file and then double click again on the script.

```
chmod a+x update.command
sudo xattr -d com.apple.quarantine bin/espflash
sudo xattr -d com.apple.quarantine bin/unor4wifi-reboot-macos
```

Running the script from the terminal instead of double click should avoid the commands above.

![image](https://github.com/pennam/UnoR4WiFiUpdate/assets/20436476/dd776469-ba93-4430-9b72-25a6ca75840f)

# Troubleshooting
* If the script report this error: `Cannot put the board in ESP mode. (via 'unor4wifi-reboot')`

## Option 1
Disconnect and connect again your UNO R4 WiFi from your PC and re-run the script.

## Option 2

Follow the "**Run espflash directly**" instructions provided in this Arduino Help Center article:

https://support.arduino.cc/hc/en-us/articles/16379769332892-Restore-the-USB-connectivity-firmware-on-UNO-R4-WiFi-with-espflash
