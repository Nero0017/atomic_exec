set(CMAKE_BUILD_TYPE "Debug")


aux_source_directory(src KERNEL_SRC_FILES)

include_directories($ENV{LIBSOPHON_TOP}/bmlib/include)
include_directories($ENV{LIBSOPHON_TOP}/bmlib/src)

# include_directories(${LIBSOPHON_INCLUDE_DIRS})
include_directories(include)
# include_directories(${CMAKE_BINARY_DIR})
# include_directories($ENV{TPUTRAIN_TOP}/include)
# link_directories($ENV{TPUTRAIN_TOP}/lib)

link_directories(${CMAKE_BINARY_DIR})

# set(KERNEL_HEADER "${CMAKE_BINARY_DIR}/firmware_core/kernel_module_data.h")
# add_custom_command(
# 	OUTPUT ${KERNEL_HEADER}
# 	COMMAND echo "const unsigned int kernel_module_data[] = {0}\;" > ${KERNEL_HEADER}
# )
# Add a custom target that depends on the custom command
# add_custom_target(kernel_module ALL DEPENDS ${KERNEL_HEADER})


# target_compile_definitions(firmware PRIVATE -DUSING_CMODEL)


# message("BMLIB = ${BMLIB_CMODEL_PATH}")

# set_target_properties(firmware PROPERTIES OUTPUT_NAME cmodel)

# Set the output file for the shared library
set(SHARED_LIBRARY_OUTPUT_FILE bm1684x_kernel_module)
file(GLOB_RECURSE DEVICE_SRCS
    ${PROJECT_SOURCE_DIR}/src/*.c
)
add_library(${SHARED_LIBRARY_OUTPUT_FILE} SHARED ${DEVICE_SRCS})

set(FULL_BM1684X -Wl,--whole-archive libbackend_1684x.so -Wl,--no-whole-archive)


target_link_directories(${SHARED_LIBRARY_OUTPUT_FILE} PRIVATE
	$ENV{ATOMIC_EXEC_TOP}/third_party/bm1684x
)
target_link_libraries(${SHARED_LIBRARY_OUTPUT_FILE} ${FULL_BM1684X})


get_property(dirs DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY INCLUDE_DIRECTORIES)
foreach(dir ${dirs})
  message(STATUS "include dir='${dir}'")
endforeach()

get_property(dirs DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY LINK_DIRECTORIES)
foreach(dir ${dirs})
  message(STATUS "link dir='${dir}'")
endforeach()


