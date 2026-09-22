/*
  =====================================================================
  MEMBER 1 - DRIVE SYSTEM (standalone test sketch)
  Robot 1 - Mecanum Wheel Drive via 2x L298N
  =====================================================================
  Purpose: Build and test the 4-motor mecanum drive on its own, BEFORE
  it's wired into the full Robot1.ino. This matches the build guide's
  recommended sequence: "Connect and test each TT motor. Then implement
  mecanum movement and finally map joystick input to movement."

  This sketch has NO Wi-Fi, NO servo, NO sensors — just motors, driven
  by a simple test routine + optional Serial commands, so Member 1 can
  verify wiring/polarity independently of everyone else's work.

  Once this is confirmed working, the same functions
  (setupDrivePins / setWheel / stopAllMotors / driveMecanum) are copied
  as-is into Robot1.ino — keep them identical so integration is a
  copy-paste, not a rewrite.

  HOW TO TEST:
    1. Upload this sketch.
    2. Open Serial Monitor at 115200 baud.
    3. Type a letter + Enter:
         f = forward   b = backward
         l = strafe left   r = strafe right
         q = rotate left   e = rotate right
         s = stop
    4. Confirm each wheel spins the correct direction. Fix wiring
       (swap IN1/IN2) rather than fix it in software, so the software
       stays simple.

  TODO CONFIRM: pins below, and the final L298N channel arrangement,
  per Section 17 of the concept document ("Motor-driver quantity/
  channel arrangement for four TT motors").
  =====================================================================
*/

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
const int TEST_SPEED = 180;  // 0-255, used by the manual test routine

const int CH_FL = 0, CH_FR = 1, CH_RL = 2, CH_RR = 3;

void setup() {
  Serial.begin(115200);
  delay(200);
  setupDrivePins();
  Serial.println("Member 1 - Drive System test ready.");
  Serial.println("Commands: f/b/l/r/q/e/s + Enter");
}

void loop() {
  if (Serial.available()) {
    char c = Serial.read();
    switch (c) {
      case 'f': driveMecanum(0, 100, 0);   Serial.println("FORWARD"); break;
      case 'b': driveMecanum(0, -100, 0);  Serial.println("BACKWARD"); break;
      case 'l': driveMecanum(-100, 0, 0);  Serial.println("STRAFE LEFT"); break;
      case 'r': driveMecanum(100, 0, 0);   Serial.println("STRAFE RIGHT"); break;
      case 'q': driveMecanum(0, 0, -100);  Serial.println("ROTATE LEFT"); break;
      case 'e': driveMecanum(0, 0, 100);   Serial.println("ROTATE RIGHT"); break;
      case 's': stopAllMotors();           Serial.println("STOP"); break;
      default: break; // ignore newline/other chars
    }
  }
}

// =====================================================================
// DRIVE FUNCTIONS (copy these unchanged into Robot1.ino)
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

// joyX/joyY/rotate expected as -100..100 (percent), matching the
// joystick scale used by Member 5 / Member 4's control packet.
void driveMecanum(int joyX, int joyY, int rotate) {
  float x = joyX / 100.0;
  float y = joyY / 100.0;
  float r = rotate / 100.0;

  float fl = y + x + r;
  float fr = y - x - r;
  float rl = y - x + r;
  float rr = y + x - r;

  float maxMag = max(1.0f, max(max(fabs(fl), fabs(fr)), max(fabs(rl), fabs(rr))));
  fl /= maxMag; fr /= maxMag; rl /= maxMag; rr /= maxMag;

  int spd = TEST_SPEED;
  setWheel(FL_IN1, FL_IN2, CH_FL, (int)(fl * spd));
  setWheel(FR_IN1, FR_IN2, CH_FR, (int)(fr * spd));
  setWheel(RL_IN1, RL_IN2, CH_RL, (int)(rl * spd));
  setWheel(RR_IN1, RR_IN2, CH_RR, (int)(rr * spd));
}
