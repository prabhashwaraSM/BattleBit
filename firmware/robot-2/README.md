# BattleBit - Robot 2 Firmware

This directory contains the firmware for **Battle Robot 2**, driven by Player 2.

---

## Features & Responsibilities

- **Motor Drive Control**: Translates incoming throttle and steering commands into differential drive PWM signals for left and right DC drive motors (H-Bridge / Motor Driver).
- **Weapon System**: Controls primary weapon actuator (e.g. spinning blade brushless ESC, servo-driven flipper, or lifting arm motor).
- **Hit & Impact Detection**: Reads onboard hit detection sensors (e.g. piezo vibration sensors, mechanical bumper switches, or IR receivers) to record enemy strikes.
- **Health Management & Telemetry**: Decrements health points upon registered hits, illuminates onboard status/health LEDs, and sends status telemetry back to the Arena Controller.
- **Paired RF Reception**: Listens exclusively to Remote 2 commands on assigned pipe address to eliminate interference with Robot 1.
- **Safety Watchdog**: Continuously monitors RF link heartbeats; halts all drive and weapon outputs immediately if no valid packet is received within 250ms.

---

## Directory Structure

```text
robot-2/
├── include/              # Header files and hardware definitions
│   ├── config.h          # Pin mappings, PWM channels, RF address (Robot 2)
│   └── motors.h          # Motor driver interface prototypes
├── src/                  # Implementation files
│   ├── main.cpp          # Setup, loop, and safety watchdog
│   ├── drive.cpp         # Differential drive mixing and PWM generation
│   ├── weapon.cpp        # Weapon motor and actuator control
│   ├── hit_detection.cpp # Sensor debouncing and hit processing
│   └── radio.cpp         # RF receiver and telemetry transmission
├── platformio.ini        # PlatformIO configuration file
└── README.md             # This file
```

---

## Hardware Interfacing

- **Microcontroller**: Arduino Uno/Nano / ESP32 / RP2040
- **Motor Driver**: Dual H-Bridge (e.g., L298N, TB6612FNG, or DRV8833)
- **Weapon Controller**: Brushless ESC (PWM signal) or Relay/MOSFET for high-current DC motor
- **Hit Sensors**: Piezoelectric disc sensors / tactile limit switches mounted on armor panels
- **Status Display**: WS2812B NeoPixel RGB LED strip (health bar)
- **Wireless Receiver**: NRF24L01+ or ESP-NOW receiver configured for Remote 2 and Arena

---

## Safety Features

1. **Watchdog Failsafe**:
   ```cpp
   if (millis() - lastPacketTime > FAILSAFE_TIMEOUT_MS) {
       stopAllMotors();
       disarmWeapon();
   }
   ```
2. **Knockout Mode**: If health reaches 0, motors are disabled, weapon is disarmed, and robot enters immobilized state until arena reset.

---

## Build & Flash

```bash
cd firmware/robot-2
pio run --target upload
pio device monitor -b 115200
```
