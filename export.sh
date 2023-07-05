# modify esp32 platform.txt from g++11 to gcc+17
rm -f UNOR4USBBridge/build/esp32-patched.esp32.esp32s3/S3.bin
rm -f UNOR4USBBridge/build/esp32-patched.esp32.esp32s3/x509_crt_bundle
echo "Build Sketch"
./compile.sh
echo "Build certificates bundle"
python certificates/gen_crt_bundle.py -i certificates/certificates.pem
mv x509_crt_bundle UNOR4USBBridge/build/esp32-patched.esp32.esp32s3/x509_crt_bundle
echo "Combine binaries"
python combine.py
echo "Done"
