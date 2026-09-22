/*
  =====================================================================
  BATTLEBIT - STAGE 1
  ROBOT 1 FIRMWARE  (ESP32 - Wi-Fi Access Point + UDP)
  =====================================================================
  Based on:
   - "BattleBit Stage 1 - Dual Robot Manual Combat System" (concept doc)
   - "Robot 1 + Remote 1 Wi-Fi AP + UDP Build Guide" (comms doc)

  This single file is organized into the 5 functional modules a 5-person
  team would normally split between them (see MEMBER markers below).
  Robot 2 uses an identical sketch with the SSID/port changed
  (see NOTES at bottom).

  ---------------------------------------------------------------------
  TEAM SPLIT (for reference / so each member can work on their block):
    MEMBER 1 - Drive System        : Mecanum motor control (L298N x2)
    MEMBER 2 - Combat Actuators    : Electromagnet relay + MG995 servo
    MEMBER 3 - Sensors & Health    : MPU6050, reed switch, health/dead logic
    MEMBER 4 - Communication/Power : Wi-Fi AP, UDP protocol, failsafe, battery
    MEMBER 5 - (works mainly on Remote1.ino, but shares packet struct below)
  ---------------------------------------------------------------------

  REQUIRED LIBRARIES (Arduino Library Manager):
    - ESP32Servo               (for MG995 servo)
    - Adafruit MPU6050
    - Adafruit Unified Sensor
    - Wire (built-in)
    - WiFi, WiFiUdp (built-in ESP32 core)

  !! ITEMS MARKED "TODO / CONFIRM" MUST BE FINALIZED DURING BUILD !!
  This matches Section 17 ("Items to Finalize") of the concept document:
  pin numbers, starting health, damage values, angle thresholds, servo
  limits, motor-driver channel wiring and UDP port/timeout are all
  placeholders here and must be tuned on the real hardware.
  =====================================================================
*/

#include <WiFi.h>
#include <WiFiUdp.h>
#include <Wire.h>
#include <ESP32Servo.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>

// =====================================================================
// MEMBER 4 - COMMUNICATION / POWER : Wi-Fi + UDP CONFIG
// =====================================================================
const char* AP_SSID     = "BATTLEBOT_R1";   // TODO: change to BATTLEBOT_R2 for robot 2
const char* AP_PASSWORD = "battlebit1";     // TODO CONFIRM: min 8 chars, set real password
IPAddress apIP(192, 168, 4, 1);
IPAddress apGateway(192, 168, 4, 1);
IPAddress apSubnet(255, 255, 255, 0);

const uint16_t CONTROL_PORT   = 4210;  // Remote -> Robot
const uint16_t TELEMETRY_PORT = 4211;  // Robot -> Remote
const unsigned long LINK_TIMEOUT_MS = 400; // TODO CONFIRM: comm-loss failsafe window
const unsigned long TELEMETRY_INTERVAL_MS = 100; // 10Hz status back to remote

WiFiUDP udp;
IPAddress remoteIP;           // learned automatically from first packet received
bool      remoteIPKnown = false;
unsigned long lastPacketTime = 0;
unsigned long lastTelemetryTime = 0;
uint32_t lastSeqReceived = 0;

// Shared packet structures (MUST match Remote1.ino exactly)
struct ControlPacket {
  uint32_t seq;
  int16_t  joyX;   // -100..100
  int16_t  joyY;   // -100..100
  uint8_t  pb1;    // magnet ON
  uint8_t  pb2;    // magnet OFF
  uint8_t  pb3;    // servo UP
  uint8_t  pb4;    // servo DOWN
};

struct TelemetryPacket {
  uint32_t seq;
  int16_t  health;
  float    batteryVoltage;
  float    angleX;
  float    angleY;
  uint8_t  magnetState;   // 0 off / 1 on
  uint8_t  servoState;    // 0 down / 1 up
  uint8_t  robotState;    // 0 alive / 1 dead
  uint8_t  linkOK;        // 1 = robot currently receiving control packets
};

// =====================================================================
// MEMBER 1 - DRIVE SYSTEM : Mecanum wheel pin map (2x L298N)
// TODO CONFIRM: final GPIOs once wiring diagram is fixed (Section 17)
// =====================================================================
// L298N #1 -> Front Left + Front Right
#define FL_IN1 16
#define FL_IN2 17
#define FL_EN  4     // PWM
#define FR_IN1 18
#define FR_IN2 19
#define FR_EN  5     // PWM

// L298N #2 -> Rear Left + Rear Right
#define RL_IN1 25
#define RL_IN2 26
#define RL_EN  27    // PWM
#define RR_IN1 32
#define RR_IN2 33
#define RR_EN  14    // PWM

