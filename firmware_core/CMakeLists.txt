project(FIRMWARE C)

message("make firmware")

if (USING_CMODEL)
    message("make cmodel")
    include(cmodel.cmake)
elseif (PCIE_MODE)
    message("make pcie")
    include(pcie.cmake)
elseif (SOC_MODE)
    message("make soc")
    include(soc.cmake)
endif()

