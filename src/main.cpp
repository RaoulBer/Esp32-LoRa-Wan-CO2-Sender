#include <Arduino.h>

//Imports for the LoRa Wan Adapter
#include <SPI.h>
#include <LoRa.h>

// LORA WAN MODULE
#define ss 5
#define rst 14
#define dio0 2

//CO2 Module MH-Z19B
#include <Wire.h>
#include <MHZ19.h>
#define RX_PIN 16
#define TX_PIN 17
#define MHZ19_BAUDRATE 9600
#define MHZ19_PROTOCOL SERIAL_8N1
#define MHZ19_PWM_PIN 21
MHZ19 mhz19b;
HardwareSerial mySerial(1);
int co2Value = 0; 
int temp_mh;    //Temperatur des MH-Z19B

void setup() {
  Serial.begin(115200);
  while (!Serial);
  //Initialize LoRa WAN Receiver
  Serial.println("LoRa Sender");
  LoRa.setPins(ss, rst, dio0);

  if (!LoRa.begin(915E6)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }

  char mhz19_version[4]; 
  mySerial.begin(MHZ19_BAUDRATE, MHZ19_PROTOCOL, RX_PIN, TX_PIN);
  delay(500);
  mhz19b.begin(mySerial); 
  mhz19b.getVersion(mhz19_version);
  mhz19b.autoCalibration(true);
}

void loop() {
  temp_mh = mhz19b.getTemperature();
  co2Value = mhz19b.getCO2();
  Serial.println(co2Value);
  Serial.print("Temperatures:");
  Serial.println(temp_mh);
  
  // send packet
  LoRa.beginPacket();
    LoRa.print("CO2-1;");
    LoRa.print("Temp:");
    LoRa.print(temp_mh);
    LoRa.print(";");
    LoRa.print("ppm:");
    LoRa.print(co2Value);
  LoRa.endPacket();
  delay(2000);

}