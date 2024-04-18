#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_ADXL345_U.h>
#include <SPI.h>
#include <RF24.h>

#define BUZZER_PIN 0 // GPIO pin connected to the buzzer
#define THRESHOLD_FREE_FALL 2.5 // Threshold value for free fall detection (adjust as needed)
#define FREE_FALL_DURATION_THRESHOLD 50 // Minimum duration of free fall in milliseconds (0.5 seconds)

Adafruit_ADXL345_Unified accel = Adafruit_ADXL345_Unified(12345);
RF24 radio(9, 10); // CE, CSN pins

unsigned long freeFallStartTime = 0; // Variable to store the start time of free fall

bool fallDetected = false; // Flag to indicate if fall is detected

void setup() {
  Serial.begin(115200);
  pinMode(BUZZER_PIN, OUTPUT);
  if (!accel.begin()) {
    Serial.println("Ooops, no ADXL345 detected ... Check your wiring!");
    while (1);
  }
  
  radio.begin();
  radio.setPALevel(RF24_PA_LOW); // Set power level to low
  radio.openWritingPipe(0xF0F0F0F0E1LL); // Set the address for the writing pipe
}

void loop() {
  // Check for fall condition
  float acceleration = getAcceleration();
  if (acceleration < THRESHOLD_FREE_FALL) {
    unsigned long freeFallDuration = millis() - freeFallStartTime;
    if (freeFallDuration >= FREE_FALL_DURATION_THRESHOLD) {
      fallDetected = true; // Set fall detection flag
    }
  } else {
    freeFallStartTime = millis(); // Start or update the free fall timer
  }

  // Check if fall is detected and activate buzzer
  if (fallDetected) {
    activateBuzzer();
    sendSignal(); // Send signal using NRF24L01
  }

  delay(100); // Small delay to reduce CPU usage
}

float getAcceleration() {
  sensors_event_t event;
  accel.getEvent(&event);
  return sqrt(pow(event.acceleration.x, 2) + pow(event.acceleration.y, 2) + pow(event.acceleration.z, 2));
}

void activateBuzzer() {
  tone(BUZZER_PIN, 1000); // Start the tone at 1000 Hz
}

void sendSignal() {
  const char* message = "Fall detected!";
  radio.write(&message, sizeof(message)); // Send the message
}
