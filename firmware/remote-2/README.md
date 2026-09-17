# BattleBit - Remote Controller 2 (Player 2)

This directory contains the firmware for **Remote Controller 2**, dedicated to Player 2 for driving and commanding **Robot 2**.

---

## Features & Responsibilities

- **Drive Input Capture**: Reads analog voltages from dual-axis thumb joysticks (forward/reverse throttle, left/right steering or dual tank drive).
- **Weapon & Auxiliary Controls**: Reads momentary pushbuttons or toggle switches for activating primary weapons and secondary systems.
- **Wireless Transmission**: Encodes control states into high-frequency packets and transmits them via RF (e.g. NRF24L01 / ESP-NOW) to Robot 2.
- **Independent RF Configuration**: Configured with unique RF address/channel pairing to prevent interference or cross-talk with Player 1.
- **Telemetry & Feedback**:
  - Link quality indicator LED (confirms active handshake with Robot 2).
  - Haptic feedback / buzzer for hit warning or low battery alerts.
  - Onboard battery voltage monitoring via ADC.

---

## Directory Structure

```text
remote-2/
├── include/              # Header files and configuration
│   ├── config.h          # Pin mappings, calibration parameters, RF channel (P2)
│   └── packet.h          # Control packet data structures
├── src/                  # Source files
│   ├── main.cpp          # Setup and main loop
│   ├── inputs.cpp        # ADC reading, deadband filtering, button debouncing
│   ├── radio.cpp         # RF transmission and connection management
│   └── feedback.cpp      # LED and haptic motor feedback
├── platformio.ini        # PlatformIO configuration file
└── README.md             # This file
```

---

## Hardware Configuration

- **Microcontroller**: Arduino Nano / ESP32 / ATmega328P / RP2040
- **Inputs**:
  - 2-Axis Analog Thumb Joystick (X: Drive/Steer, Y: Throttle)
  - Weapon Trigger Button(s) (Debounced digital inputs)
  - Pairing / Calibrate Button
- **Wireless**: NRF24L01+ 2.4GHz Transceiver (Channel/Pipe Address configured for Robot 2)
- **Indicators**: Dual-color status LED (Connection/Error), Haptic vibration disc motor
- **Power**: 1S LiPo (3.7V) with 3.3V/5V boost regulator or 9V battery with step-down

---

## Channel & Addressing Pairing

To ensure Player 1 and Player 2 operate simultaneously without collision:
- **Remote 1 / Robot 1**: Pipe `0xF0F0F0F0D2LL` / Channel `76` (default)
- **Remote 2 / Robot 2**: Pipe `0xF0F0F0F0E1LL` / Channel `76` (unique pipe address) or Channel `82`

---

## Build & Flash

```bash
cd firmware/remote-2
pio run --target upload
pio device monitor -b 115200
```
