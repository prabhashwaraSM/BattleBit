#include <WiFi.h>
#include <esp_now.h>

#define JOY_X 34
#define JOY_Y 35
#define JOY_SW 32

// Robot ESP32 MAC address
uint8_t robotAddress[] = {
  0xC8, 0x2E, 0x18, 0x55, 0xD9, 0xE0
};

esp_now_peer_info_t peerInfo;

void sendCommand(char command) {
  esp_now_send(
    robotAddress,
    (uint8_t *)&command,
    sizeof(command)
  );
}

void setup() {
  Serial.begin(115200);

  pinMode(JOY_SW, INPUT_PULLUP);

  WiFi.mode(WIFI_STA);

  Serial.print("Controller MAC: ");
  Serial.println(WiFi.macAddress());

  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW init failed");
    return;
  }

  memset(&peerInfo, 0, sizeof(peerInfo));
  memcpy(peerInfo.peer_addr, robotAddress, 6);

  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add robot");
    return;
  }

  Serial.println("Controller Ready");
}

void loop() {

  int x = analogRead(JOY_X);
  int y = analogRead(JOY_Y);

  char command = 'S';

  // Dead zone around center
  int low = 1400;
  int high = 2700;

  if (y > high) {
    command = 'F';
  }
  else if (y < low) {
    command = 'B';
  }
  else if (x < low) {
    command = 'L';
  }
  else if (x > high) {
    command = 'R';
  }
  else {
    command = 'S';
  }

  sendCommand(command);

  Serial.print("X: ");
  Serial.print(x);

  Serial.print("  Y: ");
  Serial.print(y);

  Serial.print("  Command: ");
  Serial.println(command);

  delay(50);
}