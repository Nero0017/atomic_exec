export SG1684X_TOP=/workspace/TPU1686

export ATOMIC_EXEC_TOP="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"

echo "SG1684X_TOP : ${SG1684X_TOP}"
echo "LIBSOPHON_TOP : ${LIBSOPHON_TOP}"
echo "ATOMIC_EXEC_TOP : ${ATOMIC_EXEC_TOP}"


function build_bmlib_cmodel()
{
    pushd $LIBSOPHON_TOP
    mkdir -p build
    cd build
    cmake -G Ninja -DPLATFORM=cmodel  -DCMAKE_BUILD_TYPE=Debug ..
    ninja
    popd
}

function build_atomic_cmodel(){
    build_bmlib_cmodel

    pushd $ATOMIC_EXEC_TOP
    rm -rf build
    mkdir -p build

    pushd build
    export LD_LIBRARY_PATH=/workspace/atomic_exec/build:$LD_LIBRARY_PATH
    export LIBSOPHON_TOP=/workspace/libsophon
    export TPUKERNEL_FIRMWARE_PATH=${ATOMIC_EXEC_TOP}/build/firmware_core/libbm1684x_kernel_module.so
    cmake .. -DCMAKE_BUILD_TYPE=Debug -DUSING_CMODEL=ON -DPCIE_MODE=OFF -DSOC_MODE=OFF
    make -j
    popd
    popd
}

function build_atomic_pcie(){
    pushd $ATOMIC_EXEC_TOP
    rm -rf build
    mkdir -p build

    pushd build
    export LIBSOPHON_TOP=/opt/sophon/libsophon-current
    export CROSS_TOOLCHAINS=/workspace/bm_prebuilt_toolchains
    export LD_LIBRARY_PATH=/opt/sophon/libsophon-current/lib
    cmake .. -DCMAKE_BUILD_TYPE=Debug -DUSING_CMODEL=OFF -DPCIE_MODE=ON -DSOC_MODE=OFF

    make kernel_module
    make -j
    popd
    popd
}


function build_firmware_cmodel(){
    mkdir -p $ATOMIC_EXEC_TOP/firmware_core/build
    export CHIP_ARCH=bm1684x

    pushd $ATOMIC_EXEC_TOP/firmware_core/build
    cmake .. -DUSING_CMODEL=ON
    make -j
    popd
}

function build_firmware_pcie(){
    mkdir -p $ATOMIC_EXEC_TOP/firmware_core/build
    export

    pushd $ATOMIC_EXEC_TOP/firmware_core/build
    cmake .. -DUSING_CMODEL=OFF -DPCIE_MODE=ON
    make -j
    popd
}


function rebuild_firmware_top_bm1684x()
{
    local CONFIG_FILE=${1:-basic_defconfig}
    echo "building firmware top with config $1"
    local PREBUILT_DIR=$SG1684X_TOP/../bm_prebuilt_toolchains
    if [ ! -e "${PREBUILT_DIR}" ]; then
        PREBUILT_DIR=$SG1684X_TOP/../../bm_prebuilt_toolchains
    fi
    local FIRMWARE_TOP_OUTPUT=$SG1684X_TOP/build/firmware_top
    local PRECOMPILED_TOOLCHAIN_ROOT=$PREBUILT_DIR
    local PRECOMPILED_TOOLCHAIN_WIN=$PREBUILT_DIR/../bm_prebuilt_toolchains_win
    [ ! -d $PRECOMPILED_TOOLCHAIN_WIN ] && PRECOMPILED_TOOLCHAIN_WIN=$SG1684X_TOP/../bm_prebuilt_toolchains_win
    [ -d $PRECOMPILED_TOOLCHAIN_WIN ] && PRECOMPILED_TOOLCHAIN_ROOT=$PRECOMPILED_TOOLCHAIN_WIN
    local TOOLCHAIN_PATH=$PRECOMPILED_TOOLCHAIN_ROOT/gcc-arm-10.3-2021.07-x86_64-aarch64-none-elf
    local TOOLCHAIN_PREFIX=$TOOLCHAIN_PATH/bin/aarch64-none-elf-
    pushd $SG1684X_TOP/bm1684x/firmware_top/
    rm -rf $FIRMWARE_TOP_OUTPUT
    ARCH=arm64 CROSS_COMPILE=$TOOLCHAIN_PREFIX; make distclean && make $CONFIG_FILE O=$FIRMWARE_TOP_OUTPUT
    local err=$?
    if [ $err -ne 0 ]; then
        return $err
    fi
    popd

    pushd $FIRMWARE_TOP_OUTPUT
    make ARCH=arm64 CROSS_COMPILE=$TOOLCHAIN_PREFIX
    local err=$?
    popd
    return $err
}

function cp_new_addition_kernel_file()
{
    pushd $ATOMIC_EXEC_TOP
    cp build/firmware_core/libbm1684x_kernel_module.so /workspace/tpu-mlir/third_party/atomic_exec/libbm1684x_atomic_kernel.so   
}

