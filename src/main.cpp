#include <Arduino.h>
#include <NeoPixelBus.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <string.h>
#include <ArduinoJson.h> 
#include <ElegantOTA.h>
#include <ESPAsyncWebServer.h>

#include "LEDGroupManager.h"
#include "Credentials.h"


//*************App Config*****************************************************
#define NUM_LED 104
#define NUM_HEXES 13
#define DATA_PIN D6


//LED indexes for each Hex group, starting from bottom left corner, clockwise
const uint8_t ledGroup1Indexes[] = {96,97,98,99,100,101,102,103};
const uint8_t ledGroup2Indexes[] = {48,55,54,53,52,51,50,49};
const uint8_t ledGroup3Indexes[] = {37,38,39,32,33,34,35,36};
const uint8_t ledGroup4Indexes[] = {89,90,91,92,93,94,95,88};
const uint8_t ledGroup5Indexes[] = {72,79,78,77,76,75,74,73};
const uint8_t ledGroup6Indexes[] = {45,46,47,40,41,42,43,44};
const uint8_t ledGroup7Indexes[] = {26,25,24,31,30,29,28,27};
const uint8_t ledGroup8Indexes[] = {85,86,87,80,81,82,83,84};
const uint8_t ledGroup9Indexes[] = {58,57,56,63,62,61,60,59};
const uint8_t ledGroup10Indexes[] = {18,17,16,23,22,21,20,19};
const uint8_t ledGroup11Indexes[] = {68,67,66,65,64,71,70,69};
const uint8_t ledGroup12Indexes[] = {10,9,8,15,14,13,12,11};
const uint8_t ledGroup13Indexes[] = {3,4,5,6,7,0,1,2};

//     3   4
//      ----
// 2  /      \ 5
// 1  \      / 6
//      ----
//     0   7
int colorSaturation = 255;

/// @brief Define colors for Neo Pixel Bus
const RgbwColor red(colorSaturation, 0, 0,0);
const RgbwColor green(0, colorSaturation, 0,0);
const RgbwColor blue(0, 0, colorSaturation,0);
const RgbwColor white(0,0,0,colorSaturation);
const RgbwColor black(0);
const RgbwColor yellow(colorSaturation/2,(colorSaturation/2)-25,0,0);

RgbwColor FAA_MVFR(25,50,255,0);
RgbwColor FAA_VFR(25,255,12,0);
RgbwColor FAA_IFR(255,7,3,0);
RgbwColor FAA_LIFR(255,20,255,0);

//This is so messy. 
//TODO: Fix this so it's not so messy
const char* STATION_ID_KEQY = "KEQY"; //Monroe
const char* STATION_ID_KRUQ = "KRUQ"; //Mid Carolina
const char* STATION_ID_KUZA = "KUZA"; //Rock Hill
const char* STATION_ID_KCLT = "KCLT"; //Charlotte
const char* STATION_ID_KESN = "KESN"; //Easton
const char* STATION_ID_KLEE = "KLEE"; //Leesburg
const char* STATION_ID_KRDU = "KRDU"; //Raleigh
const char* STATION_ID_KAVL = "KAVL"; //Asheville
const char* STATION_ID_KHXD = "KHXD"; //Hilton Head
const char* STATION_ID_KFFA = "KFFA"; //First Flight
const char* STATION_ID_KIPJ = "KIPJ"; //Lincolnton
const char* STATION_ID_KHKY = "KHKY"; //Hickory
const char* STATION_ID_KMTV = "KMTV"; //Blue Ridge
//****************End App Config***********************************************

//We use these objects to manage wifi / make requests / run a webserver
WiFiClient client;
HTTPClient http;
AsyncWebServer server(80);

// NeoPixelBus<NeoGrbwFeature, NeoEsp8266BitBangSk6812Method> strip (NUM_LED, DATA_PIN);
//Alternative DMA mode, uses GPIO3, and interferes with serial, but is supposed to be a lot faster
//NeoPixelBus<NeoGrbwFeature, NeoEsp8266Dma800KbpsMethod> strip (NUM_LED);

//UART method, UART0 uses TXD0/GPIO1 pin. UART1 uses TXD1/GPIO2. D4 on NodeMCU
NeoPixelBus<NeoGrbwFeature, NeoEsp8266Uart1800KbpsMethod> strip (NUM_LED);

LEDGroupController led_controllers[] = {
  LEDGroupController("Group1", ledGroup1Indexes, 8, strip),
  LEDGroupController("Group2", ledGroup2Indexes, 8, strip),
  LEDGroupController("Group3", ledGroup3Indexes, 8, strip),
  LEDGroupController("Group4", ledGroup4Indexes, 8, strip),
  LEDGroupController("Group5", ledGroup5Indexes, 8, strip),
  LEDGroupController("Group6", ledGroup6Indexes, 8, strip),
  LEDGroupController("Group7", ledGroup7Indexes, 8, strip),
  LEDGroupController("Group8", ledGroup8Indexes, 8, strip),
  LEDGroupController("Group9", ledGroup9Indexes, 8, strip),
  LEDGroupController("Group10", ledGroup10Indexes, 8, strip),
  LEDGroupController("Group11", ledGroup11Indexes, 8, strip),
  LEDGroupController("Group12", ledGroup12Indexes, 8, strip),
  LEDGroupController("Group13", ledGroup13Indexes, 8, strip)
};

