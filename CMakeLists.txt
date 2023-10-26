cmake_minimum_required(VERSION 3.5)
project(AtomicExec)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_program(CCACHE ccache)

if(CCACHE)
    set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ${CCACHE})
    set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK ${CCACHE})
endif()

if((NOT DEFINED ENV{SG1684X_TOP}) AND (NOT DEFINED ENV{NNTC_TOP}))
  message(FATAL_ERROR "execute envsetup.sh first")
endif()

add_executable(atomic ./main.cpp)


# libbm1684x_kernel_module.so
include_directories(${CMAKE_BINARY_DIR}/firmware_core)

# bm1684x.a
link_directories($ENV{ATOMIC_EXEC_TOP}/third_party/bm1684x)


add_definitions(-DDEBUG --std=c++11 -Wall)
set(CMAKE_BUILD_TYPE Debug)
include_directories($ENV{ATOMIC_EXEC_TOP}/include)

if(USING_CMODEL)
  include_directories($ENV{LIBSOPHON_TOP}/bmlib/include)
  link_directories($ENV{LIBSOPHON_TOP}/build/bmlib)
  add_definitions(-DUSING_CMODEL) 
  target_link_libraries(atomic $ENV{LIBSOPHON_TOP}/build/bmlib/libbmlib.so)
else()
  include_directories($ENV{LIBSOPHON_TOP}/include)
  link_directories($ENV{LIBSOPHON_TOP}/lib)
  target_link_libraries(atomic $ENV{LIBSOPHON_TOP}/lib/libbmlib.so)
endif()

if(USING_CMODEL)
  add_custom_command(
    TARGET atomic  # Replace <your_target_name> with your actual target name
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/third_party/bm1684x/libbackend_1684x.so ${CMAKE_BINARY_DIR}/libcmodel.so
  )
endif()

add_subdirectory(firmware_core)

get_property(dirs DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY INCLUDE_DIRECTORIES)
foreach(dir ${dirs})
  message(STATUS "include dir='${dir}'")
endforeach()

get_property(dirs DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY LINK_DIRECTORIES)
foreach(dir ${dirs})
  message(STATUS "link dir='${dir}'")
endforeach()



