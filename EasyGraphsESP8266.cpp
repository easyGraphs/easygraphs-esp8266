/*
Copyright (c) 2019 EasyGraphs.io

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Created by Dmitrii Levin - Founder of EasyGraphs.io
*/

#include "EasyGraphsESP8266.h"
#include <string.h>

EasyGraphs::EasyGraphs(char* deviceToken, bool isDebug) {
  secretToken = deviceToken;
  deviceName = DEVICE_NAME;
  devicePlatform = DEVICE_PLATFORM;
  deviceType = DEVICE_TYPE;
  debug = isDebug;
  counter = 0;
  dataSets = (Collection *)malloc(50*sizeof(dataSets));
}

// Update device name
void EasyGraphs::setDeviceName(char *name) {
  deviceName = name;
}

// Update device platform
void EasyGraphs::setDevicePlatform(char *platform) {
  devicePlatform = platform;
}

// Update device type
void EasyGraphs::setDeviceType(char *type) {
  deviceType = type;
}

// Set device gps longitude
void EasyGraphs::setDeviceLongitude(float longitude) {
  deviceLongitude = longitude;
}

// Set device gps latitude
void EasyGraphs::setDeviceLatitude(float latitude) {
  deviceLatitude = latitude;
}

// Add parameter to collection
void EasyGraphs::addParameter(char *name, float value) {
    (dataSets+counter)->name = name;
    (dataSets+counter)->value = value;
    counter++;
}

// Init the WIFI connection
void EasyGraphs::initWIFI(char *ssid, char *password) {
  Serial.println("Trying reach the WIFI connection");
  WiFi.begin(ssid, password);
  WiFi.setAutoReconnect(true);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    yield();
  }
  Serial.println("Connected");
  Serial.println("IP address is: ");
  Serial.println(WiFi.localIP());
}

// Publish current collection state
bool EasyGraphs::publish() {

  if (counter == 0) {
    error.message = "No parameters in collection to send";
    return false;
  }

  // Create json body
  String json;
  for (uint8_t i = 0; i < counter; i++) {
    json += "\""+  String((dataSets + i)->name) + "\":\"" + String((dataSets + i)->value) + "\",";
  }
  json = "{" + json.substring(0, json.length()-1) + "}";

  // Init http request
  http.begin(String(API_SERVER)+":"+String(API_SERVER_PORT)+"/dataset/collections");

  // Add requred headers
  http.addHeader("Content-Type", "application/json");
  http.addHeader("SecretToken", secretToken);

  // Device information headers
  http.addHeader("Data-Device-Name", deviceName);
  http.addHeader("Data-Device-Type", deviceType);
  http.addHeader("Data-Device-Platform", devicePlatform);

  // Device gps position headers
  http.addHeader("Data-Device-Longitude", String(deviceLongitude));
  http.addHeader("Data-Device-Latitude", String(deviceLatitude));

  logDebug("request body: " + json);

  // Send the post data
  int httpCode = http.POST(json);
  String payload = http.getString();

  //Close http connection
  http.end();

  logDebug("response code: " + String(httpCode));
  logDebug("response payload: " + payload);

  if (httpCode != 200) {
    error.message = payload;
    error.code = httpCode;
    return false;
  }

  counter = 0;
  return true;
}

// Return error details
Error EasyGraphs::getError() {
  Error data = error;
  error = Error{};
  return data;
}

// Log debug messages
void EasyGraphs::logDebug(String text) {
  if (debug == true) {
    Serial.println(text);
  }
}