LEDGroupManager group_manager(led_controllers, 13);


String getSerialString(){
  while (Serial.available() == 0) {}
  String optionParam = Serial.readString();
  Serial.println(optionParam);
  optionParam.trim();
  return optionParam;
}

void initToggle(){
  strip.SetPixelColor(0, red);
  strip.Show();

  delay(500);
  
  strip.SetPixelColor(0, black);
  strip.Show();
}

String httpGETRequest(const char* serverName) {

  // Your IP address with path or Domain name with URL path 
  http.begin(client, serverName);

  // If you need Node-RED/server authentication, insert user and password below
  http.setAuthorization(avwx_auth_header);

  // Send HTTP POST request
  int httpResponseCode = http.GET();

  String payload = "{}"; 

  if (httpResponseCode == 200) {
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
    payload = http.getString();
  }
  else {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
  }
  // Free resources
  http.end();

  return payload;
}

const char* getFlightRules(const char* station_id){
  if(WiFi.status()== WL_CONNECTED){
    char base_url[50] = "http://avwx.rest/api/metar/";
    char* URL = strcat(base_url, station_id); //Add station ID to URL
    URL = strcat(URL, "?filter=flight_rules&onfail=error"); //Add filter for only flight_rules
    String jsonResponse = httpGETRequest(URL);
    Serial.print(jsonResponse);

    DynamicJsonDocument doc(256);
    DeserializationError error = deserializeJson(doc, jsonResponse);
    
    if (error) {
      Serial.print(F("deserializeJson() failed: "));
      Serial.println(error.f_str());
      return "ERROR";
    }

    const char* flight_rules = doc["flight_rules"];
    Serial.print("Station ID: ");
    Serial.println(station_id);

    Serial.print("Flight Rules:");
    Serial.println(flight_rules);
    Serial.println("");

    return flight_rules;

  }else{
    return "ERROR";
  }
}

void connectWifi(){
  WiFi.begin(ssid, password);
  WiFi.setAutoReconnect(true);

  Serial.println();
  Serial.print("Connecting");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  

  Serial.println("success!");
  Serial.print("IP Address is: ");
  Serial.println(WiFi.localIP());
}

void setup() {
  Serial.begin(115200);
  // while (!Serial); // wait for serial attach

  Serial.println();
  Serial.println("Initializing...");
  Serial.flush();

  strip.Begin();

  //show init toggle
  initToggle();

  //Connect to wifi
  connectWifi();

  ElegantOTA.begin(&server);    // Start ElegantOTA
  server.begin();
  Serial.println("HTTP server started");

  Serial.println();
  Serial.println("Running...");


}

void setHexFlightCategory(const char* flight_category, uint8_t group_index, Animations transition){
  if (flight_category != NULL){
    //Really don't want to have constants comparing these, but couldn't find a way around it. #C++noob
    if (strcmp(flight_category, "VFR") == 0){
      group_manager.setGroupColor(group_index, FAA_VFR, transition);
    }else if (strcmp(flight_category, "MVFR") == 0){
      group_manager.setGroupColor(group_index, FAA_MVFR, transition);
    }else if (strcmp(flight_category, "IFR") == 0){
      group_manager.setGroupColor(group_index, FAA_IFR, transition);
    }else if (strcmp(flight_category, "LIFR") == 0){
      group_manager.setGroupColor(group_index, FAA_LIFR, transition);
    }
  }else{
    group_manager.setGroupColor(group_index, white, transition);
  }
}

void R_W_B_scroll(){
  for (int i=0; i<NUM_LED; i++){
    strip.SetPixelColor(i, red);
    strip.Show();
    delay(20);

    // i++;
    // strip.SetPixelColor(i, white);
    // strip.Show();
    // delay(20);

    // i++;
    // strip.SetPixelColor(i, blue);
    // strip.Show();
    // delay(20);

    // i++;
    // strip.SetPixelColor(i, white);
    // strip.Show();
    // delay(20);
  }

  for (int i=0; i<NUM_LED; i++){
    strip.SetPixelColor(i, black);
    strip.Show();
    delay(20);
  }

  for (int i=0; i<NUM_LED; i++){
    strip.SetPixelColor(i, white);
    strip.Show();
    delay(20);
  }

  for (int i=0; i<NUM_LED; i++){
    strip.SetPixelColor(i, black);
    strip.Show();
    delay(20);
  }

  for (int i=0; i<NUM_LED; i++){
    strip.SetPixelColor(i, blue);
    strip.Show();
    delay(20);
  }

  for (int i=0; i<NUM_LED; i++){
    strip.SetPixelColor(i, black);
    strip.Show();
    delay(20);
  }

}

