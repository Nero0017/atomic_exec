```
# libfirmware_core.a

pushd TPU1686
source scripts/envsetup.sh
rebuild_firmware
cp ./build/firmware_core/libfirmware_core.a ./
```