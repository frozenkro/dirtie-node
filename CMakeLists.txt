# cmake -B ./build -DPICO_BOARD=pico_w -DCMAKE_BUILD_TYPE=Debug -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
cmake_minimum_required(VERSION 3.12)
include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)

project(dirtie_node C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

pico_sdk_init()

add_library(lib_${PROJECT_NAME}
  src/connect/connect.c
  src/sensor/sensor.c
  src/access_point/dhcpserver.c
  src/access_point/host.c
  src/flashmem/flashmem.c
)

add_executable(${PROJECT_NAME}
  src/main.c
)

add_subdirectory(test/Unity Unity_build)

add_executable(test_${PROJECT_NAME}
  test/test_main.c
  test/test_some_module.c
)

pico_set_linker_script(${PROJECT_NAME} ${CMAKE_SOURCE_DIR}/memmap_custom.ld)
pico_set_linker_script(test_${PROJECT_NAME} ${CMAKE_SOURCE_DIR}/memmap_custom.ld)

pico_add_extra_outputs(${PROJECT_NAME})
pico_add_extra_outputs(test_${PROJECT_NAME})


target_link_libraries(lib_${PROJECT_NAME}
  pico_cyw43_arch_lwip_threadsafe_background
  pico_stdlib
  pico_lwip_mqtt
  hardware_flash
  hardware_i2c
)

target_link_libraries(${PROJECT_NAME}
  lib_${PROJECT_NAME}
  pico_cyw43_arch_lwip_threadsafe_background
  pico_stdlib
  pico_lwip_mqtt
  hardware_flash
  hardware_i2c
)

target_link_libraries(test_${PROJECT_NAME} 
  lib_${PROJECT_NAME}
  unity
  pico_cyw43_arch_lwip_threadsafe_background
  pico_stdlib
  pico_lwip_mqtt
  hardware_flash
  hardware_i2c
)


target_include_directories(
  ${PROJECT_NAME} PUBLIC 
  ${CMAKE_CURRENT_LIST_DIR}/src
  ${CMAKE_CURRENT_LIST_DIR}/hw_test
)

target_include_directories(
  lib_${PROJECT_NAME} PUBLIC 
  ${CMAKE_CURRENT_LIST_DIR}/src
)

target_include_directories(
  test_${PROJECT_NAME} PRIVATE 
  ${CMAKE_CURRENT_LIST_DIR}/test
  ${CMAKE_CURRENT_LIST_DIR}/src
)

pico_enable_stdio_usb(${PROJECT_NAME} 1)
pico_enable_stdio_uart(${PROJECT_NAME} 1)

# set(TEST_MODE 1)
# target_compile_definitions(${PROJECT_NAME} PRIVATE "TEST_MODE=${TEST_MODE}")

if (DEFINED ENV{MQTT_BROKER_IP} AND (NOT MQTT_BROKER_IP))
  set (MQTT_BROKER_IP $ENV{MQTT_BROKER_IP})
  message("Using MQTT_BROKER_IP from environment ('${MQTT_BROKER_IP}')")
endif()
if (MQTT_BROKER_IP)
  target_compile_definitions(${PROJECT_NAME} PRIVATE "MQTT_BROKER_IP=\"${MQTT_BROKER_IP}\"")
  target_compile_definitions(test_${PROJECT_NAME} PRIVATE "MQTT_BROKER_IP=\"${MQTT_BROKER_IP}\"")
else()
  message(WARNING 
    "MQTT_BROKER_IP NOT SPECIFIED.\n"
    "This will prevent the device from publishing any data\n"
    "Specify with -DMQTT_BROKER_IP\n")
endif()
