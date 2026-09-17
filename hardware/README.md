# BattleBit - Hardware Design & Assets

This directory contains the electronic hardware design files, printed circuit board (PCB) layouts, mechanical CAD models, 3D printing assets, and Bill of Materials (BOM) for the **BattleBit** project.

---

## Directory Structure

```text
hardware/
├── pcb/                  # PCB layout files, schematics, and Gerber archives
│   ├── arena_hub/        # Arena referee master board
│   ├── remote/           # Universal remote controller handheld PCB
│   └── robot/            # Robot mainboard (MCU + driver + hit sensing)
├── 3d_models/            # CAD design files and 3D printable models
│   ├── chassis/          # Robot chassis frames, motor mounts, armor plates (.stl, .step)
│   ├── weapons/          # Weapon attachments (spinner blades, flipper arms)
│   └── remote_cases/     # Ergonomic handheld controller shells
├── bom/                  # Bill of Materials spreadsheets and vendor part numbers
│   └── battlebit_bom.csv # Consolidated parts list with pricing and ratings
└── README.md             # This file
```

---

## Bill of Materials (BOM) Highlights

| Component Category | Recommended Part | Notes |
| :--- | :--- | :--- |
| **Microcontrollers** | ESP32 / Arduino Nano / RP2040 | 3.3V / 5V logic levels |
| **Wireless Transceivers** | NRF24L01+ with external antenna / ESP-NOW | 2.4 GHz ISM band |
| **Drive Motors** | N20 6V/12V Metal Gearmotors or TT Motors | 300–600 RPM recommended |
| **Motor Drivers** | TB6612FNG / DRV8833 / L298N | 1.2A continuous per channel |
| **Weapon Actuator** | Coreless DC motor, Micro servo (MG90S), or Brushless | Dependent on robot class |
| **Hit Sensors** | 20mm Piezo Vibration Sensors / Limit switches | Mounted behind armor plates |
| **Power Source** | 2S 7.4V LiPo (Robots), 1S LiPo or 9V (Remotes) | With balance lead and BMS |

---

## 3D Printing Guidelines

- **Filament Choice**:
  - **Chassis & Armor**: TPU (flexible, high impact absorption) or PETG / ABS (rigid impact resistance). *PLA is not recommended for combat armor as it tends to shatter under impact.*
  - **Remote Shells**: PLA or PLA+ (easy printing and ergonomic finish).
- **Print Settings**:
  - Infill: 40% - 60% Gyroid or Honeycomb for structural rigidity.
  - Perimeters / Shells: Minimum 4–5 walls for structural mounting points.

---

## Electrical & Battery Safety

> [!CAUTION]
> Lithium Polymer (LiPo) batteries can be hazardous if punctured, shorted, or over-discharged.
> - Always include a fuse or resettable PTC inline with the battery positive terminal.
> - Never discharge 1S LiPo below 3.0V, or 2S LiPo below 6.0V.
> - Ensure all high-current motor grounds are bonded to a common star ground to prevent MCU reset during stall current spikes.
