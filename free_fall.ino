#include <ESP8266WiFi.h>
#include <espnow.h>

#define BUZZER_PIN 0 // GPIO pin connected to the buzzer

uint8_t mac[] = {0x24, 0x6F, 0x28, 0xAE, 0x59, 0xA8}; // MAC address of ESP32
bool isFallDetected = false;

void setup() {
  Serial.begin(115200);
  pinMode(BUZZER_PIN, OUTPUT);

  // Initialize ESP-NOW
  if (esp_now_init() != 0) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Register callback function to handle received data
  esp_now_set_self_role(ESP_NOW_ROLE_CONTROLLER);
  esp_now_register_recv_cb(onDataReceived);
}

void loop() {
  // Check if fall is detected, then send signal
  if (isFallDetected) {
    sendData();
    isFallDetected = false; // Reset flag
  }
}

void onDataReceived(uint8_t *macAddr, uint8_t *data, uint8_t len) {
  // Handle received data (if needed)
}

void sendData() {
  uint8_t data = 1; // Data to send (can be any value)
  esp_now_send(mac, &data, sizeof(data));
}
