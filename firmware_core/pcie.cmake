# Set the C compiler, The path needs to be modified
set(CMAKE_C_COMPILER $ENV{CROSS_TOOLCHAINS}/gcc-arm-10.3-2021.07-x86_64-aarch64-none-linux-gnu/bin/aarch64-none-linux-gnu-gcc)

message("set source from ${PROJECT_SOURCE_DIR}")

# Set the source files for the shared library
file(GLOB_RECURSE DEVICE_SRCS
    ${PROJECT_SOURCE_DIR}/src/*.c
)
set(CMAKE_BUILD_TYPE "Debug")

# Set the include directories for the shared library
include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(${CMAKE_BINARY_DIR})

# Set the output file for the shared library
set(SHARED_LIBRARY_OUTPUT_FILE libbm1684x_kernel_module)

# Create the shared library
add_library(${SHARED_LIBRARY_OUTPUT_FILE} SHARED ${DEVICE_SRCS})


target_link_directories(${SHARED_LIBRARY_OUTPUT_FILE} PRIVATE
	$ENV{ATOMIC_EXEC_TOP}/third_party/bm1684x/lib
)

# Link the libraries for the shared library

target_link_libraries(${SHARED_LIBRARY_OUTPUT_FILE} -Wl,--whole-archive libfirmware_core.a -Wl,--no-whole-archive m)

# Set the output file properties for the shared library
set_target_properties(${SHARED_LIBRARY_OUTPUT_FILE} PROPERTIES PREFIX "" SUFFIX ".so" COMPILE_FLAGS "-O2 -fPIC" LINK_FLAGS "-shared")

# Set the path to the input file
set(INPUT_FILE "${CMAKE_BINARY_DIR}/firmware_core/libbm1684x_kernel_module.so")

set(HEAD "\# ==============================================================================\n\#\n\# Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.\n\#\n\# TPU-MLIR is licensed under the 2-Clause BSD License except for the\n\# third-party components.\n\#\n\# ==============================================================================")

# Set the path to the output file
set(OUTPUT_FILE "${CMAKE_BINARY_DIR}/firmware_core/kernel_module_data.py")
add_custom_command(
    OUTPUT ${OUTPUT_FILE}
    DEPENDS ${SHARED_LIBRARY_OUTPUT_FILE}
    COMMAND echo "${HEAD}" > ${OUTPUT_FILE}
    COMMAND echo "\# auto generated" > ${OUTPUT_FILE}
    COMMAND echo "kernel_module_data = [" > ${OUTPUT_FILE}
    COMMAND hexdump -v -e '1/4 \"0x%08x,\\n\"' ${INPUT_FILE} >> ${OUTPUT_FILE}
    COMMAND echo "]\;" >> ${OUTPUT_FILE}
)

# Add a custom target that depends on the custom command
# add_custom_target(dynamic_library ALL DEPENDS ${SHARED_LIBRARY_OUTPUT_FILE})
add_custom_target(kernel_module DEPENDS ${OUTPUT_FILE})
