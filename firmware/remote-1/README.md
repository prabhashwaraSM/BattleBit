# BattleBit - Remote Controller 1 (Player 1)

This directory contains the firmware for **Remote Controller 1**, dedicated to Player 1 for driving and commanding **Robot 1**.

---

## Features & Responsibilities

- **Drive Input Capture**: Reads analog voltages from dual-axis thumb joysticks (forward/reverse throttle, left/right steering or dual tank drive).
- **Weapon & Auxiliary Controls**: Reads momentary pushbuttons or toggle switches for activating primary weapons (spinners, flippers, lifters) and secondary systems.
- **Wireless Transmission**: Encodes control states into high-frequency packets and transmits them via RF (e.g. NRF24L01 / ESP-NOW) to Robot 1 with low latency (<20ms).
- **Telemetry & Feedback**:
  - Link quality indicator LED (confirms active handshake with Robot 1).
  - Haptic feedback / buzzer for hit warning or low battery alerts.
  - Onboard battery voltage monitoring via ADC.

---

## Directory Structure

```text
remote-1/
├── include/              # Header files and configuration
│   ├── config.h          # Pin mappings, calibration parameters, RF channel
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
- **Wireless**: NRF24L01+ 2.4GHz Transceiver (Channel/Pipe Address configured for Robot 1)
- **Indicators**: Dual-color status LED (Connection/Error), Haptic vibration disc motor
- **Power**: 1S LiPo (3.7V) with 3.3V/5V boost regulator or 9V battery with step-down

---

## Packet Definition (Reference)

```cpp
struct ControlPacket {
    uint8_t  packetId;       // Rolling sequence ID
    int8_t   throttle;       // -100 to 100
    int8_t   steering;       // -100 to 100
    uint8_t  weaponState;    // Bitmask for weapon triggers
    uint8_t  checksum;       // CRC or XOR checksum
};
```

---

## Build & Flash

```bash
cd firmware/remote-1
pio run --target upload
pio device monitor -b 115200
```