const int PWM_FREQ = 5000;
const int PWM_RES  = 8;      // 0-255
const int JOY_DEADZONE = 8;  // % deadzone around center

// PWM channels (ESP32 LEDC)
const int CH_FL = 0, CH_FR = 1, CH_RL = 2, CH_RR = 3;

// =====================================================================
// MEMBER 2 - COMBAT ACTUATORS : Electromagnet relay + MG995 servo
// TODO CONFIRM: pins, servo safe angle limits after mechanical build
// =====================================================================
#define RELAY_MAGNET_PIN 13
Servo mg995;
#define SERVO_PIN 12
const int SERVO_ANGLE_DOWN = 20;   // TODO CONFIRM after arm geometry finalized
const int SERVO_ANGLE_UP   = 150;  // TODO CONFIRM after arm geometry finalized
int  currentServoAngle = SERVO_ANGLE_DOWN;
bool magnetOn = false;
bool servoUp  = false;

// =====================================================================
// MEMBER 3 - SENSORS & HEALTH SYSTEM
// TODO CONFIRM: starting health, damage per hit, angle thresholds (Section 17)
// =====================================================================
#define REED_SWITCH_PIN 34          // input only pin, needs external pull as required
#define LED_STATUS_PIN  15          // general status LED
#define LED_DEAD_PIN    2           // red dead-state LED on robot

Adafruit_MPU6050 mpu;
bool mpuOK = false;

const int16_t HEALTH_MAX = 100;          // TODO CONFIRM starting health
const int16_t DAMAGE_PER_HIT = 15;       // TODO CONFIRM damage per reed-switch trigger
const unsigned long REED_COOLDOWN_MS = 400; // debounce/cooldown so 1 hit != many hits
unsigned long lastReedTriggerTime = 0;

const float TILT_ANGLE_LIMIT_DEG = 45.0;     // TODO CONFIRM safe body-angle range
const unsigned long TILT_PENALTY_HOLD_MS = 1500; // must stay tilted this long
const int16_t TILT_DAMAGE = 5;               // TODO CONFIRM
unsigned long tiltStartTime = 0;
bool tiltPenaltyApplied = false;

int16_t health = HEALTH_MAX;
bool robotDead = false;

// =====================================================================
// MEMBER 4 (cont.) - BATTERY SENSING
// NOTE: Per build guide Section 14, the raw 3S pack CANNOT go directly
// into an ESP32 ADC pin. This assumes a voltage-divider circuit is
// added (TODO CONFIRM resistor values / scaling factor once built).
// =====================================================================
#define BATTERY_SENSE_PIN 35
const float BATTERY_DIVIDER_SCALE = 4.0; // TODO CONFIRM after divider is built & calibrated
const float ADC_REF_VOLTAGE = 3.3;
const int   ADC_MAX = 4095;

float readBatteryVoltage() {
  int raw = analogRead(BATTERY_SENSE_PIN);
  float vAtPin = (raw / (float)ADC_MAX) * ADC_REF_VOLTAGE;
  return vAtPin * BATTERY_DIVIDER_SCALE;
}

// =====================================================================
// SETUP
// =====================================================================
void setup() {
  Serial.begin(115200);
  delay(200);

  setupDrivePins();          // MEMBER 1
  setupCombatActuators();    // MEMBER 2
  setupSensorsAndHealth();   // MEMBER 3
  setupWiFiAndUDP();         // MEMBER 4

  Serial.println("Robot 1 ready.");
}

// =====================================================================
// MAIN LOOP
// =====================================================================
void loop() {
  handleIncomingControlPackets();   // MEMBER 4 / applies MEMBER 1 & 2 outputs
  checkCommsFailsafe();             // MEMBER 4
  updateHealthSystem();             // MEMBER 3
  updateDeadStateIndicators();      // MEMBER 3
  sendTelemetryIfDue();             // MEMBER 4
}

// =====================================================================
// MEMBER 1 - DRIVE SYSTEM FUNCTIONS
// =====================================================================
void setupDrivePins() {
  pinMode(FL_IN1, OUTPUT); pinMode(FL_IN2, OUTPUT);
  pinMode(FR_IN1, OUTPUT); pinMode(FR_IN2, OUTPUT);
  pinMode(RL_IN1, OUTPUT); pinMode(RL_IN2, OUTPUT);
  pinMode(RR_IN1, OUTPUT); pinMode(RR_IN2, OUTPUT);

  ledcSetup(CH_FL, PWM_FREQ, PWM_RES); ledcAttachPin(FL_EN, CH_FL);
  ledcSetup(CH_FR, PWM_FREQ, PWM_RES); ledcAttachPin(FR_EN, CH_FR);
  ledcSetup(CH_RL, PWM_FREQ, PWM_RES); ledcAttachPin(RL_EN, CH_RL);
  ledcSetup(CH_RR, PWM_FREQ, PWM_RES); ledcAttachPin(RR_EN, CH_RR);

  stopAllMotors();
}

