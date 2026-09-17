# BattleBit - Testing & Quality Assurance

This directory contains test suites, hardware-in-the-loop (HIL) verification scripts, calibration tools, and pre-combat inspection checklists for the **BattleBit** system.

---

## Directory Structure

```text
testing/
├── unit/                 # Unit tests for protocol encoding/decoding and math logic
│   └── test_packet_crc.cpp
├── integration/          # Multi-node integration and latency measurement
│   ├── test_rf_ping_pong.cpp
│   └── test_telemetry_sync.cpp
├── calibration/          # Calibration and diagnostic scripts
│   ├── motor_trim_test.ino
│   ├── joystick_deadband_cal.ino
│   └── hit_sensor_threshold.ino
├── checklists/           # Pre-match and post-match inspection sheets
│   └── pre_combat_inspection.md
└── README.md             # This file
```

---

## Test Categories

### 1. RF Link & Latency Testing
- **Ping-Pong Latency**: Measures round-trip time between remote transmitter and robot receiver. Target latency: `< 20ms`.
- **Packet Drop Rate**: Measures percentage of dropped or corrupted packets at varying distances (1m to 15m) and behind obstacles.

### 2. Motor & Drive Calibration
- **Deadband Tuning**: Calibrates the center resting point of analog joysticks to prevent robot "creep" when idle.
- **Motor Speed Balancing (Trim)**: Compares left and right motor RPM under equal PWM duty cycles to ensure straight-line tracking.

### 3. Hit Sensor Calibration
- **Threshold Tuning**: Adjusts analog/piezo sensitivity thresholds to differentiate between self-generated motor vibrations and genuine enemy hits.
- **Bumper Switch Debounce**: Verifies debounce window (e.g. 50ms) to prevent false multi-counting from a single mechanical impact.

---

## Pre-Combat Safety Checklist

Before placing robots in the arena, verify:
- [ ] Emergency failsafe functions properly (motors stop when remote is powered off).
- [ ] LiPo voltage is within safe operating range (>3.7V per cell).
- [ ] Weapon disarm switch is functional and disarmed during handling.
- [ ] Radio channels are properly separated (Player 1 on Channel A, Player 2 on Channel B).
- [ ] Arena boundaries and emergency stop buttons are verified and responsive.
