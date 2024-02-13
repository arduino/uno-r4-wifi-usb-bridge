# modify esp32 platform.txt from g++11 to gcc+17
rm -rf UNOR4USBBridge/build
echo "Build Sketch"
./compile.sh
echo "Build certificates bundle"
python certificates/gen_crt_bundle.py -i certificates/certificates.pem
mv x509_crt_bundle UNOR4USBBridge/build/esp32-patched.esp32.arduino_unor4wifi_usb_bridge/x509_crt_bundle
echo "Combine binaries"
python combine.py
echo "Done"
