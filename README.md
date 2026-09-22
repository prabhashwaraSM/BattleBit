# BattleBit — Dual-Robot Manual Combat System

BattleBit is a **dual-robot manual combat system** built as a microcontroller-based application project.

The system consists of **two independently controlled combat robots**, each piloted through its own dedicated **ESP32 remote controller**. Every robot combines omnidirectional mecanum drive, combat actuators, sensor-based hit detection, real-time health monitoring, wireless communication, and remote user control into a single coordinated platform.

---

## Features

* **Mecanum Drive System** — Four-wheel omnidirectional movement.
* **Combat Mechanism** — Electromagnet and MG995 servo working together to grip, hold, lift, push, and throw.
* **Hit Detection** — Magnetic reed switches register valid combat events.
* **Health System** — Robot health decreases whenever a valid hit is detected.
* **Angle Monitoring** — MPU6050 tracks the robot's body orientation.
* **Wireless Communication** — ESP32-based link handles control and status data.
* **Remote Control** — A dedicated ESP32 controller for each robot.
* **OLED Interface** — Displays health, status, communication, and control information.
* **Fail-Safe System** — Automatically halts robot operation if communication is lost.
* **DEAD State** — Disables robot operation and activates red indicators once health hits zero.

---

## System Architecture

```text
                         BATTLEBIT
                            │
             ┌──────────────┴──────────────┐
             │                             │
             ▼                             ▼
    ┌─────────────────┐           ┌─────────────────┐
    │ RemoteController│           │ RemoteController│
    │       1         │           │       2         │
    │     ESP32       │           │     ESP32       │
    │ Joystick + OLED │           │ Joystick + OLED │
    └────────┬────────┘           └────────┬────────┘
             │                             │
        Wireless Link                 Wireless Link
             │                             │
             ▼                             ▼
    ┌─────────────────┐           ┌─────────────────┐
    │     Robot 1     │           │     Robot 2     │
    │      ESP32      │           │      ESP32      │
    ├─────────────────┤           ├─────────────────┤
    │ Mecanum Drive   │           │ Mecanum Drive   │
    │ Combat System   │           │ Combat System   │
    │ Hit Detection   │           │ Hit Detection   │
    │ Health System   │           │ Health System   │
    │ MPU6050         │           │ MPU6050         │
    └─────────────────┘           └─────────────────┘
```

Each robot operates independently while staying in constant communication with its own remote controller.

---

## Robot Hardware

Each combat robot is built around an ESP32 and includes the following components:

| Component              | Purpose                     |
| ----------------------- | --------------------------- |
| ESP32                  | Main microcontroller        |
| TT DC Motors           | Wheel drive                 |
| L298N Motor Drivers    | Motor control                |
| Mecanum Wheels         | Omnidirectional movement    |
| MG995 Servo            | Combat mechanism control    |
| Electromagnet          | Gripping / holding opponent |
| MPU6050                | Body angle monitoring       |
| Magnetic Reed Switches | Hit detection                |
| LED Indicators         | Robot state indication      |
| 18650 Battery Pack     | Power supply                |

---

## Drive System

Each robot moves on **four TT DC motors** controlled through **L298N motor drivers**.

The mecanum wheel configuration enables full omnidirectional movement, including:

* Forward and reverse movement
* Left and right movement
* Strafing
* Rotation
* Combined (diagonal) movement

The drive system takes movement commands from the ESP32 controller and translates them into individual motor control signals.

---

## Combat System

The combat mechanism pairs an **electromagnet** with an **MG995 servo**.

### Electromagnet

Used to grip and hold the opponent during combat.

### MG995 Servo

Drives the mechanical combat mechanism for lifting, pushing, and throwing actions.

Both actuators are controlled directly by the robot's ESP32.

---

## Hit Detection

BattleBit uses **magnetic reed switches** to register valid combat events.

```text
Combat Event
     ↓
Reed Switch Detection
     ↓
Validate Hit
     ↓
Reduce Health
     ↓
Update Robot Status
     ↓
Send Status to Controller
```

This pipeline keeps each robot's health state accurate and up to date during a match.

---

## Health System

Each robot tracks its own independent health value.

```text
Initial Health
      ↓
Valid Hit Detected
      ↓
Health Reduction
      ↓
Check Health
   ↙          ↘
Health > 0   Health = 0
    ↓             ↓
Continue       DEAD State
Combat         + Red Indicator
```

Once health reaches zero, the robot enters the **DEAD state** and further combat operation is disabled.

---

## MPU6050 Monitoring

The **MPU6050** tracks the robot's body orientation and angle, providing motion and orientation data that can be used to monitor the robot throughout operation.

---

## Wireless Communication

An ESP32-based wireless link connects each robot to its dedicated remote controller, carrying:

* Movement commands
* Combat commands
* Health information
* Robot status
* Controller status
* Control feedback

This same link underpins the system's fail-safe mechanism.

---

## Communication Fail-Safe

Robots continuously monitor their connection to the controller. If the link drops for a defined period:

