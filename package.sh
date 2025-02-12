VERSION="0.5.2"
mkdir -p $VERSION
echo "Build in release mode"
bash export.sh > $VERSION/release.log 2>&1
echo "Create release zip"
zip -r $VERSION/unor4wifi-$VERSION-release.zip UNOR4USBBridge/build
echo "Create unpdater packages"
pushd unor4wifi-updater
bash package.sh $VERSION > ../$VERSION/updater.log 2>&1
popd
mv unor4wifi-updater/unor4wifi-update-windows.zip $VERSION/unor4wifi-update-windows.zip
mv unor4wifi-updater/unor4wifi-update-linux.zip $VERSION/unor4wifi-update-linux.zip
mv unor4wifi-updater/unor4wifi-update-macos.zip $VERSION/unor4wifi-update-macos.zip
echo "Build in debug mode"
sed -i 's/DebugLevel=none/DebugLevel=verbose/g' compile.sh
sed -i 's/DBG_ERROR/DBG_VERBOSE/g' UNOR4USBBridge/UNOR4USBBridge.ino
bash export.sh > $VERSION/debug.log 2>&1
echo "Create debug zip"
zip -r $VERSION/unor4wifi-$VERSION-debug.zip UNOR4USBBridge/build
echo "Done"
echo "..."
echo "Remember to restore compile script and .ino file"