void Toggle_LEDs_RED(){
  for (uint8_t i=0; i<NUM_HEXES;i++){
    group_manager.setGroupColor(i, red, FADE);
  }

  while(!group_manager.transitionComplete()){
    group_manager.loop(millis());
  }

  for (uint8_t i=0; i<NUM_HEXES;i++){
    group_manager.setGroupColor(i, black, FADE);
  }
}

void setLEDFlightRules(){
  setHexFlightCategory(getFlightRules(STATION_ID_KEQY), 0, FADE);
  setHexFlightCategory(getFlightRules(STATION_ID_KRUQ), 1, FADE);
  setHexFlightCategory(getFlightRules(STATION_ID_KESN), 2, FADE);
  setHexFlightCategory(getFlightRules(STATION_ID_KUZA), 3, FADE);
  setHexFlightCategory(getFlightRules(STATION_ID_KCLT), 4, FADE);
  setHexFlightCategory(getFlightRules(STATION_ID_KLEE), 5, FADE);
  setHexFlightCategory(getFlightRules(STATION_ID_KRDU), 6, FADE);
  setHexFlightCategory(getFlightRules(STATION_ID_KAVL), 7, FADE);
  setHexFlightCategory(getFlightRules(STATION_ID_KHXD), 8, FADE);
  setHexFlightCategory(getFlightRules(STATION_ID_KFFA), 9, FADE);
  setHexFlightCategory(getFlightRules(STATION_ID_KIPJ), 10, FADE);
  setHexFlightCategory(getFlightRules(STATION_ID_KHKY), 11, FADE);
  setHexFlightCategory(getFlightRules(STATION_ID_KMTV), 12, FADE);
}

void setLedRed(int led_index){
  String str_led_index = String(led_index);
  Serial.print("Led Index: ");
  Serial.println(str_led_index);
  strip.SetPixelColor(led_index, red);
  strip.Show();
}

void UpdateWifi(){
  switch(WiFi.status()){
    case WL_CONNECTION_LOST:
      WiFi.reconnect();
      break;
    default:
      break;
  }
}

bool actionsShown = false;
void DoSerialAction(){

  if (actionsShown == false){
  //Print actions
    Serial.print("Wifi IsConnected: ");
    Serial.println(WiFi.isConnected());
    Serial.print("Wifi Status: ");
    Serial.println(WiFi.status());
    Serial.println();

    Serial.println("Select an Action:");
    Serial.println("1. Toggle LED Red");
    Serial.println("2. Flag Show");
    Serial.println("3. Light Position");
    Serial.println("4. Toggle LED Red By Hex");
    Serial.println("5. Light Hex By Index");
    Serial.println("6. Show FAA Colors");
    Serial.println("7. Get Flight Rules");
    Serial.println("8. Reconnect Wifi");
    Serial.println();
    Serial.print("Option: ");
    actionsShown = true;
  }

  if (Serial.available() > 0){
    char iByte = Serial.read();
    Serial.println(iByte);

    switch(iByte){
      case '1': 
        Toggle_LEDs_RED();
        break;
      case '2':
        R_W_B_scroll();
        break;
      case '3':
        setLedRed(getSerialString().toInt());
        break;
      case '4':
        // ScrollByHex();
        break;
      case '5':
        group_manager.setGroupColor(getSerialString().toInt(), FAA_LIFR, DIRECT);
        break;
      case '6':
        // setHexFlightCategory("VFR", 3, DIRECT);
        // setHexFlightCategory("MVFR", 4, DIRECT);
        // setHexFlightCategory("IFR", 5, DIRECT);
        // setHexFlightCategory("LIFR", 6, DIRECT);
        group_manager.setGroupColor(3, FAA_VFR, FADE);
        group_manager.setGroupColor(4, FAA_MVFR, FADE);
        group_manager.setGroupColor(5, FAA_IFR, FADE);
        group_manager.setGroupColor(6, FAA_LIFR, FADE);
        break;
      case '7':
        setLEDFlightRules();
        break;
      case '8':
        WiFi.reconnect();
        break;

    }
    actionsShown = false;
  }
}


//TODO: Need to find or implement timer class
unsigned long p_time1 = 0;
unsigned long p_time2 = 0;
void loop() {
  group_manager.loop(millis());
  ElegantOTA.loop();

  //30 minute timer
  if (p_time1 == 0 || millis() - p_time1 > (1000*60*30)){

    setLEDFlightRules();

    p_time1 = millis();
  }

  //1 second timer
  if (p_time2 == 0 || millis() - p_time2 > (1000)){
  
    if (group_manager.transitionComplete()){
      group_manager.startBreathe();
    }

    UpdateWifi();
    DoSerialAction();

    p_time2 = millis();
  }

}
