project(FIRMWARE C)

message("make firmware")


include_directories(
	$ENV{ATOMIC_EXEC_TOP}/third_party/bm1684x/include
)
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -O0")

set(BM1684X_FIRMWARE_BASE /workspace/nntoolchain/TPU1686/bm1684x/firmware_base)
set(BM1684X_FIRMWARE_TOP /workspace/nntoolchain/TPU1686/bm1684x/firmware_top)

include_directories(${BM1684X_FIRMWARE_BASE}/include)
include_directories(${BM1684X_FIRMWARE_TOP}/include)

include_directories(/workspace/nntoolchain/TPU1686/bm1684x/spec/include)

include_directories(/workspace/nntoolchain/TPU1686/common/include/)
include_directories(/workspace/nntoolchain/TPU1686/common/include/firmware)

include_directories(/workspace/nntoolchain/TPU1686/a53lite_drv)

include_directories(/workspace/nntoolchain/TPU1686/kernel/include/)


# for including WRITE_CMD_XXXX macro
add_definitions( -DCONFIG_EU_SHIFT=4)
add_definitions( -DCONFIG_GLOBAL_DATA_INITIAL=0xdeadbeef)
add_definitions( -DCONFIG_GLOBAL_MEM_SIZE=0x100000000)
add_definitions( -DCONFIG_L2_SRAM_SIZE=0x1FB000)
add_definitions( -DCONFIG_LOCAL_DATA_INITIAL=0xdeadbeef)
add_definitions( -DCONFIG_LOCAL_MEM_ADDRWIDTH=18)
add_definitions( -DCONFIG_LOCAL_MEM_BANKS=16)
add_definitions( -DCONFIG_MAJOR_VERSION=1)
add_definitions( -DCONFIG_MAX_NODECHIP_NUM=1)
add_definitions( -DCONFIG_MINOR_VERSION=0)
add_definitions( -DCONFIG_NPU_SHIFT=6)
add_definitions( -DCONFIG_STATIC_MEM_SIZE=0x10000)


if (USING_CMODEL)

    message("make cmodel")
    include_directories(/workspace/nntoolchain/TPU1686/bm1684x/cmodel/include/)
    include(cmodel.cmake)
elseif (PCIE_MODE)
    message("make pcie")
    include(pcie.cmake)
elseif (SOC_MODE)
    message("make soc")
    include(soc.cmake)
endif()

