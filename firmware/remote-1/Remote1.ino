/*
  =====================================================================
  BATTLEBIT - STAGE 1
  REMOTE 1 FIRMWARE  (ESP32 - Wi-Fi Station + UDP + OLED)
  =====================================================================
  Based on:
   - "BattleBit Stage 1 - Dual Robot Manual Combat System" (concept doc)
   - "Robot 1 + Remote 1 Wi-Fi AP + UDP Build Guide" (comms doc)

  Connects to Robot 1's access point (BATTLEBOT_R1), sends joystick +
  button commands over UDP, and displays returned telemetry on the OLED.

  ---------------------------------------------------------------------
  TEAM SPLIT (mirrors Robot1.ino so both halves of the system line up):
    MEMBER 5 - Remote Input & Display : Joystick, PB1-4, OLED, red LED
    MEMBER 4 - Communication/Power    : Wi-Fi STA, UDP protocol, failsafe
    (Members 1/2/3's logic lives on the robot side; this file only
     reads/display their results.)
  ---------------------------------------------------------------------

  REQUIRED LIBRARIES (Arduino Library Manager):
    - Adafruit SSD1306
    - Adafruit GFX Library
    - WiFi, WiFiUdp (built-in ESP32 core)

  !! ITEMS MARKED "TODO / CONFIRM" MUST BE FINALIZED DURING BUILD !!
  =====================================================================
*/

#include <WiFi.h>
#include <WiFiUdp.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// =====================================================================
// MEMBER 4 - COMMUNICATION / POWER : Wi-Fi + UDP CONFIG
// (must match Robot1.ino exactly)
// =====================================================================
const char* AP_SSID     = "BATTLEBOT_R1";  // TODO: BATTLEBOT_R2 for remote 2
const char* AP_PASSWORD = "battlebit1";
IPAddress robotIP(192, 168, 4, 1);         // Robot 1 AP default IP

const uint16_t CONTROL_PORT   = 4210; // Remote -> Robot
const uint16_t TELEMETRY_PORT = 4211; // Robot -> Remote
const unsigned long SEND_INTERVAL_MS   = 50;  // 20Hz control rate
const unsigned long LINK_TIMEOUT_MS    = 400; // must match robot's failsafe window

WiFiUDP udp;
uint32_t sendSeq = 0;
unsigned long lastSendTime = 0;
unsigned long lastTelemetryReceivedTime = 0;
bool linkOK = false;

// Shared packet structures (MUST match Robot1.ino exactly)
struct ControlPacket {
  uint32_t seq;
  int16_t  joyX;
  int16_t  joyY;
  uint8_t  pb1;
  uint8_t  pb2;
  uint8_t  pb3;
  uint8_t  pb4;
};

struct TelemetryPacket {
  uint32_t seq;
  int16_t  health;
  float    batteryVoltage;
  float    angleX;
  float    angleY;
  uint8_t  magnetState;
  uint8_t  servoState;
  uint8_t  robotState; // 0 alive / 1 dead
  uint8_t  linkOK;
};

TelemetryPacket lastTelemetry = {0, 100, 0, 0, 0, 0, 0, 0, 0};

// =====================================================================
// MEMBER 5 - REMOTE INPUT : Joystick + Push Buttons
// TODO CONFIRM: final GPIOs once remote enclosure/wiring is fixed
// =====================================================================
#define JOY_X_PIN 34   // analog
#define JOY_Y_PIN 35   // analog
const int ADC_CENTER = 2048; // TODO CONFIRM after calibrating your joystick module
const int ADC_MAX_TRAVEL = 2048;

#define PB1_PIN 25 // Magnet ON
#define PB2_PIN 26 // Magnet OFF
#define PB3_PIN 27 // Servo UP
#define PB4_PIN 14 // Servo DOWN
// TODO CONFIRM: buttons wired with INPUT_PULLUP -> active LOW, as assumed below

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

bool readDebounced(DebouncedButton &b) {
  bool reading = (digitalRead(b.pin) == LOW); // active LOW with INPUT_PULLUP
  if (reading != b.lastReading) {
    b.lastChangeTime = millis();
    b.lastReading = reading;
  }
  if (millis() - b.lastChangeTime > BUTTON_DEBOUNCE_MS) {
    b.stableState = reading;
  }
  return b.stableState;
}

// =====================================================================
// MEMBER 5 - OLED DISPLAY + RED DEAD-STATE LED
// TODO CONFIRM: pins/I2C address for your specific OLED module
// =====================================================================
#define OLED_WIDTH  128
#define OLED_HEIGHT 64
#define OLED_ADDR   0x3C
Adafruit_SSD1306 display(OLED_WIDTH, OLED_HEIGHT, &Wire, -1);
bool oledOK = false;

#define LED_DEAD_PIN 2 // red indicator, turns ON when paired robot dies

// =====================================================================
// SETUP
// =====================================================================
void setup() {
  Serial.begin(115200);
  delay(200);

  setupInputs();      // MEMBER 5
  setupDisplay();     // MEMBER 5
  setupWiFiAndUDP();  // MEMBER 4

  Serial.println("Remote 1 ready.");
}

// =====================================================================
// MAIN LOOP
// =====================================================================
void loop() {
  connectWiFiIfNeeded();        // MEMBER 4
  sendControlPacketIfDue();     // MEMBER 4 (reads MEMBER 5 inputs)
  receiveTelemetry();           // MEMBER 4
  updateLinkStatus();           // MEMBER 4
  updateDeadIndicatorLED();     // MEMBER 5
  updateOLED();                 // MEMBER 5
}

