# RTOS STM32 Project

## Overview

This repository contains an STM32 embedded application for the STM32F103 series, built with FreeRTOS and STM32CubeMX-generated HAL code. The firmware implements a simple vehicle control simulation with:

- FreeRTOS task-based architecture
- ADC input, brake input, and dashboard remote commands
- Servo output control for actuator simulation
- LCD display updates via I2C
- USART command receive and parsing
- Vehicle/engine state modeling via generated `EngTrModel` code

## Project Structure

- `CMakeLists.txt` - root CMake project definition
- `CMakePresets.json` - build presets for Debug/Release using Ninja and GNU Arm toolchain
- `cmake/` - custom CMake toolchain and STM32CubeMX support files
- `Core/Inc/` - user and generated header files
- `Core/Src/` - main firmware source files and middleware integrations
- `Drivers/` - STM32 HAL and CMSIS drivers
- `Middlewares/Third_Party/FreeRTOS/Source/` - FreeRTOS kernel source
- `build/` - generated build artifacts and intermediate files
- `rtos.ioc` - STM32CubeMX project configuration file
- `startup_stm32f103xb.s` - startup assembly for the STM32F103 device
- `STM32F103XX_FLASH.ld` - linker script for flash memory layout

## Key Firmware Components

- `Core/Src/main.c` - application entry point and RTOS tasks
- `Core/Src/freertos.c` - FreeRTOS initialization wrapper
- `Core/Src/user_uart.c` / `Core/Inc/user_uart.h` - USART1 setup and UART helpers
- `Core/Src/user_i2c.c` / `Core/Inc/user_i2c.h` - I2C peripheral setup for LCD
- `Core/Src/lcd.c` / `Core/Inc/lcd.h` - LCD display handling
- `Core/Src/EngTrModel.c` / `Core/Inc/EngTrModel.h` - generated vehicle engine model code

## Build Requirements

- CMake 3.22 or newer
- Ninja build system
- GNU Arm Embedded Toolchain (arm-none-eabi)
- STM32CubeMX or STM32CubeIDE compatibility for generated HAL configuration

## Build Instructions

From the repository root:

```powershell
cmake --preset Debug
cmake --build --preset Debug
```

For a release build:

```powershell
cmake --preset Release
cmake --build --preset Release
```

The generated executable and supporting build artifacts will be placed under `build/Debug/` or `build/Release/`.

## Flashing / Deployment

Use your preferred STM32 programmer tool to flash the generated binary or ELF file to the target MCU. Common tools include:

- STM32CubeProgrammer
- ST-Link Utility
- `st-flash`
- PlatformIO / OpenOCD

Example (toolchain-specific):

```powershell
# Replace with your actual tool and target file
st-flash write build/Debug/rtos.bin 0x08000000
```

## Usage

The application starts the FreeRTOS scheduler after hardware initialization and runs three main tasks:

- `Task_control` — reads inputs, applies manual/remote control logic, updates the engine model, and commands servo outputs
- `Task_output` — refreshes LCD output and writes sensor state to peripherals
- `Task_rx_parse` — receives UART/ESP commands and updates remote dashboard state

The system supports a manual throttle/brake input mode and a remote control mode via UART commands.

## Dashboard and Connectivity

This project also includes a Raspberry Pi dashboard and ESP8266 bridge to display STM32 telemetry and send remote control commands.

- `STM_ESP8266.ino` runs on an ESP8266 module. It receives telemetry from the STM32 over a software serial link at `19200` baud, parses lines like `TH=... ENG=... VEH=... G=... BR=...`, and publishes JSON telemetry to MQTT topic `motor/data`.
- The same ESP8266 subscribes to MQTT topics `motor/control/accel` and `motor/control/brake`; when commands arrive from the Raspberry Pi dashboard, it forwards them to the STM32 as serial commands prefixed with `A:` or `B:`.
- `dashboard.py` is a Python MQTT-to-InfluxDB bridge intended for the Raspberry Pi. It listens to telemetry (`motor/data`) and driver-camera topics, then stores the data in InfluxDB so Grafana or another dashboard can display live engine and vehicle metrics.
- `dashboard2.py` exposes a small Flask web API that receives dashboard control input and publishes it to MQTT. This is useful for connecting Grafana or another web frontend to the live control topics without talking directly to MQTT.
- `camara.py` runs face and expression detection on the Raspberry Pi camera using Picamera2 and OpenCV. It can publish driver state or alerts over MQTT, adding a driver-monitoring layer to the dashboard.

Together, these components form an integrated telemetry stack:

1. STM32 firmware generates vehicle state and sends it to the ESP8266 over UART.
2. ESP8266 publishes STM32 telemetry to the MQTT broker.
3. Raspberry Pi services consume MQTT telemetry, store it in InfluxDB, and provide web API endpoints for dashboard controls.
4. The dashboard can display vehicle speed, RPM, gear, brake, throttle, and driver state, while remote commands flow back to the STM32 through MQTT and ESP8266.

## Notes

- `cmake/stm32cubemx/` contains STM32CubeMX-generated project support and should be kept aligned with the CubeMX configuration.
- The application is configured for an STM32F103 MCU. Verify the device settings in `rtos.ioc` and the linker script before porting to another target.
- The `build/` directory is generated and can be removed safely before rebuilding.

## License

This repository does not include a specific license file. Add a license if you intend to publish or share the project.
