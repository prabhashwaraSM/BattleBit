# BattleBit - Robot 1 Firmware

This directory contains the firmware and subsystem test programs for **Battle Robot 1**, controlled by Player 1 through a dedicated ESP32 remote controller.

---

## Features & Responsibilities

* **Mecanum Drive System**: Controls four TT DC motors through L298N motor drivers and provides omnidirectional movement using mecanum wheels.

* **Combat Actuator Control**: Controls the MG995 servo-based mechanism for lifting, pushing, or throwing, together with the electromagnet used to grip and hold the opponent.

* **Hit & Damage Detection**: Uses magnetic reed switches to detect valid hit events and process damage received by the robot.

* **Health Management**: Maintains the robot's health value, reduces health when valid hits are detected, and manages the robot's ALIVE/DEAD state.

* **Motion Monitoring**: Uses the MPU6050 sensor to monitor the robot's body angle and orientation during operation.

* **Wireless Communication**: Provides ESP32-to-ESP32 communication between Robot 1 and its dedicated remote controller for receiving control commands and transmitting health and status information.

* **Safety & Fail-Safe**: Monitors the communication link and disables robot movement and combat actuators when communication is lost. When health reaches zero, the robot enters the DEAD state and activates red indicators.

---

## File Structure

```text
robot-1/
├── Member1_DriveSystem.ino
├── Member2_CombatActuators.ino
├── Member3_SensorsHealth.ino
├── Member4_Robot_Comm_Test.ino
├── Robot1.ino
└── README.md
```

---

## File Descriptions

* **`Member1_DriveSystem.ino`**
  Implements and tests the four-wheel mecanum drive system using TT DC motors and L298N motor drivers.

* **`Member2_CombatActuators.ino`**
  Implements and tests the MG995 servo mechanism and electromagnet used for gripping and combat actions.

* **`Member3_SensorsHealth.ino`**
  Handles MPU6050 motion monitoring, magnetic reed switch hit detection, and robot health management.

* **`Member4_Robot_Comm_Test.ino`**
  Tests ESP32-to-ESP32 wireless communication between the robot and its remote controller.

* **`Robot1.ino`**
  Main Robot 1 firmware integrating the drive system, combat actuators, sensors, health system, wireless communication, and safety logic.

---

## Hardware Interfacing

* **Microcontroller**: ESP32
* **Drive System**: Four TT DC motors with four mecanum wheels
* **Motor Driver**: L298N
* **Combat Mechanism**: MG995 servo motor
* **Gripping System**: Electromagnet
* **Hit Detection**: Magnetic reed switches
* **Motion Sensor**: MPU6050
* **Power Supply**: 18650 battery pack
* **Wireless Communication**: ESP32-to-ESP32
* **Remote Display**: OLED display for health, battery/status, communication, joystick, and combat information

---

## Safety Features

1. **Communication Fail-Safe**:

   ```cpp
   if (millis() - lastPacketTime > FAILSAFE_TIMEOUT_MS) {
       stopDriveMotors();
       disableCombatMechanisms();
   }
   ```

2. **DEAD State**: When the robot's health reaches 0, drive motors and combat mechanisms are disabled, red indicators are activated, and the robot remains immobilized until reset.

3. **Communication Loss Protection**: If valid control packets are not received within the configured timeout period, the robot automatically stops its active outputs.

---

## Build & Upload

Open the required `.ino` file in the **Arduino IDE**, select the appropriate **ESP32 board and COM port**, and upload the firmware.

For the complete Robot 1 system, use:

```text
Robot1.ino
```

The `Member*.ino` files are individual subsystem implementations and testing programs used during development.

---

## BattleBit

BattleBit consists of two independently controlled combat robots, each with a dedicated ESP32 remote controller.

```text
Robot 1  <──── Wireless Communication ────>  Remote 1
Robot 2  <──── Wireless Communication ────>  Remote 2
```

Each robot independently handles movement, combat mechanisms, hit detection, health monitoring, and communication safety.
