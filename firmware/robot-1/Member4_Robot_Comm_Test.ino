/*
  =====================================================================
  MEMBER 4 - COMMUNICATION & POWER (standalone test sketch, ROBOT side)
  Robot 1 - Wi-Fi Access Point + UDP protocol
  =====================================================================
  Purpose: Prove out the Wi-Fi AP + UDP link on its own, in both
  directions, before anyone's motors/sensors/actuators are wired in.
  Matches build guide steps 1-5: create AP, connect remote, one-way
  UDP, then two-way UDP.

  Run this sketch on the ROBOT's ESP32 at the same time as
  Member4_Remote_Comm_Test.ino runs on the REMOTE's ESP32.

  WHAT IT DOES:
    - Creates the BATTLEBOT_R1 access point.
    - Listens for a ControlPacket from the remote and prints it.
    - Sends a TelemetryPacket back with a fake/incrementing health
      value so Member 5 (OLED) and Member 4 (remote side) can verify
      the round trip without needing real sensors yet.
    - Implements the comms failsafe: if no packet arrives within
      LINK_TIMEOUT_MS, it prints "LINK LOST" (stands in for "stop
      motors" until Member 1's code is merged in).

  This packet struct is the CONTRACT between Robot1.ino and
  Remote1.ino — keep it byte-for-byte identical in both, and in the
  final Overall_Final files.

  TODO CONFIRM: SSID/password, UDP ports, LINK_TIMEOUT_MS (Section 17).
  =====================================================================
*/

#include <WiFi.h>
#include <WiFiUdp.h>

const char* AP_SSID     = "BATTLEBOT_R1";
const char* AP_PASSWORD = "battlebit1";
IPAddress apIP(192, 168, 4, 1);
IPAddress apGateway(192, 168, 4, 1);
IPAddress apSubnet(255, 255, 255, 0);

const uint16_t CONTROL_PORT   = 4210;
const uint16_t TELEMETRY_PORT = 4211;
const unsigned long LINK_TIMEOUT_MS = 400;
const unsigned long TELEMETRY_INTERVAL_MS = 100;

WiFiUDP udp;
IPAddress remoteIP;
bool remoteIPKnown = false;
unsigned long lastPacketTime = 0;
unsigned long lastTelemetryTime = 0;

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
  uint8_t  robotState;
  uint8_t  linkOK;
};

int16_t fakeHealth = 100; // stand-in until Member 3's code is merged in

void setup() {
  Serial.begin(115200);
  delay(200);

  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(apIP, apGateway, apSubnet);
  WiFi.softAP(AP_SSID, AP_PASSWORD);
  Serial.print("AP started: "); Serial.println(AP_SSID);
  Serial.print("AP IP: "); Serial.println(WiFi.softAPIP());

  udp.begin(CONTROL_PORT);
  Serial.println("Member 4 (Robot side) comms test ready.");
}

void loop() {
  handleIncoming();
  checkFailsafe();
  sendTelemetryIfDue();
}

void handleIncoming() {
  int packetSize = udp.parsePacket();
  if (packetSize == sizeof(ControlPacket)) {
    ControlPacket pkt;
    udp.read((uint8_t*)&pkt, sizeof(pkt));
    remoteIP = udp.remoteIP();
    remoteIPKnown = true;
    lastPacketTime = millis();

    Serial.print("RX seq="); Serial.print(pkt.seq);
    Serial.print(" joyX="); Serial.print(pkt.joyX);
    Serial.print(" joyY="); Serial.print(pkt.joyY);
    Serial.print(" pb1-4="); Serial.print(pkt.pb1);
    Serial.print(pkt.pb2); Serial.print(pkt.pb3); Serial.println(pkt.pb4);
  } else if (packetSize > 0) {
    udp.flush();
  }
}

void checkFailsafe() {
  if (remoteIPKnown && (millis() - lastPacketTime > LINK_TIMEOUT_MS)) {
    Serial.println("LINK LOST - (would stop motors here)");
  }
}

void sendTelemetryIfDue() {
  unsigned long now = millis();
  if (!remoteIPKnown) return;
  if (now - lastTelemetryTime < TELEMETRY_INTERVAL_MS) return;
  lastTelemetryTime = now;

  fakeHealth = max(0, fakeHealth - 0); // placeholder, real health comes from Member 3

  TelemetryPacket t;
  t.seq = 0;
  t.health = fakeHealth;
  t.batteryVoltage = 11.1; // fake value for testing
  t.angleX = 0;
  t.angleY = 0;
  t.magnetState = 0;
  t.servoState = 0;
  t.robotState = 0;
  t.linkOK = (now - lastPacketTime <= LINK_TIMEOUT_MS) ? 1 : 0;

  udp.beginPacket(remoteIP, TELEMETRY_PORT);
  udp.write((uint8_t*)&t, sizeof(t));
  udp.endPacket();
}
