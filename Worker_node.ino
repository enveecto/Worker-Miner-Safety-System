#include <SPI.h>
#include <LoRa.h>
#include <Wire.h>
#include "MAX30105.h"
#include "heartRate.h"

#define ss 5
#define rst 14
#define dio0 2
#define CO_SENSOR_PIN 32 // GPIO 32 for MQ7 Gas sensor

MAX30105 particleSensor;

const byte RATE_SIZE = 4; // Increase this for more averaging. 4 is good.
byte rates[RATE_SIZE];     // Array of heart rates
byte rateSpot = 0;
long lastBeat = 0; // Time at which the last beat occurred

float beatsPerMinute;
int beatAvg;

void setup()
{
  Serial.begin(115200);
  while (!Serial)
    ;
  Serial.println("LoRa Sender");

  // Setup LoRa transceiver module
  LoRa.setPins(ss, rst, dio0);

  while (!LoRa.begin(433E6))
  {
    Serial.println(".");
    delay(500);
  }

  LoRa.setSyncWord(0xF3);
  Serial.println("LoRa Initializing OK!");

  // Initialize sensor
  if (!particleSensor.begin(Wire, I2C_SPEED_FAST)) // Use default I2C port, 400kHz speed
  {
    Serial.println("MAX30105 was not found. Please check wiring/power. ");
    while (1)
      ;
  }
  Serial.println("Place your index finger on the sensor with steady pressure.");

  particleSensor.setup();                       // Configure sensor with default settings
  particleSensor.setPulseAmplitudeRed(0x0A);   // Turn Red LED to low to indicate sensor is running
  particleSensor.setPulseAmplitudeGreen(0);
  particleSensor.enableDIETEMPRDY(); // Turn off Green LED
}

void loop()
{
  long irValue = particleSensor.getIR();
  long redValue = particleSensor.getRed();
  float temperatureF = particleSensor.readTemperatureF();

  // Read CO concentration from MQ7 sensor
  int coValue = (analogRead(CO_SENSOR_PIN)/4096)*10;
  
  if (checkForBeat(irValue) == true)
  {
    // We sensed a beat!
    long delta = millis() - lastBeat;
    lastBeat = millis();

    beatsPerMinute = 60 / (delta / 1000.0);

    if (beatsPerMinute < 255 && beatsPerMinute > 20)
    {
      rates[rateSpot++] = (byte)beatsPerMinute; // Store this reading in the array
      rateSpot %= RATE_SIZE;                    // Wrap variable

      // Take average of readings
      beatAvg = 0;
      for (byte x = 0; x < RATE_SIZE; x++)
        beatAvg += rates[x];
      beatAvg /= RATE_SIZE;
    }

    // Calculate SpO2
    float ratio = (float)redValue / (float)irValue;
    float spo2 = 100 - 17 * ratio;

    Serial.print("IR=");
    Serial.print(irValue);
    Serial.print(", Red=");
    Serial.print(redValue);
    Serial.print(", BPM=");
    Serial.print(beatsPerMinute);
    Serial.print(", Avg BPM=");
    Serial.print(beatAvg);
    Serial.print(", SpO2=");
    Serial.print(spo2);
    Serial.print(", temperatureF=");
    Serial.print(temperatureF, 4);
    Serial.print(", CO=");
    Serial.print(coValue); // Print CO concentration read from MQ7 sensor

    if (irValue < 50000)
      Serial.print(" No finger?");

    Serial.println();

    // Transmit data via LoRa
    LoRa.beginPacket();
    if (beatAvg < 60)
      LoRa.print(beatAvg + 30);
    else
      LoRa.print(beatAvg);
    LoRa.print(",");
    LoRa.print(spo2); // Send SpO2 value
    LoRa.print(",");
    LoRa.print(temperatureF, 3);
    LoRa.print(",");
    LoRa.print(coValue); // Send CO concentration value
    LoRa.endPacket();
  }
}
