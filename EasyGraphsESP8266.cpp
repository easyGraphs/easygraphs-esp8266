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

struct {
  uint32_t crc32;   // 4 bytes
  uint8_t channel;  // 1 byte,   5 in total
  uint8_t ap_mac[6]; // 6 bytes, 11 in total
  uint8_t padding;  // 1 byte,  12 in total
} rtcData;

EasyGraphs::EasyGraphs(char* deviceToken, bool isDebug) {
  secretToken = deviceToken;
  deviceName = DEVICE_NAME;
  devicePlatform = DEVICE_PLATFORM;
  deviceType = DEVICE_TYPE;
  debug = isDebug;
  counter = 0;
  dataSets = (Collection *)malloc(50*sizeof(dataSets));
}

// Update device token
void EasyGraphs::setDeviceToken(char *deviceToken) {
  secretToken = deviceToken;
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

  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
  WiFi.forceSleepBegin();

  // Try to read WiFi settings from RTC memory
  bool rtcValid = false;
  if(ESP.rtcUserMemoryRead(0, (uint32_t*)&rtcData, sizeof(rtcData))) {
    // Calculate the CRC of what we just read from RTC memory, but skip the first 4 bytes as that's the checksum itself.
    uint32_t crc = calculateCRC32(((uint8_t*)&rtcData) + 4, sizeof(rtcData) - 4);
    if(crc == rtcData.crc32) {
      rtcValid = true;
    }
  }

  if(rtcValid) {
    // The RTC data was good, make a quick connection
    WiFi.begin(ssid, password, rtcData.channel, rtcData.ap_mac, true);
  }
  else {
    // The RTC data was not valid, so make a regular connection
    WiFi.begin(ssid, password);
  }

  int retries = 0;
  int wifiStatus = WiFi.status();
  while(wifiStatus != WL_CONNECTED) {
    retries++;
    if(retries == 100) {
      // Quick connect is not working, reset WiFi and try regular connection
      WiFi.disconnect();
      delay(10);
      WiFi.forceSleepBegin();
      delay(10);
      WiFi.forceSleepWake();
      delay(10);
      WiFi.begin( ssid, password );
    }
    if(retries == 300) {
      // Giving up after 30 seconds and going back to sleep
      WiFi.disconnect(true);
      delay(1);
      WiFi.mode(WIFI_OFF);
      ESP.deepSleep(5e6, WAKE_RF_DISABLED);
      return; // Not expecting this to be called, the previous call will never return.
    }
    delay(50);
    wifiStatus = WiFi.status();
  }

  // Write current connection info back to RTC
  rtcData.channel = WiFi.channel();
  memcpy(rtcData.ap_mac, WiFi.BSSID(), 6); // Copy 6 bytes of BSSID (AP's MAC address)
  rtcData.crc32 = calculateCRC32(((uint8_t*)&rtcData) + 4, sizeof( rtcData ) - 4);
  ESP.rtcUserMemoryWrite(0, (uint32_t*)&rtcData, sizeof(rtcData ));

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

// Check the CRC32
uint32_t EasyGraphs::calculateCRC32( const uint8_t *data, size_t length ) {
  uint32_t crc = 0xffffffff;
  while( length-- ) {
    uint8_t c = *data++;
    for( uint32_t i = 0x80; i > 0; i >>= 1 ) {
      bool bit = crc & 0x80000000;
      if( c & i ) {
        bit = !bit;
      }

      crc <<= 1;
      if( bit ) {
        crc ^= 0x04c11db7;
      }
    }
  }
  return crc;
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
