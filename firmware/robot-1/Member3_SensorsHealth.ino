/*
  =====================================================================
  MEMBER 3 - SENSORS & HEALTH SYSTEM (standalone test sketch)
  Robot 1 - MPU6050 body-angle + Magnetic Reed Switch + Health/Dead logic
  =====================================================================
  Purpose: Bring up the MPU6050 and reed switch on their own, and prove
  out the health/dead-state rules on the Serial Monitor before they're
  wired into the full Robot1.ino. Matches build guide step: "Connect
  MPU6050 and magnetic reed switch. Test each sensor locally before
  integrating health logic."

  NO Wi-Fi, NO motors, NO actuators here.

  HOW TO TEST:
    1. Upload this sketch. Open Serial Monitor at 115200 baud.
    2. Tilt the robot body past TILT_ANGLE_LIMIT_DEG and hold it — after
       TILT_PENALTY_HOLD_MS you should see one damage event and health
       drop by TILT_DAMAGE.
    3. Trigger the reed switch — health should drop by DAMAGE_PER_HIT
       once per trigger (not many times per press, thanks to the
       cooldown/debounce).
    4. Watch health count down to 0 and confirm DEAD state + LED prints.
    5. Type 'r' + Enter any time to reset health for another test run.

  REQUIRED LIBRARIES: Adafruit MPU6050, Adafruit Unified Sensor

  TODO CONFIRM (Section 17 of concept doc):
    - Starting health value and damage per reed-switch hit
    - MPU6050 angle threshold, hold time, and tilt damage
    - Reed switch active level / pull resistor arrangement
  =====================================================================
*/

#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>

#define REED_SWITCH_PIN 34   // TODO CONFIRM active level / pull resistor
#define LED_DEAD_PIN    2    // red dead-state LED on robot

Adafruit_MPU6050 mpu;
bool mpuOK = false;

const int16_t HEALTH_MAX = 100;              // TODO CONFIRM
const int16_t DAMAGE_PER_HIT = 15;           // TODO CONFIRM
const unsigned long REED_COOLDOWN_MS = 400;  // debounce/cooldown
unsigned long lastReedTriggerTime = 0;

const float TILT_ANGLE_LIMIT_DEG = 45.0;         // TODO CONFIRM
const unsigned long TILT_PENALTY_HOLD_MS = 1500; // TODO CONFIRM
const int16_t TILT_DAMAGE = 5;                   // TODO CONFIRM
unsigned long tiltStartTime = 0;
bool tiltPenaltyApplied = false;

int16_t health = HEALTH_MAX;
bool robotDead = false;

unsigned long lastPrint = 0;

void setup() {
  Serial.begin(115200);
  delay(200);

  pinMode(REED_SWITCH_PIN, INPUT);
  pinMode(LED_DEAD_PIN, OUTPUT);
  digitalWrite(LED_DEAD_PIN, LOW);

  Wire.begin();
  mpuOK = mpu.begin();
  if (mpuOK) {
    mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
    mpu.setGyroRange(MPU6050_RANGE_500_DEG);
    mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
    Serial.println("MPU6050 OK");
  } else {
    Serial.println("MPU6050 NOT FOUND - check wiring/address");
  }

  Serial.println("Member 3 - Sensors & Health test ready. Type 'r' to reset health.");
}

void loop() {
  if (Serial.available()) {
    char c = Serial.read();
    if (c == 'r') {
      health = HEALTH_MAX;
      robotDead = false;
      Serial.println(">>> HEALTH RESET <<<");
    }
  }

  updateHealthSystem();
  updateDeadIndicator();

  if (millis() - lastPrint > 500) {
    lastPrint = millis();
    float ax, ay;
    float tilt = readTiltAngleDeg(ax, ay);
    Serial.print("HP: "); Serial.print(health);
    Serial.print("  Tilt: "); Serial.print(tilt, 1);
    Serial.print(" deg  State: "); Serial.println(robotDead ? "DEAD" : "ALIVE");
  }
}

float readTiltAngleDeg(float &angleX, float &angleY) {
  if (!mpuOK) { angleX = 0; angleY = 0; return 0; }
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);
  angleX = atan2(a.acceleration.y, a.acceleration.z) * 180.0 / PI;
  angleY = atan2(a.acceleration.x, a.acceleration.z) * 180.0 / PI;
  return max(fabs(angleX), fabs(angleY));
}

void applyDamage(int16_t amount) {
  if (robotDead) return;
  health -= amount;
  Serial.print(">>> DAMAGE EVENT: -"); Serial.println(amount);
  if (health <= 0) {
    health = 0;
    robotDead = true;
    Serial.println(">>> ROBOT DEAD <<<");
  }
}

void updateHealthSystem() {
  if (robotDead) return;

  bool reedActive = (digitalRead(REED_SWITCH_PIN) == HIGH); // TODO CONFIRM active level
  unsigned long now = millis();
  if (reedActive && (now - lastReedTriggerTime > REED_COOLDOWN_MS)) {
    lastReedTriggerTime = now;
    applyDamage(DAMAGE_PER_HIT);
  }

  float ax, ay;
  float tilt = readTiltAngleDeg(ax, ay);
  if (tilt > TILT_ANGLE_LIMIT_DEG) {
    if (tiltStartTime == 0) tiltStartTime = now;
    if (!tiltPenaltyApplied && (now - tiltStartTime > TILT_PENALTY_HOLD_MS)) {
      applyDamage(TILT_DAMAGE);
      tiltPenaltyApplied = true;
    }
  } else {
    tiltStartTime = 0;
    tiltPenaltyApplied = false;
  }
}

void updateDeadIndicator() {
  digitalWrite(LED_DEAD_PIN, robotDead ? HIGH : LOW);
}
