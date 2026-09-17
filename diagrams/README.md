# BattleBit - System Diagrams

This directory contains system architecture diagrams, circuit schematics, wiring pinouts, communication sequence diagrams, and flowcharts for the **BattleBit** microcontroller combat robot project.

---

## Directory Purpose

Visual diagrams provide quick reference for hardware connections, firmware state machines, and system-level communication flows between:
- Remote Controllers (Player 1 & Player 2)
- Battle Robots (Robot 1 & Robot 2)
- Arena Controller & Referee Display

---

## Proposed Directory Structure

```text
diagrams/
├── architecture/         # High-level system architecture and block diagrams
│   └── system_overview.png
├── schematics/           # Wiring schematics and circuit diagrams
│   ├── arena_schematic.png
│   ├── remote1_schematic.png
│   ├── remote2_schematic.png
│   ├── robot1_schematic.png
│   └── robot2_schematic.png
├── sequences/            # Communication protocols and timing sequence diagrams
│   └── rf_protocol_sequence.png
├── state_machines/       # Firmware state machines and game loop flowcharts
│   ├── arena_state_machine.png
│   └── robot_state_machine.png
└── README.md             # This file
```

---

## Diagram Specifications & Formats

To ensure clarity and maintainability across the team:
- **Source Files**: Editable source formats should be included (e.g., `.drawio`, `.puml`, Mermaid markdown, or EDA project files).
- **Exported Formats**: High-resolution `.png` or vector `.svg` for documentation and quick viewing.
- **Color Coding Conventions**:
  - `Power (VCC / Battery)`: Red
  - `Ground (GND)`: Black
  - `Logic / Signals (SPI, I2C, UART)`: Blue / Green / Yellow
  - `PWM / Motor Signals`: Orange / Purple

---

## Key Diagrams to Include

1. **System Topology Diagram**: Wireless communication links and frequencies (Remote 1 &rarr; Robot 1, Remote 2 &rarr; Robot 2, Arena &harr; Robots / Remotes).
2. **Robot Power & Drive Schematic**: Battery protection, step-down voltage regulators (5V/3.3V), motor driver wiring (H-Bridge/MOSFETs), and MCU connections.
3. **Remote Control Layout**: Pinout mapping for joysticks, weapon buttons, status LEDs, and wireless transceivers.
4. **Arena Hub Interfacing**: Display connections (I2C/SPI), buzzer/alarm outputs, hazard triggers, and sensor inputs.
