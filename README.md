Hardware:
- Raspberry Pi 5
- STM32 Nucleo-F446RE

UART wiring:
- STM32 PA9/D8 TX → Pi GPIO15/RX Pin 10
- STM32 PA10/D2 RX ← Pi GPIO14/TX Pin 8
- GND ↔ GND

Protocol:
- PING
- GET_STATUS
- LED_ON
- LED_OFF
