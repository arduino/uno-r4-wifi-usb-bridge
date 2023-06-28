# modify esp32 platform.txt from g++11 to gcc+17
rm -f S3.bin
rm -f x509_crt_bundle
echo "Build Sketch"
arduino-cli compile -e --clean --quiet --fqbn=esp32:esp32:esp32s3:JTAGAdapter=default,PSRAM=disabled,FlashMode=qio,FlashSize=4M,LoopCore=1,EventsCore=1,USBMode=default,CDCOnBoot=default,MSCOnBoot=default,DFUOnBoot=default,UploadMode=default,PartitionScheme=huge_app,CPUFreq=240,UploadSpeed=921600,DebugLevel=none,EraseFlash=none -v UNOR4USBBridge
echo "Build certificates bundle"
python certificates/gen_crt_bundle.py -i certificates/certificates.pem
echo "Combine binaries"
python combine.py
echo "Done"