void setWheel(int in1Pin, int in2Pin, int ch, int speedSigned) {
  // speedSigned: -255..255
  speedSigned = constrain(speedSigned, -255, 255);
  if (speedSigned >= 0) {
    digitalWrite(in1Pin, HIGH);
    digitalWrite(in2Pin, LOW);
  } else {
    digitalWrite(in1Pin, LOW);
    digitalWrite(in2Pin, HIGH);
  }
  ledcWrite(ch, abs(speedSigned));
}

void stopAllMotors() {
  setWheel(FL_IN1, FL_IN2, CH_FL, 0);
  setWheel(FR_IN1, FR_IN2, CH_FR, 0);
  setWheel(RL_IN1, RL_IN2, CH_RL, 0);
  setWheel(RR_IN1, RR_IN2, CH_RR, 0);
}

// Standard mecanum mixing. joyX/joyY are -100..100 (percent of stick travel).
// rotate is left as 0 here; TODO: wire a second axis/twist input if the
// joystick module supports rotation, per Section 4 "Rotational command".
void driveMecanum(int joyX, int joyY, int rotate = 0) {
  if (robotDead) { stopAllMotors(); return; }

  if (abs(joyX) < JOY_DEADZONE) joyX = 0;
  if (abs(joyY) < JOY_DEADZONE) joyY = 0;

  float x = joyX / 100.0;
  float y = joyY / 100.0;
  float r = rotate / 100.0;

  float fl = y + x + r;
  float fr = y - x - r;
  float rl = y - x + r;
  float rr = y + x - r;

  float maxMag = max(1.0f, max(max(fabs(fl), fabs(fr)), max(fabs(rl), fabs(rr))));
  fl /= maxMag; fr /= maxMag; rl /= maxMag; rr /= maxMag;

  setWheel(FL_IN1, FL_IN2, CH_FL, (int)(fl * 255));
  setWheel(FR_IN1, FR_IN2, CH_FR, (int)(fr * 255));
  setWheel(RL_IN1, RL_IN2, CH_RL, (int)(rl * 255));
  setWheel(RR_IN1, RR_IN2, CH_RR, (int)(rr * 255));
}

// =====================================================================
// MEMBER 2 - COMBAT ACTUATOR FUNCTIONS
// =====================================================================
void setupCombatActuators() {
  pinMode(RELAY_MAGNET_PIN, OUTPUT);
  digitalWrite(RELAY_MAGNET_PIN, LOW); // magnet OFF at boot

  mg995.attach(SERVO_PIN);
  mg995.write(SERVO_ANGLE_DOWN);
  currentServoAngle = SERVO_ANGLE_DOWN;
}

void setMagnet(bool on) {
  if (robotDead) on = false; // dead-state lock, Section 10
  magnetOn = on;
  digitalWrite(RELAY_MAGNET_PIN, on ? HIGH : LOW);
}

void setServo(bool up) {
  if (robotDead) return; // dead-state lock, Section 10
  servoUp = up;
  currentServoAngle = up ? SERVO_ANGLE_UP : SERVO_ANGLE_DOWN;
  mg995.write(currentServoAngle);
}

// =====================================================================
// MEMBER 3 - SENSORS & HEALTH SYSTEM FUNCTIONS
// =====================================================================
void setupSensorsAndHealth() {
  pinMode(REED_SWITCH_PIN, INPUT); // TODO CONFIRM pull-up/pull-down per final circuit
  pinMode(LED_STATUS_PIN, OUTPUT);
  pinMode(LED_DEAD_PIN, OUTPUT);
  digitalWrite(LED_STATUS_PIN, LOW);
  digitalWrite(LED_DEAD_PIN, LOW);

  Wire.begin();
  mpuOK = mpu.begin();
  if (mpuOK) {
    mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
    mpu.setGyroRange(MPU6050_RANGE_500_DEG);
    mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
    Serial.println("MPU6050 OK");
  } else {
    Serial.println("MPU6050 NOT FOUND - angle features disabled");
  }

  health = HEALTH_MAX;
  robotDead = false;
}

