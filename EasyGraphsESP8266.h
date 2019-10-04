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

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include "WiFiUdp.h"

// EasyGraphs.io collections server api
#ifndef API_SERVER
#define API_SERVER "http://api.easygraphs.io"
#endif

// EasyGraphs.io collections server port
#ifndef API_SERVER_PORT
#define API_SERVER_PORT 4020
#endif

// Default device name
#ifndef DEVICE_NAME
#define DEVICE_NAME "ESP8266"
#endif

// Default device platform
#ifndef DEVICE_PLATFORM
#define DEVICE_PLATFORM "Arduino"
#endif

// Default device type
#ifndef DEVICE_TYPE
#define DEVICE_TYPE "ESP8266"
#endif

// Default debug stage
#ifndef DEBUG
#define DEBUG false
#endif

// Base struct of collection entity
typedef struct Collection {
  char*  name;
  float value;
} Collection;

// Base struct of errors
typedef struct Error {
  String message;
  int code;
} Error;

class EasyGraphs {
  public:
    EasyGraphs(char* deviceToken, bool debug = DEBUG);
    void setDeviceName(char* deviceName);
    void setDevicePlatform(char* devicePlatform);
    void setDeviceType(char* deviceType);
    void setDeviceLongitude(float longitude);
    void setDeviceLatitude(float latitude);
    void addParameter(char *name, float value);
    void initWIFI(char *ssid, char *password);
    bool publish();
    Error getError();
  private:
    bool debug = false;
    char* secretToken;
    char* deviceName;
    char* devicePlatform;
    char* deviceType;
    float deviceLongitude;
    float deviceLatitude;
    Collection * dataSets;
    uint8_t counter;
    HTTPClient http;
    Error error;
    void logDebug(String text);
};
