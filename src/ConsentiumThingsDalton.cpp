#include <ArduinoJson.h>

#include "ConsentiumThingsDalton.h"
#include "consentium_essentials.h"

WiFiClientSecure client;
HTTPClient http;

const int kselect_lines[4] = {S_0, S_1, S_2, S_3}; // MUX select lines

const int MAX_JSON_SIZE = 200;
const int MAX_JSON_SENSOR_DATA_SIZE = 50;

const int kMUXtable[16][4] = {
  {0, 0, 0, 0}, {1, 0, 0, 0}, {0, 1, 0, 0}, {1, 1, 0, 0},
  {0, 0, 1, 0}, {1, 0, 1, 0}, {0, 1, 1, 0}, {1, 1, 1, 0},
  {0, 0, 0, 1}, {1, 0, 0, 1}, {0, 1, 0, 1}, {1, 1, 0, 1},
  {0, 0, 1, 1}, {1, 0, 1, 1}, {0, 1, 1, 1}, {1, 1, 1, 1}
};

ConsentiumThings::ConsentiumThings() {}

void ConsentiumThings::begin() {
  Serial.begin(EspBaud);
  #ifdef ESP32
    client.setCACert(consentium_root_ca);
  #elif ESP8266
    client.setInsecure();
    client.setCACert((const uint8_t*)consentium_root_ca, sizeof(consentium_root_ca) - 1);
  #endif
  for (int i = 0; i < 4; i++) {
    pinMode(kselect_lines[i], OUTPUT);
  }
}

void ConsentiumThings::initWiFi(const char* ssid, const char* password) {
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Got IP : ");
  Serial.println(WiFi.localIP());
}

float ConsentiumThings::busRead(int j, float threshold) {
  for (int i = 0; i < 4; i++) {
    digitalWrite(kselect_lines[i], kMUXtable[j][i]);
  }
  return analogRead(A0) * threshold;
}

void ConsentiumThings::sendREST(const char* key, const char* board_id, float sensor_data[], String sensor_info[], int sensor_num, int precision) {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi not connected. Cannot send REST request.");
    return;
  }

  // create the server URL
  String serverUrl = String(server_url);
  serverUrl.reserve(100);
  serverUrl.concat("key=");
  serverUrl.concat(String(key));
  serverUrl.concat("&boardkey=");
  serverUrl.concat(String(board_id));

  DynamicJsonDocument jsonDocument(MAX_JSON_SIZE + sensor_num * MAX_JSON_SENSOR_DATA_SIZE);

  // Create a JSON array for sensor data
  JsonArray sensorDataArray = jsonDocument["sensors"]["sensorData"].to<JsonArray>();

  // Add sensor data dynamically
  for (int i = 0; i < sensor_num; i++) {
    // Create a JSON object for the current sensor data
    JsonObject sensorData = sensorDataArray.createNestedObject();
    sensorData["info"] = sensor_info[i];
    sensorData["data"] = String(sensor_data[i], precision);
  }

  // Serialize the JSON document to a string
  String jsonString;
  serializeJson(jsonDocument, jsonString);

  http.begin(client, serverUrl);

  // Set the content type header to application/json
  http.addHeader("Content-Type", "application/json");

  // Make the POST request
  int httpCode = http.sendRequest("POST", jsonString);

  // Check for errors
  if (httpCode > 0) {
    if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_CREATED) {
      String response = http.getString();
      String combinedOutput = "Server response: " + response + "\nRespose code: " + String(httpCode) + "\nData packet: " + jsonString;
      Serial.println(combinedOutput);
    }
  } else {
    Serial.println("HTTP POST request failed.");
  }
  // Close the HTTP connection
  http.end();
}