// Returns approximate body tilt angle (degrees from level) using accel only.
// TODO: consider complementary/Kalman filter with gyro for stability.
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
  if (health <= 0) {
    health = 0;
    robotDead = true;
    Serial.println("ROBOT 1 DEAD");
  }
}

void updateHealthSystem() {
  if (robotDead) return;

  // --- Reed switch damage event (Section 8) ---
  bool reedActive = (digitalRead(REED_SWITCH_PIN) == HIGH); // TODO CONFIRM active level
  unsigned long now = millis();
  if (reedActive && (now - lastReedTriggerTime > REED_COOLDOWN_MS)) {
    lastReedTriggerTime = now;
    applyDamage(DAMAGE_PER_HIT);
  }

  // --- MPU6050 tilt penalty (Section 7) ---
  float ax, ay;
  float tilt = readTiltAngleDeg(ax, ay);
  if (tilt > TILT_ANGLE_LIMIT_DEG) {
    if (tiltStartTime == 0) tiltStartTime = now;
    if (!tiltPenaltyApplied && (now - tiltStartTime > TILT_PENALTY_HOLD_MS)) {
      applyDamage(TILT_DAMAGE);
      tiltPenaltyApplied = true; // one penalty per sustained tilt event
    }
  } else {
    tiltStartTime = 0;
    tiltPenaltyApplied = false;
  }
}

void updateDeadStateIndicators() {
  digitalWrite(LED_DEAD_PIN, robotDead ? HIGH : LOW);
  digitalWrite(LED_STATUS_PIN, remoteIPKnown ? HIGH : LOW);
  if (robotDead) {
    stopAllMotors();
    setMagnet(false);
  }
}

// =====================================================================
// MEMBER 4 - COMMUNICATION / POWER FUNCTIONS
// =====================================================================
void setupWiFiAndUDP() {
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(apIP, apGateway, apSubnet);
  WiFi.softAP(AP_SSID, AP_PASSWORD);
  Serial.print("AP started: ");
  Serial.println(AP_SSID);
  Serial.print("AP IP: ");
  Serial.println(WiFi.softAPIP());

  udp.begin(CONTROL_PORT);
}

void handleIncomingControlPackets() {
  int packetSize = udp.parsePacket();
  if (packetSize == sizeof(ControlPacket)) {
    ControlPacket pkt;
    udp.read((uint8_t*)&pkt, sizeof(pkt));

    remoteIP = udp.remoteIP();
    remoteIPKnown = true;
    lastPacketTime = millis();

    if (pkt.seq > lastSeqReceived || lastSeqReceived == 0) {
      lastSeqReceived = pkt.seq;

      driveMecanum(pkt.joyX, pkt.joyY);       // MEMBER 1
      if (pkt.pb1) setMagnet(true);           // MEMBER 2
      if (pkt.pb2) setMagnet(false);          // MEMBER 2
      if (pkt.pb3) setServo(true);            // MEMBER 2
      if (pkt.pb4) setServo(false);           // MEMBER 2
    }
  } else if (packetSize > 0) {
    // wrong size packet - ignore, drain it
    udp.flush();
  }
}

void checkCommsFailsafe() {
  if (remoteIPKnown && (millis() - lastPacketTime > LINK_TIMEOUT_MS)) {
    // link lost - stop moving rather than hold last command (Section 12 / 15)
    stopAllMotors();
  }
}

void sendTelemetryIfDue() {
  unsigned long now = millis();
  if (!remoteIPKnown) return;
  if (now - lastTelemetryTime < TELEMETRY_INTERVAL_MS) return;
  lastTelemetryTime = now;

  TelemetryPacket t;
  t.seq = lastSeqReceived;
  t.health = health;
  t.batteryVoltage = readBatteryVoltage();
  float ax, ay;
  readTiltAngleDeg(ax, ay);
  t.angleX = ax;
  t.angleY = ay;
  t.magnetState = magnetOn ? 1 : 0;
  t.servoState = servoUp ? 1 : 0;
  t.robotState = robotDead ? 1 : 0;
  t.linkOK = (now - lastPacketTime <= LINK_TIMEOUT_MS) ? 1 : 0;

  udp.beginPacket(remoteIP, TELEMETRY_PORT);
  udp.write((uint8_t*)&t, sizeof(t));
  udp.endPacket();
}

/*
  =====================================================================
  NOTES FOR ROBOT 2:
  Duplicate this file, change:
    - AP_SSID to "BATTLEBOT_R2"
    - (optionally) CONTROL_PORT/TELEMETRY_PORT if you want both robots'
      networks distinguishable on the same bench during testing
  Everything else is identical per Section 15 of the concept document
  ("Robot 2 uses the same operating architecture as Robot 1").
  =====================================================================
*/
