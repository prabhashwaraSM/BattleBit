/*
  =====================================================================
  MEMBER 5 - REMOTE INPUT & DISPLAY (standalone test sketch)
  Remote 1 - Joystick + PB1-PB4 + OLED + red dead LED
  =====================================================================
  Purpose: Build and test the remote's input/output hardware entirely
  on its own — no Wi-Fi, no robot needed yet. Matches build guide
  steps: "Build Remote input section... Read and print all values
  locally before sending them wirelessly" and "Add Remote OLED."

  HOW TO TEST:
    1. Upload this sketch. Open Serial Monitor at 115200 baud (optional
       — the OLED itself is the main output).
    2. Move the joystick — X/Y values on the OLED (and Serial) should
       change smoothly and sit near 0,0 at rest. If they don't sit at
       0,0, adjust ADC_CENTER below to match your actual joystick.
    3. Press PB1-PB4 — the OLED should show which one is active.
    4. This sketch fakes HP counting down from 100 to 0 automatically
       so you can confirm the "STATE: ALIVE" -> "STATE: DEAD" switch
       and the red LED turning on, without needing the robot's real
       health system yet. Type 'r' + Enter to reset the fake HP.

  REQUIRED LIBRARIES: Adafruit SSD1306, Adafruit GFX Library

  TODO CONFIRM: joystick/button/OLED pins, ADC_CENTER calibration,
  OLED I2C address (Section 17 / Section 11 of the documents).
  =====================================================================
*/

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define JOY_X_PIN 34
#define JOY_Y_PIN 35
const int ADC_CENTER = 2048;       // TODO CONFIRM after calibrating your joystick
const int ADC_MAX_TRAVEL = 2048;

#define PB1_PIN 25 // Magnet ON
#define PB2_PIN 26 // Magnet OFF
#define PB3_PIN 27 // Servo UP
#define PB4_PIN 14 // Servo DOWN
// Wired with INPUT_PULLUP -> active LOW. TODO CONFIRM matches your wiring.

const unsigned long BUTTON_DEBOUNCE_MS = 40;
struct DebouncedButton {
  uint8_t pin;
  bool lastReading;
  bool stableState;
  unsigned long lastChangeTime;
};
DebouncedButton pb1 = {PB1_PIN, false, false, 0};
DebouncedButton pb2 = {PB2_PIN, false, false, 0};
DebouncedButton pb3 = {PB3_PIN, false, false, 0};
DebouncedButton pb4 = {PB4_PIN, false, false, 0};

#define OLED_WIDTH  128
#define OLED_HEIGHT 64
#define OLED_ADDR   0x3C
Adafruit_SSD1306 display(OLED_WIDTH, OLED_HEIGHT, &Wire, -1);
bool oledOK = false;

#define LED_DEAD_PIN 2

// Fake telemetry so the screen layout can be tested without the robot
int16_t fakeHP = 100;
unsigned long lastHPTick = 0;

void setup() {
  Serial.begin(115200);
  delay(200);

  pinMode(PB1_PIN, INPUT_PULLUP);
  pinMode(PB2_PIN, INPUT_PULLUP);
  pinMode(PB3_PIN, INPUT_PULLUP);
  pinMode(PB4_PIN, INPUT_PULLUP);
  pinMode(LED_DEAD_PIN, OUTPUT);
  digitalWrite(LED_DEAD_PIN, LOW);
  analogReadResolution(12);

  Wire.begin();
  oledOK = display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR);
  if (oledOK) {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println("Member 5 UI test");
    display.println("Starting...");
    display.display();
  } else {
    Serial.println("OLED NOT FOUND - check wiring/address");
  }

  Serial.println("Member 5 - Remote UI test ready. Type 'r' to reset fake HP.");
}

void loop() {
  if (Serial.available()) {
    char c = Serial.read();
    if (c == 'r') { fakeHP = 100; Serial.println(">>> FAKE HP RESET <<<"); }
  }

  // fake HP ticks down every 3s just so DEAD state is reachable in testing
  if (fakeHP > 0 && millis() - lastHPTick > 3000) {
    lastHPTick = millis();
    fakeHP -= 10;
    if (fakeHP < 0) fakeHP = 0;
  }

  updateOLED();
  digitalWrite(LED_DEAD_PIN, fakeHP <= 0 ? HIGH : LOW);
}

void readJoystick(int16_t &outX, int16_t &outY) {
  int rawX = analogRead(JOY_X_PIN);
  int rawY = analogRead(JOY_Y_PIN);
  int dx = rawX - ADC_CENTER;
  int dy = rawY - ADC_CENTER;
  outX = constrain(map(dx, -ADC_MAX_TRAVEL, ADC_MAX_TRAVEL, -100, 100), -100, 100);
  outY = constrain(map(dy, -ADC_MAX_TRAVEL, ADC_MAX_TRAVEL, -100, 100), -100, 100);
}

bool readDebounced(DebouncedButton &b) {
  bool reading = (digitalRead(b.pin) == LOW);
  if (reading != b.lastReading) {
    b.lastChangeTime = millis();
    b.lastReading = reading;
  }
  if (millis() - b.lastChangeTime > BUTTON_DEBOUNCE_MS) {
    b.stableState = reading;
  }
  return b.stableState;
}

void updateOLED() {
  int16_t jx, jy;
  readJoystick(jx, jy);
  bool b1 = readDebounced(pb1);
  bool b2 = readDebounced(pb2);
  bool b3 = readDebounced(pb3);
  bool b4 = readDebounced(pb4);

  if (!oledOK) {
    // still print to Serial so Member 5 can debug without a working OLED
    Serial.print("JOY:"); Serial.print(jx); Serial.print(","); Serial.print(jy);
    Serial.print(" PB:"); Serial.print(b1); Serial.print(b2); Serial.print(b3); Serial.print(b4);
    Serial.print(" HP:"); Serial.println(fakeHP);
    return;
  }

  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("ROBOT 1 (fake data)");

  display.print("HP: "); display.println(fakeHP);
  display.print("JOY: "); display.print(jx); display.print(","); display.println(jy);

  display.print("PB:");
  display.print(b1 ? "1" : "-");
  display.print(b2 ? "2" : "-");
  display.print(b3 ? "3" : "-");
  display.println(b4 ? "4" : "-");

  display.print("STATE: ");
  display.println(fakeHP <= 0 ? "DEAD" : "ALIVE");

  display.display();
}
