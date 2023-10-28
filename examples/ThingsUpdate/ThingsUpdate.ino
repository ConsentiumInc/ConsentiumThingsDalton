/***************************************************
  This is Consentium's IoT library
  ----> https://consentiuminc.online/
  Check out the links above for our tutorials and product diagrams.

  This Consentium's IoT library works only for ESP8266/ESP32/Raspberry Pi Pico W compatible Edge boards. 
  
  Connect an LED to GPIO 16 for ESP 8266, GPIO 23 for ESP 32 and GPIO 25 for Raspberry Pi Pico W to indicate the REST events. 
 
  Written by Debjyoti Chowdhury for Consentium.
  MIT license, all text above must be included in any redistribution
 ****************************************************/


#include <ConsentiumThingsDalton.h>

ConsentiumThings board; // Create ConsentiumThings object

const char *ssid = "YOUR_WIFI_SSID"; // Add WiFi SSID
const char *pass = "YOUR_WIFI_PASSWORD"; // Add WiFi password
constexpr int interval = 7000; // Wait for 7 seconds
const char *ApiKey = "YOUR_API_KEY"; // Write API key
const char *BoardApiKey = "YOUR_BOARD_API_KEY"; // Board API key

void setup() {
  board.begin(); // Initialize IoT board
  board.initWiFi(ssid, pass); // Begin WiFi connection
}

void loop(){
  double data_0 = board.busRead(0);  // read voltage data
  
  double sensorValues[] = {data_0};  // sensor data array
  const char* sensorInfo[] = {"Temperature"}; // sensor info. array
  
  int sensorCount = sizeof(sensorValues)/sizeof(sensorValues[0]); // number of sensors connected 
  
  board.sendREST(ApiKey, BoardApiKey, sensorValues, sensorInfo, sensorCount, LOW_PRE); // send over REST with delay with desired prescision

  delay(interval);
}
