#include <Arduino.h>

//Imports for the LoRa Wan Adapter
#include <SPI.h>
#include <LoRa.h>

// LORA WAN MODULE
#define ss 5
#define rst 14
#define dio0 2

//Importing networking libraries
#include <WiFi.h>
#include "WiFiSTA.h"

//Import http async webserver and filesystem emulation package
#include "ESPAsyncWebServer.h"
#include <SPIFFS.h>

//String to int conversion 
#include <sstream>

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

// VARIABLES OF THE HTTP WEB SERVER
AsyncWebServer server(80);

// VARIABLES OF THE HTTP WEB wifiClient
WiFiClient wifiClient;

// Initialize SPIFFS
void initSPIFFS() {
  if (!SPIFFS.begin(true)) {
    Serial.println("An error has occurred while mounting SPIFFS");
  }
  Serial.println("SPIFFS mounted successfully");
}

// Read File from SPIFFS
String readFile(fs::FS &fs, const char * path){
  Serial.printf("Reading file: %s\r\n" , path);

  File file = fs.open(path, "r");
  if(!file || file.isDirectory()){
    Serial.println("- failed to open file for reading");
    return String();
  }
  
  String fileContent;
  while(file.available()){
    fileContent = file.readStringUntil('\n');
    break;     
  }
  file.close();
  return fileContent;
}

// Write file to SPIFFS
void writeFile(fs::FS &fs, const char * path, const char * message){
  Serial.printf("Writing file: %s\r\n", path);

  File file = fs.open(path, "w");
  if(!file){
    Serial.println("- failed to open file for writing");
    return;
  }
  if(file.print(message)){
    Serial.println("- file written");
  } else {
    Serial.println("- write failed");
  }
  file.close();
}

const char* syncwordPath = "/syncword.txt";
const char* wifissidPath = "/wifissid.txt";
const char* wifipwdPath ="/wifipwd.txt";

String syncword;
String wifissid;
String wifipwd ;

String processor(const String& var){
  //Serial.println(var);
  if(var == "specialphrase"){
    return readFile(SPIFFS, syncwordPath);
  }
  
  else if(var == "wifissid"){
    return readFile(SPIFFS, wifissidPath);
  }
  return String();
}

void setup() {
  Serial.begin(115200);
  while (!Serial);
  

  initSPIFFS();
  delay(1);

  syncword = readFile(SPIFFS, syncwordPath);
  wifissid = readFile(SPIFFS, wifissidPath);
  wifipwd = readFile(SPIFFS, wifipwdPath);

  Serial.println(wifissid);
  Serial.println(wifipwd);

  //Initialize LoRa WAN Receiver
  Serial.println("LoRa Sender");
  LoRa.setPins(ss, rst, dio0);

  std::istringstream is(syncword.c_str());
  int syncInt;
  if (is >> syncInt){
    LoRa.setSyncWord(syncInt);
  }

  if (!LoRa.begin(866E6)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }

  WiFi.mode(WIFI_STA);
  WiFi.begin(wifissid.c_str(), wifipwd.c_str());
  int n = 0;
  while ((WiFi.status() != WL_CONNECTED) && (n <= 10)){
    Serial.println("Connecting to WiFi..");
    delay(1000);
    n++;
  }

  // Print ESP Local IP Address
  Serial.println(WiFi.localIP());
  server.begin();

  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/index.html", String(), false, processor);
  });

  // Route to load style.css file
  server.on("/index.css", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/index.css", "text/css");
  });

  server.on("/", HTTP_POST, [](AsyncWebServerRequest *request) {
  int params = request->params();
  for(int i=0;i<params;i++){
    AsyncWebParameter* p = request->getParam(i);
    if(p->isPost()){
      // HTTP POST Special Phase
      if (p->name() == "specialphrase") {
        syncword= p->value().c_str();
        Serial.print("Lora Wan special phrase: ");
        Serial.println(syncword);
        // Write file to save value
        if(syncword.c_str() != ""){
          writeFile(SPIFFS, syncwordPath, syncword.c_str());  
        }
      }
      // HTTP POST ip value
      if (p->name() == "wifissid") {
        wifissid = p->value().c_str();
        Serial.print("Wifi SSID set to: ");
        Serial.println(wifissid);
        // Write file to save value
        if(wifissid.c_str() != ""){
          writeFile(SPIFFS, wifissidPath, wifissid.c_str());
        }
      }
     // HTTP POST ip value
      if (p->name() == "wifipassword") {
        wifipwd = p->value().c_str();
        Serial.print("Wifi password set to: ");
        Serial.println(wifipwd);
        // Write file to save value
        if(wifipwd.c_str() != ""){
          writeFile(SPIFFS, wifipwdPath, wifipwd.c_str());
        }
      }
      if (p->name() == "reboot") {
        Serial.println("Rebooting");
        // Write file to save value
        ESP.restart();
      }
      request->send(SPIFFS, "/index.html", String(), false, processor);
    }
  }
  });

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
    LoRa.print("{sensor:CO2-1,");
    LoRa.print("readings:{");
    LoRa.print("temperature:");
    LoRa.print(temp_mh);
    LoRa.print(",");
    LoRa.print("co2concentration:");
    LoRa.print(co2Value);
    LoRa.print("}}");
  LoRa.endPacket();
  delay(2000);
}