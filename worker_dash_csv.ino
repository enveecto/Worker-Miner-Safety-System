#include <SPI.h>
#include <LoRa.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "SPIFFS.h" // Include the SPIFFS library

#define ss 5
#define rst 14
#define dio0 2

const char *ssid = "Worker Dashboard";
const char *password = "12345678";

AsyncWebServer server(80);

int heartRateData;
float spo2Data;
float temperatureData;
int coData; // CO value received from LoRa

void setup() {
  Serial.begin(115200);
  pinMode(15, OUTPUT);
  while (!Serial);
  Serial.println("LoRa Receiver");

  LoRa.setPins(ss, rst, dio0);

  while (!LoRa.begin(433E6)) {
    Serial.println(".");
    delay(500);
  }
  digitalWrite(15, HIGH);

  LoRa.setSyncWord(0xF3);
  Serial.println("LoRa Initializing OK!");

  WiFi.softAP(ssid, password);
  IPAddress ip = WiFi.softAPIP();
  Serial.println("AP IP address: " + ip.toString());

  // Initialize SPIFFS
  if (!SPIFFS.begin(true)) {
    Serial.println("SPIFFS initialization failed");
    return;
  }

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    String html = "<html><head>";
    html += "<style>body{font-family: Arial, sans-serif; text-align: center; margin: 50px; background-color: #000; color: #fff;}"
            "h1{font-size: 36px; margin-bottom: 20px;}"
            "p{font-size: 24px;}"
            "#data-box{border: 2px solid #fff; padding: 20px; margin: 20px; text-align: center; border-radius: 10px;}"
            "#gas-status{font-size: 24px;}";
    html += "</style>";
    html += "<meta http-equiv='refresh' content='1'></head><body>";
    html += "<h1>Worker Safety Monitoring System</h1>";
    html += "<div id='data-box'>";
    html += "<p><strong>Heartrate:</strong> " + String(heartRateData) + "</p>";
    html += "<p><strong>SpO2:</strong> " + String(spo2Data) + "</p>";
    html += "<p><strong>Temperature:</strong> " + String(temperatureData) + " °F</p>";
    html += "</div>";
    html += "<p><a href='/download'>Download Data</a></p>"; // Add a link to download the CSV file
    html += "</body></html>";
    request->send(200, "text/html", html);
  });

  server.on("/download", HTTP_GET, [](AsyncWebServerRequest *request) {
    // Open the data.csv file
    File dataFile = SPIFFS.open("/data.csv", FILE_READ);
    if (!dataFile) {
      request->send(404, "text/plain", "File not found");
      return;
    }

    // Set the content type as CSV
    request->send(dataFile, "/data.csv", "text/csv");
  });

  server.begin();
}

void handleReceivedData(String receivedData) {
  char dataBuffer[50];
  receivedData.toCharArray(dataBuffer, 50);

  char *token = strtok(dataBuffer, ",");
  heartRateData = atoi(token);
  if (heartRateData < 50)
    heartRateData = 0;

  token = strtok(NULL, ",");
  spo2Data = atof(token);
  if (spo2Data < 95)
    spo2Data = 0;

  token = strtok(NULL, ",");
  temperatureData = atof(token);
  if (temperatureData >= 90 && temperatureData < 93) {
    temperatureData = temperatureData + 4;
  }
  if (temperatureData > 85 && temperatureData < 90) {
    temperatureData = temperatureData + 8;
  }
  if (temperatureData < 85)
    temperatureData = 0;

  token = strtok(NULL, ",");
  coData = atoi(token); // Parse the CO value from the received data

  // Save data to data.csv file on SPIFFS
  File dataFile = SPIFFS.open("/data.csv", FILE_APPEND);
  if (!dataFile) {
    Serial.println("Failed to open file for appending");
    return;
  }

  // Append data to the CSV file
  dataFile.print(heartRateData);
  dataFile.print(",");
  dataFile.print(spo2Data);
  dataFile.print(",");
  dataFile.print(temperatureData);
  dataFile.println();
  dataFile.close();
}

void loop() {
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    Serial.print("Received packet '");

    String receivedData = "";
    while (LoRa.available()) {
      receivedData += LoRa.readString();
    }
    Serial.print(receivedData);
    handleReceivedData(receivedData);
    digitalWrite(2, HIGH);
    delay(500);
    digitalWrite(2, LOW);

    Serial.print("' with RSSI ");
    Serial.println(LoRa.packetRssi());
  }
}
