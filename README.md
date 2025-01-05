dirtie-node
---

A Raspberry Pi Pico W based soil monitoring system that publishes moisture readings to an MQTT broker.

## Overview

Dirtie is a microcontroller application designed to monitor plant soil conditions using a Raspberry Pi Pico W. It reads soil moisture data from connected sensors and publishes the readings to an MQTT broker, enabling integration with other monitoring and automation systems.

## Ecosystem

[dirtie-srv](https://github.com/frozenkro/dirtie-srv)
- Contains broker and web server used for device provisioning and data collection

[dirtie-client](https://github.com/frozenkro/dirtie-client)
- Android application used for device setup and viewing data from the device

## Hardware Requirements

- Raspberry Pi Pico W
- [Adafruit soil moisture sensor](https://www.adafruit.com/product/4026)

## Software Dependencies

- [Pico SDK](https://github.com/raspberrypi/pico-sdk) 
  - Relatively involved setup process for this
- arm-none-eabi toolchain
- CMake

## Building

- `cmake -B build -DPICO_BOARD=pico_w`
- `cmake --install build`

Additional cmake Flags:
- -DMQTT_BROKER_IP 
  - Specifies the location of your mqtt broker
- -DCMAKE_EXPORT_COMPILE_COMMANDS 
  - Outputs compile_commands.json
- -DCMAKE_BUILD_TYPE=Debug
  - Includes debug symbols in output so application can be debugged with GDP/openocd/picoprobe

## neovim lsp help

> Because this was a pain point for me :)

- Have clangd installed and set up in neovim
- Run the initial cmake command with the -DCMAKE_EXPORT_COMPILE_COMMANDS flag
- Symlink the output compile commands to the root of the repo 
  - `ln -s build/compile_commands.json ./compile_commands.json`
- Create the following .clangd file (.gitignored already) in the root of the repo

```
If:
  PathMatch: .*\.c$|.*\.h$
CompileFlags:
  Add: [
    "-isystem/usr/arm-none-eabi/include/"
  ]
```
