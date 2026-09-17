# BattleBit - Firmware

This directory contains the embedded software source code for all microcontrollers in the **BattleBit** combat robotics system.

---

## Subsystem Overview

| Directory | Target Hardware | Primary Role |
| :--- | :--- | :--- |
| [`arena/`](file:///Volumes/Academic/Project/Microcontroller%20base%20project/BattleBit/Code/firmware/arena/README.md) | Arena Controller MCU | Match referee, round timer, hazard activation, and scoreboard display |
| [`remote-1/`](file:///Volumes/Academic/Project/Microcontroller%20base%20project/BattleBit/Code/firmware/remote-1/README.md) | Remote Controller 1 (P1) | Player 1 transmitter: joysticks, weapon switches, telemetry indicator |
| [`remote-2/`](file:///Volumes/Academic/Project/Microcontroller%20base%20project/BattleBit/Code/firmware/remote-2/README.md) | Remote Controller 2 (P2) | Player 2 transmitter: joysticks, weapon switches, telemetry indicator |
| [`robot-1/`](file:///Volumes/Academic/Project/Microcontroller%20base%20project/BattleBit/Code/firmware/robot-1/README.md) | Battle Robot 1 MCU | Drive motor control, weapon actuation, hit detection, safety watchdog |
| [`robot-2/`](file:///Volumes/Academic/Project/Microcontroller%20base%20project/BattleBit/Code/firmware/robot-2/README.md) | Battle Robot 2 MCU | Drive motor control, weapon actuation, hit detection, safety watchdog |

---

## Architecture & Communication Flow

```mermaid
flowchart TD
    subgraph Controllers
        R1[Remote 1 (P1)]
        R2[Remote 2 (P2)]
    end

    subgraph Robots
        B1[Robot 1 (Bot 1)]
        B2[Robot 2 (Bot 2)]
    end

    subgraph Central Hub
        AR[Arena Controller]
        DISP[Scoreboard & Timer Display]
        HAZ[Arena Hazards & Sound Effects]
    end

    R1 -- "Control Packets (RF Ch A)" --> B1
    R2 -- "Control Packets (RF Ch B)" --> B2
    B1 -. "Hit & Status Telemetry" .-> AR
    B2 -. "Hit & Status Telemetry" .-> AR
    AR --> DISP
    AR --> HAZ
```

---

## Development Environment & Toolchain

The firmware modules are designed to be built using **PlatformIO** (recommended) or the **Arduino IDE / ESP-IDF**.

### Prerequisites
- [PlatformIO Core or PlatformIO IDE extension for VS Code](https://platformio.org/)
- USB to Serial drivers for target microcontrollers (e.g. CH340, CP2102, FTDI)

### Common Guidelines
- **Failsafe Watchdog**: Every robot firmware module MUST implement a communication watchdog timer. If control packets are lost for >250ms, drive and weapon motors must automatically shut down (`FAILSAFE_STOP`).
- **Channel Pairing**: Dedicated RF channels or pipe addresses must be configured to prevent cross-talk between Player 1 and Player 2.
- **Consistent Packet Structures**: All modules should share standard header files defining data structures for control payloads and telemetry frames.
