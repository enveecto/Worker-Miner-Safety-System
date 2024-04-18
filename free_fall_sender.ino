#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_ADXL345_U.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>

#define BUZZER_PIN 0 // GPIO pin connected to the buzzer
#define THRESHOLD_FREE_FALL 2.5 // Threshold value for free fall detection (adjust as needed)
#define FREE_FALL_DURATION_THRESHOLD 50 // Minimum duration of free fall in milliseconds (0.5 seconds)

Adafruit_ADXL345_Unified accel = Adafruit_ADXL345_Unified(12345);

unsigned long freeFallStartTime = 0; // Variable to store the start time of free fall

const char* ssid = "HP";
const char* password = "12345678";

AsyncWebServer server(80);

bool fallDetected = false; // Flag to indicate if fall is detected

void setup() {
  Serial.begin(115200);
  pinMode(BUZZER_PIN, OUTPUT);
  if (!accel.begin()) {
    Serial.println("Ooops, no ADXL345 detected ... Check your wiring!");
    while (1);
  }

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("WiFi connected.");

server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    String html = "<html><head>";
    html += "<style>body{font-family: Arial, sans-serif; text-align: center; margin: 50px; background-color: #000; color: #fff;}"
            "h1{font-size: 36px; margin-bottom: 20px;}"
            "p{font-size: 24px;}"
            "#data-box{border: 2px solid #fff; padding: 20px; margin: 20px; text-align: center; border-radius: 10px;}"
            "#status{font-size: 30px; margin-top: 50px;}"
            ".button {background-color: #4CAF50; /* Green */border: none;color: white;padding: 15px 32px;text-align: center;text-decoration: none;display: inline-block;font-size: 16px;margin: 4px 2px;cursor: pointer;}"
            ".button2 {background-color: #008CBA;} /* Blue */";
    html += "</style>";
    html += "<meta http-equiv='refresh' content='1'></head><body>";
    html += "<h1>Worker Safety Monitoring System</h1>";
    html += "<div id='data-box'>";
    html += "<p><strong>Status:</strong> <span id='status'>";

    // Display emergency message if fall is detected
    if (fallDetected) {
      html += "EMERGENCY - FALL DETECTED";
      activateBuzzer();
      html += "<br><br><a class='button' href='https://www.tnfrs.tn.gov.in/contact/'>Call Fire Department</a>";
      html += "<br><br><a class='button button2' href='https://www.cmcvellorechittoorcampus.ac.in/emergency-care/'>Call Medical Services</a>";
    } else {
      html += "Worker is Safe";
    }

    html += "</span></p>";
    html += "</div>";
    html += "</body></html>";
    request->send(200, "text/html", html);
  });



  server.begin();
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
