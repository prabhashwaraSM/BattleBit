# BattleBit - Project Documentation

This directory contains technical documentation, communication protocol specifications, game rules, hardware pinout references, and operation guides for the **BattleBit** combat robotics platform.

---

## Documentation Structure

```text
docs/
├── architecture.md       # Full system architecture and component interactions
├── protocols/            # Wireless communication and packet frame specifications
│   ├── rf_packet_format.md
│   └── telemetry_spec.md
├── hardware/             # Pinouts, power budgets, and wiring references
│   ├── pinouts.md
│   └── power_budget.md
├── rules/                # Game rules, hit scoring criteria, and arena hazard mechanics
│   └── tournament_rules.md
├── guides/               # User guides, assembly manuals, and troubleshooting
│   ├── assembly_guide.md
│   └── calibration_guide.md
└── README.md             # This file
```

---

## Key Documents Overview

### 1. Protocols (`docs/protocols/`)
- **RF Packet Format**: Defines payload structure (Control inputs: X/Y joystick values, weapon trigger bitmasks, checksum/CRC) transmitted between Remotes and Robots.
- **Telemetry Specification**: Outlines telemetry frames sent by robots to the Arena controller (Health/Hit count, battery voltage level, ping/ACK status).

### 2. Hardware References (`docs/hardware/`)
- **Pinout Mapping**: Master table correlating MCU GPIOs for all nodes (Remote 1, Remote 2, Robot 1, Robot 2, Arena Hub).
- **Power Distribution**: Battery discharge ratings, operating voltages, regulator limits, and logic level translations (e.g. 5V motor logic vs 3.3V RF modules).

### 3. Game & Match Rules (`docs/rules/`)
- **Match Timing**: Round durations, overtime rules, sudden death conditions.
- **Scoring & Health**: Initial hit points, damage point multipliers, out-of-ring penalties, immobilized/knockout detection.

### 4. Guides (`docs/guides/`)
- **Assembly Guide**: Mechanical assembly, chassis mounting, and wiring steps.
- **Calibration Guide**: Joystick deadband tuning, motor PWM trim calibration, and sensor threshold adjustments.
