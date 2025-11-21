cmake_minimum_required(VERSION 3.22)

#
# This file is generated only once,
# and is not re-generated if converter is called multiple times.
#
# User is free to modify the file as much as necessary
#

# Setup compiler settings
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)


# Define the build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()

# Set the project name
set(CMAKE_PROJECT_NAME f3-traffic-light)

# Enable compile command to ease indexing with e.g. clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

# Core project settings
project(${CMAKE_PROJECT_NAME})
message("Build type: " ${CMAKE_BUILD_TYPE})

# Enable CMake support for ASM and C languages
enable_language(C ASM)

# Create an executable object type
add_executable(${CMAKE_PROJECT_NAME})

# Add STM32CubeMX generated sources
add_subdirectory(cmake/stm32cubemx)

# Link directories setup
target_link_directories(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined library search paths
)

# Add sources to executable
target_sources(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user sources here
)

# Add include paths
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined include paths
    ${CMAKE_CURRENT_SOURCE_DIR}/VendorUsb/Device/Inc
    ${CMAKE_CURRENT_SOURCE_DIR}/VendorUsb/Core/Inc
    ${CMAKE_CURRENT_SOURCE_DIR}/VendorUsb/Class/Inc
)

# Add project symbols (macros)
target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined symbols
)

# Remove wrong libob.a library dependency when using cpp files
list(REMOVE_ITEM CMAKE_C_IMPLICIT_LINK_LIBRARIES ob)

add_library(USB_Device_Library OBJECT
        ${CMAKE_CURRENT_SOURCE_DIR}/VendorUsb/Device/Src/usbd_conf.c
        ${CMAKE_CURRENT_SOURCE_DIR}/VendorUsb/Device/Src/usb_device.c
        ${CMAKE_CURRENT_SOURCE_DIR}/VendorUsb/Device/Src/usbd_desc.c
        ${CMAKE_CURRENT_SOURCE_DIR}/VendorUsb/Device/Src/usbd_winusb_if.c
        ${CMAKE_CURRENT_SOURCE_DIR}/VendorUsb/Core/Src/usbd_core.c
        ${CMAKE_CURRENT_SOURCE_DIR}/VendorUsb/Core/Src/usbd_ctlreq.c
        ${CMAKE_CURRENT_SOURCE_DIR}/VendorUsb/Core/Src/usbd_ioreq.c
        ${CMAKE_CURRENT_SOURCE_DIR}/VendorUsb/Class/Src/usbd_winusb.c
)
target_include_directories(USB_Device_Library PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/VendorUsb/Device/Inc
        ${CMAKE_CURRENT_SOURCE_DIR}/VendorUsb/Core/Inc
        ${CMAKE_CURRENT_SOURCE_DIR}/VendorUsb/Class/Inc
)
target_link_libraries(USB_Device_Library PUBLIC stm32cubemx)

# Add linked libraries
target_link_libraries(${CMAKE_PROJECT_NAME}
    stm32cubemx
    USB_Device_Library
    # Add user defined libraries
)
