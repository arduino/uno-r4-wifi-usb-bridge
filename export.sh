# modify esp32 platform.txt from g++11 to gcc+17
rm -f S3.bin
rm -f x509_crt_bundle
echo "Build Sketch"
./compile.sh
echo "Build certificates bundle"
python certificates/gen_crt_bundle.py -i certificates/certificates.pem
echo "Combine binaries"
python combine.py
echo "Done"