// =====================================================================
// MEMBER 5 - INPUT FUNCTIONS
// =====================================================================
void setupInputs() {
  pinMode(PB1_PIN, INPUT_PULLUP);
  pinMode(PB2_PIN, INPUT_PULLUP);
  pinMode(PB3_PIN, INPUT_PULLUP);
  pinMode(PB4_PIN, INPUT_PULLUP);
  pinMode(LED_DEAD_PIN, OUTPUT);
  digitalWrite(LED_DEAD_PIN, LOW);
  analogReadResolution(12); // ESP32 default ADC 0-4095
}

// Returns joystick reading scaled to -100..100
void readJoystick(int16_t &outX, int16_t &outY) {
  int rawX = analogRead(JOY_X_PIN);
  int rawY = analogRead(JOY_Y_PIN);
  int dx = rawX - ADC_CENTER;
  int dy = rawY - ADC_CENTER;
  outX = constrain(map(dx, -ADC_MAX_TRAVEL, ADC_MAX_TRAVEL, -100, 100), -100, 100);
  outY = constrain(map(dy, -ADC_MAX_TRAVEL, ADC_MAX_TRAVEL, -100, 100), -100, 100);
}

// =====================================================================
// MEMBER 5 - OLED FUNCTIONS
// =====================================================================
void setupDisplay() {
  Wire.begin();
  oledOK = display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR);
  if (oledOK) {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println("BATTLEBIT - ROBOT 1");
    display.println("Starting...");
    display.display();
  } else {
    Serial.println("OLED NOT FOUND");
  }
}

void updateOLED() {
  if (!oledOK) return;

  int16_t jx, jy;
  readJoystick(jx, jy);
  bool b1 = readDebounced(pb1);
  bool b2 = readDebounced(pb2);
  bool b3 = readDebounced(pb3);
  bool b4 = readDebounced(pb4);

  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("ROBOT 1");

  display.print("LINK: ");
  display.println(linkOK ? "OK" : (WiFi.status() == WL_CONNECTED ? "LOST" : "CONNECTING"));

  display.print("HP: ");
  display.println(lastTelemetry.health);

  display.print("BAT: ");
  display.print(lastTelemetry.batteryVoltage, 1);
  display.println("V");

  display.print("ANGLE: ");
  display.print((int)lastTelemetry.angleX);
  display.print(",");
  display.println((int)lastTelemetry.angleY);

  display.print("JOY: ");
  display.print(jx);
  display.print(",");
  display.println(jy);

  display.print("MAG:");
  display.print(lastTelemetry.magnetState ? "ON " : "OFF");
  display.print(" SRV:");
  display.println(lastTelemetry.servoState ? "UP" : "DN");

  display.print("PB:");
  display.print(b1 ? "1" : "-");
  display.print(b2 ? "2" : "-");
  display.print(b3 ? "3" : "-");
  display.println(b4 ? "4" : "-");

  display.print("STATE: ");
  display.println(lastTelemetry.robotState ? "DEAD" : "ALIVE");

  display.display();
}

void updateDeadIndicatorLED() {
  digitalWrite(LED_DEAD_PIN, lastTelemetry.robotState ? HIGH : LOW);
}

// =====================================================================
// MEMBER 4 - COMMUNICATION / POWER FUNCTIONS
// =====================================================================
void setupWiFiAndUDP() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(AP_SSID, AP_PASSWORD);
  Serial.print("Connecting to ");
  Serial.println(AP_SSID);

  udp.begin(TELEMETRY_PORT);
}

void connectWiFiIfNeeded() {
  if (WiFi.status() != WL_CONNECTED) {
    static unsigned long lastAttempt = 0;
    if (millis() - lastAttempt > 3000) {
      lastAttempt = millis();
      WiFi.disconnect();
      WiFi.begin(AP_SSID, AP_PASSWORD);
      Serial.println("Retrying Wi-Fi connection...");
    }
  }
}

void sendControlPacketIfDue() {
  if (WiFi.status() != WL_CONNECTED) return;
  if (millis() - lastSendTime < SEND_INTERVAL_MS) return;
  lastSendTime = millis();

  ControlPacket pkt;
  pkt.seq = ++sendSeq;
  readJoystick(pkt.joyX, pkt.joyY);
  pkt.pb1 = readDebounced(pb1) ? 1 : 0;
  pkt.pb2 = readDebounced(pb2) ? 1 : 0;
  pkt.pb3 = readDebounced(pb3) ? 1 : 0;
  pkt.pb4 = readDebounced(pb4) ? 1 : 0;

  udp.beginPacket(robotIP, CONTROL_PORT);
  udp.write((uint8_t*)&pkt, sizeof(pkt));
  udp.endPacket();
}

void receiveTelemetry() {
  int packetSize = udp.parsePacket();
  if (packetSize == sizeof(TelemetryPacket)) {
    TelemetryPacket t;
    udp.read((uint8_t*)&t, sizeof(t));
    lastTelemetry = t;
    lastTelemetryReceivedTime = millis();
  } else if (packetSize > 0) {
    udp.flush();
  }
}

void updateLinkStatus() {
  linkOK = (WiFi.status() == WL_CONNECTED) &&
           (millis() - lastTelemetryReceivedTime < LINK_TIMEOUT_MS) &&
           (lastTelemetryReceivedTime != 0);
}

/*
  =====================================================================
  NOTES FOR REMOTE 2:
  Duplicate this file, change:
    - AP_SSID to "BATTLEBOT_R2" (must match Robot 2's AP name)
  Everything else stays the same per Section 15 of the concept document.
  =====================================================================
*/
