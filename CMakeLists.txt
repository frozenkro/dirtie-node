# cmake -B ./build -DPICO_BOARD=pico_w -DCMAKE_BUILD_TYPE=Debug -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
cmake_minimum_required(VERSION 3.12)
include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)

project(dirtie-node C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

pico_sdk_init()

add_executable(${PROJECT_NAME}
  main.c
  connect.c
  connect.h
  sensor.c
  sensor.h
  test.c
  test.h
  usb_cfg.c
  usb_cfg.h
  dirtie_globals.h
  usb_descriptors.c
  )
pico_add_extra_outputs(${PROJECT_NAME})
target_link_libraries(${PROJECT_NAME}
  pico_cyw43_arch_lwip_threadsafe_background
  pico_stdlib
  pico_lwip_mqtt
  hardware_i2c
  tinyusb_device
  tinyusb_board
  )
target_include_directories(
  ${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_LIST_DIR}  
  )

pico_enable_stdio_usb(${PROJECT_NAME} 1)
pico_enable_stdio_uart(${PROJECT_NAME} 1)

set(TEST_MODE 1)
target_compile_definitions(${PROJECT_NAME} PRIVATE "TEST_MODE=${TEST_MODE}")

# if (DEFINED ENV{WIFI_SSID} AND (NOT WIFI_SSID))
#   set(WIFI_SSID $ENV{WIFI_SSID})
#   message("Using WIFI_SSID from environment ('${WIFI_SSID}')")
# endif()
# if (WIFI_SSID)
#   target_compile_definitions(${PROJECT_NAME} PRIVATE "WIFI_SSID=\"${WIFI_SSID}\"")
# endif()
#
# if (DEFINED ENV{WIFI_PASSWORD} AND (NOT WIFI_PASSWORD))
#   set(WIFI_PASSWORD $ENV{WIFI_PASSWORD})
#   message("Using WIFI_PASSWORD from environment ('${WIFI_PASSWORD}')")
# endif()
# if (WIFI_PASSWORD)
#   target_compile_definitions(${PROJECT_NAME} PRIVATE "WIFI_PASSWORD=\"${WIFI_PASSWORD}\"")
# endif()
#
if (DEFINED ENV{MQTT_BROKER_IP} AND (NOT MQTT_BROKER_IP))
  set (MQTT_BROKER_IP $ENV{MQTT_BROKER_IP})
  message("Using MQTT_BROKER_IP from environment ('${MQTT_BROKER_IP}')")
endif()
if (MQTT_BROKER_IP)
  target_compile_definitions(${PROJECT_NAME} PRIVATE "MQTT_BROKER_IP=\"${MQTT_BROKER_IP}\"")
endif()
