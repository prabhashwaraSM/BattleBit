# BattleBit - Arena Firmware

This directory contains the firmware for the **Arena Controller and Referee System** of the BattleBit project.

---

## Role & Responsibilities

The Arena Controller acts as the master referee and game coordinator for the BattleBit arena:
1. **Match Management**: Manages round countdown, active match timer, pause/resume, and match conclusion.
2. **Hit & Health Tracking**: Listens for telemetry packets from Robot 1 and Robot 2 to maintain live health points and hit counters.
3. **Hazard Automation**: Triggers arena hazards (e.g. pit traps, floor flippers, pop-up spikes, rotating obstacles) at scheduled match intervals.
4. **Display & Audio Output**: Drives live scoreboards (LCD/OLED/LED matrix) and buzzer/audio signals for round start, warnings, and winner announcements.
5. **Master Emergency Stop (E-Stop)**: Provides a physical master abort button to broadcast an emergency stop command to all active robots.

---

## Directory Structure

```text
arena/
├── include/              # Header files and configuration macros
│   ├── config.h          # Pin definitions, timings, and RF configuration
│   └── game_state.h      # Match state definitions and struct declarations
├── src/                  # Main source code
│   ├── main.cpp          # Entry point and main superloop / RTOS tasks
│   ├── display.cpp       # Scoreboard and UI management
│   ├── hazards.cpp       # Arena hazard actuation logic
│   ├── rf_receiver.cpp   # Wireless telemetry receiver
│   └── sound.cpp         # Buzzer and sound effects controller
├── platformio.ini        # PlatformIO configuration file
└── README.md             # This file
```

---

## Hardware Interfacing

- **Microcontroller**: ESP32 / Arduino Mega / RP2040
- **Display**: I2C / SPI OLED (e.g., SSD1306) or multi-digit 7-segment LED display
- **Wireless Module**: NRF24L01+ / ESP-NOW receiver
- **Actuators**: Relays / MOSFET modules for hazard solenoids or motors
- **Audio**: Piezo buzzer or DFPlayer Mini audio module
- **Inputs**: Physical arcade push buttons (Start, Pause, Reset, Master E-Stop)

---

## State Machine Overview

```text
[IDLE / READY] ---> [COUNTDOWN (3..2..1)] ---> [MATCH ACTIVE] ---> [MATCH END / WINNER]
                           ^                         |                      |
                           |                         v                      |
                           +------------------- [PAUSED] <------------------+
                                                     |
                                            [EMERGENCY STOP]
```

---

## Getting Started

1. Open this directory in PlatformIO:
   ```bash
   cd firmware/arena
   pio run
   ```
2. Upload firmware to the Arena Controller board:
   ```bash
   pio run --target upload
   ```
3. Open Serial Monitor for debug output:
   ```bash
   pio device monitor -b 115200
   ```
