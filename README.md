# Yocto STM32 UART Edge Gateway

Embedded Linux edge gateway running on Raspberry Pi 5 with a custom Yocto image.  
The gateway communicates with an STM32 Nucleo-F446RE over UART using a lightweight command-response protocol.

## Overview

This project demonstrates an embedded Linux gateway architecture with:

- Custom Yocto layer and BitBake recipe
- C++ edge-gateway application
- systemd service integration
- UART communication between Raspberry Pi 5 and STM32
- STM32 firmware using HAL UART
- Basic protocol parsing and telemetry handling

## Architecture

```text
STM32 Nucleo-F446RE
        ↕ UART
Raspberry Pi 5
        ↕
Yocto Linux
        ↕
C++ edge-gateway service
```

## Hardware

- Raspberry Pi 5
- STM32 Nucleo-F446RE
- UART connection via GPIO
- HDMI/keyboard for local Raspberry Pi access

## UART Wiring

| STM32 Nucleo-F446RE | Raspberry Pi 5 |
|---|---|
| PA9 / D8 / USART1_TX | GPIO15 / Pin 10 / RX |
| PA10 / D2 / USART1_RX | GPIO14 / Pin 8 / TX |
| GND | GND / Pin 6 |

UART settings:

```text
Baud rate: 115200
Data bits: 8
Parity: None
Stop bits: 1
Flow control: None
```

## Protocol

### Commands

```text
PING
GET_STATUS
LED_ON
LED_OFF
```

### Responses

```text
ACK:PING
ACK:LED_ON
ACK:LED_OFF
STATUS:TEMP=19;HUM=60
NACK:UNKNOWN_CMD
```

## Project Structure

```text
.
├── meta-anas-edge/
│   └── recipes-apps/
│       └── edge-gateway/
│           ├── edge-gateway_0.1.bb
│           └── files/
│               ├── CMakeLists.txt
│               ├── main.cpp
│               ├── edge-gateway.service
│               ├── manager/
│               │   ├── uartManager.cpp
│               │   ├── uartManager.hpp
│               │   ├── protocolParser.cpp
│               │   └── protocolParser.hpp
│               └── utils/
│                   ├── Result.cpp
│                   └── Result.hpp
├── docs/
├── README.md
└── .gitignore
```

## Yocto Setup

Clone required layers:

```bash
git clone -b scarthgap https://git.yoctoproject.org/poky
git clone -b scarthgap https://github.com/agherzan/meta-raspberrypi.git
```

Initialize the build environment:

```bash
source poky/oe-init-build-env build
```

Add layers:

```bash
bitbake-layers add-layer ../meta-raspberrypi
bitbake-layers add-layer ../meta-anas-edge
```

## Build

Build the gateway application:

```bash
bitbake edge-gateway
```

Build the full image:

```bash
bitbake core-image-base
```

Image output:

```text
build/tmp/deploy/images/raspberrypi5/
```

## Flash Image

```bash
sudo bmaptool copy core-image-base-raspberrypi5.rootfs-*.wic.bz2 /dev/sdX
sync
```

Replace `/dev/sdX` with the correct SD card device.

## Runtime

Check service status:

```bash
systemctl status edge-gateway
```

Follow logs:

```bash
journalctl -u edge-gateway -f
```

Example output:

```text
PI Sent: GET_STATUS
STM32 Reply: STATUS:TEMP=19;HUM=60

PI Sent: LED_ON
STM32 Reply: ACK:LED_ON

PI Sent: LED_OFF
STM32 Reply: ACK:LED_OFF
```

## Components

### UartManager

Handles UART device access:

- Opens `/dev/ttyAMA0`
- Configures UART with `termios`
- Sends line-based commands
- Reads line-based responses

### ProtocolParser

Parses protocol messages:

- ACK responses
- NACK responses
- STATUS telemetry messages

### systemd Service

The gateway is installed as a systemd service and starts automatically on boot.

## Current Features

- Custom Yocto image for Raspberry Pi 5
- C++ UART gateway application
- systemd integration
- STM32 UART command-response firmware
- Line-based UART protocol
- Basic telemetry parsing

## Roadmap

- Add FreeRTOS-based STM32 firmware
- Add MQTT telemetry publishing
- Add persistent telemetry storage
- Add Zigbee device integration
- Add Linux kernel module experiments