```text
Communication Lost
        ↓
Fail-Safe Triggered
        ↓
Stop Robot Movement
        ↓
Disable Combat Operation
```

This keeps a robot from continuing to operate uncontrolled if it loses contact with its controller.

---

## Remote Controller

Each robot is paired with its own **ESP32 remote controller**, which provides:

* Joystick-based movement
* Combat controls
* OLED display
* Health information
* Robot status
* Communication status
* User interface feedback

```text
Player 1 → Remote Controller 1 → Robot 1

Player 2 → Remote Controller 2 → Robot 2
```

---

## OLED User Interface

The OLED display gives the operator real-time information, including:

* Robot health
* Battery/status information
* Communication status
* Joystick activity
* Combat controls
* Robot state

This lets the operator monitor the robot's condition while actively controlling it.

---

## Robot States

```text
             ┌─────────────┐
             │    ACTIVE   │
             └──────┬──────┘
                    │
             Valid Hit Detected
                    │
                    ▼
             ┌─────────────┐
             │Health Update│
             └──────┬──────┘
                    │
             Health = 0?
              ↙           ↘
            No             Yes
            │               │
            ▼               ▼
        Continue          DEAD
         Combat        + Red Indicator
```

---

## Safety Features

### Communication Fail-Safe

Robot movement and combat operation stop automatically if communication with the controller is lost.

### DEAD State

When a robot's health reaches zero:

* Movement stops
* Combat operation is disabled
* Red indicators activate
* Robot status is updated

---

## Robot 1 Firmware

The Robot 1 firmware is split into separate files by development area:

| File                          | Main Area                          |
| ------------------------------ | ----------------------------------- |
| `Member1_DriveSystem.ino`     | Drive system and motor control      |
| `Member2_CombatActuators.ino` | Combat actuator control             |
| `Member3_SensorsHealth.ino`   | Sensors, hit detection, and health  |
| `Member4_Robot_Comm_Test.ino` | Robot communication testing         |
| `Member5_RemoteUI.ino`        | Remote controller UI                |
| `Remote1.ino`                 | Remote controller firmware          |
| `Robot1.ino`                  | Robot 1 main firmware               |
| `README.md`                   | Robot 1 firmware documentation      |

This modular structure reflects how the firmware was developed, tested, and integrated.

---

## Project Structure

```text
BattleBit/
│
├── firmware/
│   ├── robot-1/
│   │   ├── Member1_DriveSystem.ino
│   │   ├── Member2_CombatActuators.ino
│   │   ├── Member3_SensorsHealth.ino
│   │   ├── Member4_Robot_Comm_Test.ino
│   │   ├── Member5_RemoteUI.ino
│   │   ├── Remote1.ino
│   │   ├── Robot1.ino
│   │   └── README.md
│   │
│   └── robot-2/
│
├── hardware/
│   ├── circuit/
│   ├── CAD/
│   └── documentation/
│
├── remote/
│   ├── remote-1/
│   └── remote-2/
│
├── documentation/
│
└── README.md
```

---

## Technologies

* **ESP32**
* **Arduino IDE**
* **C/C++**
* **Mecanum Drive**
* **L298N Motor Driver**
* **TT DC Motors**
* **MG995 Servo**
* **MPU6050**
* **Magnetic Reed Switches**
* **OLED Display**
* **ESP32 Wireless Communication**

---

## Team Contributions

Responsibilities are divided by development area. The firmware itself is modularized for development, testing, and integration — this division reflects areas of ownership, not that each member built an entire subsystem alone.

| Member       | Name                     | Responsibility & Role                                |
| ------------ | ------------------------ | ----------------------------------------------------- |
| **Member 1** | **Perera O.S.G**         | **Robot Drive, Firmware Integration & Team Leader**   |
| **Member 2** | **Samarasinghe S.M.M.P** | **Combat Mechanism, Servo & Electromagnet Control**   |
| **Member 3** | **Jayasinghe K.T.S**     | **Sensors, Hit Detection & Health System**            |
| **Member 4** | **Thimandee H.A.D**      | **Wireless Communication & Communication Testing**    |
| **Member 5** | **Amhar M.N**            | **Remote Controller UI & OLED Interface**             |

---

## Project Objective

BattleBit aims to build a **controlled, interactive robotic combat platform** that brings together:

* Embedded systems
* Mechanical actuation
* Motor control
* Wireless communication
* Sensor systems
* Real-time health monitoring
* Human-operated remote control

The project shows how these hardware and software subsystems can be integrated into one coordinated, playable robotic platform.

---

## Project Information

| Category                    | Details                           |
| ---------------------------- | ---------------------------------- |
| **Project Name**            | BattleBit                          |
| **Project Type**            | Microcontroller-Based Application  |
| **Platform**                | ESP32                              |
| **Control Method**          | Manual Wireless Control            |
| **Combat Robots**           | 2                                   |
| **Remote Controllers**      | 2                                   |
| **Development Environment** | Arduino IDE                        |
| **Programming Language**    | C/C++                              |

---

## Status

**Development in Progress**
