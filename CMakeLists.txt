cmake_minimum_required(VERSION 3.12)
include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)

project(plant-mudder-node C CXX ASM)
set(SMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

pico_sdk_init()

add_executable(${PROJECT_NAME}
  main.c
  )
pico_add_extra_outputs(${PROJECT_NAME})
target_link_libraries(${PROJECT_NAME}
  pico_cyw43_arch_lwip_threadsafe_background
  pico_stdlib
  )
target_include_directories(
  ${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_LIST_DIR}  
  )

pico_enable_stdio_usb(${PROJECT_NAME} 0)
pico_enable_stdio_uart(${PROJECT_NAME} 1)

if (DEFINED ENV{WIFI_SSID} AND (NOT WIFI_SSID))
  set(WIFI_SSID $ENV{WIFI_SSID})
  add_compile_definitions(WIFI_SSID="${WIFI_SSID}")
  message("Using WIFI_SSID from environment ('${WIFI_SSID}')")
endif()

if (DEFINED ENV{WIFI_PASSWORD} AND (NOT WIFI_PASSWORD))
  set(WIFI_PASSWORD $ENV{WIFI_PASSWORD})
  add_compile_definitions(WIFI_PASSWORD="${WIFI_PASSWORD}")
  message("Using WIFI_PASSWORD from environment ('${WIFI_PASSWORD}')")
endif()
