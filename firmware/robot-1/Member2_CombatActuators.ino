/*
  =====================================================================
  MEMBER 2 - COMBAT ACTUATORS (standalone test sketch)
  Robot 1 - Electromagnet (relay) + MG995 Servo lift/throw mechanism
  =====================================================================
  Purpose: Test the electromagnet relay and MG995 servo mechanism on
  their own before wiring into the full Robot1.ino. Matches build
  guide steps: "Add electromagnet control" / "Add MG995 control".

  NO Wi-Fi, NO motors, NO sensors here — just the two combat outputs,
  controlled from Serial so Member 2 can verify wiring, relay logic
  level, and safe servo travel independently.

  HOW TO TEST:
    1. Upload this sketch.
    2. Open Serial Monitor at 115200 baud.
    3. Type a letter + Enter:
         1 = magnet ON     2 = magnet OFF
         3 = servo UP       4 = servo DOWN
         + / - = nudge servo angle by 5 degrees (use this to FIND the
                 real safe UP/DOWN limits once the arm is assembled,
                 then update SERVO_ANGLE_UP/DOWN below)

  IMPORTANT (from build guide Section 10 / 9):
    - Electromagnet is an inductive, high-current load. Do NOT power it
      from an ESP32 GPIO — it must go through the relay module as shown.
    - Do not power a loaded MG995 from the ESP32 3.3V output; give it
      its own regulated servo supply with a shared ground to the ESP32.

  TODO CONFIRM: pins, and SERVO_ANGLE_UP/DOWN once the mechanical arm
  geometry and safe travel limits are finalized (Section 17).
  =====================================================================
*/

#include <ESP32Servo.h>

#define RELAY_MAGNET_PIN 13
#define SERVO_PIN 12

Servo mg995;

int SERVO_ANGLE_DOWN = 20;   // TODO CONFIRM after arm geometry finalized
int SERVO_ANGLE_UP   = 150;  // TODO CONFIRM after arm geometry finalized
int currentAngle = 20;

bool magnetOn = false;

void setup() {
  Serial.begin(115200);
  delay(200);

  pinMode(RELAY_MAGNET_PIN, OUTPUT);
  digitalWrite(RELAY_MAGNET_PIN, LOW); // magnet OFF at boot

  mg995.attach(SERVO_PIN);
  currentAngle = SERVO_ANGLE_DOWN;
  mg995.write(currentAngle);

  Serial.println("Member 2 - Combat Actuators test ready.");
  Serial.println("Commands: 1=magnet ON  2=magnet OFF  3=servo UP  4=servo DOWN  +/- = nudge angle");
}

void loop() {
  if (Serial.available()) {
    char c = Serial.read();
    switch (c) {
      case '1':
        magnetOn = true;
        digitalWrite(RELAY_MAGNET_PIN, HIGH);
        Serial.println("MAGNET ON");
        break;
      case '2':
        magnetOn = false;
        digitalWrite(RELAY_MAGNET_PIN, LOW);
        Serial.println("MAGNET OFF");
        break;
      case '3':
        currentAngle = SERVO_ANGLE_UP;
        mg995.write(currentAngle);
        Serial.println("SERVO UP");
        break;
      case '4':
        currentAngle = SERVO_ANGLE_DOWN;
        mg995.write(currentAngle);
        Serial.println("SERVO DOWN");
        break;
      case '+':
        currentAngle = constrain(currentAngle + 5, 0, 180);
        mg995.write(currentAngle);
        Serial.print("Angle nudged to: "); Serial.println(currentAngle);
        break;
      case '-':
        currentAngle = constrain(currentAngle - 5, 0, 180);
        mg995.write(currentAngle);
        Serial.print("Angle nudged to: "); Serial.println(currentAngle);
        break;
      default: break;
    }
  }
}
