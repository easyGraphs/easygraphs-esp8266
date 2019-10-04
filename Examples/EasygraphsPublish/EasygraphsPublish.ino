#include "EasyGraphsESP8266.h"

#define DEVICE_TOKEN  "your_device_token"
#define WIFI_SSID "your_wifi_ssid"
#define WIFI_PASSWORD "your_wifi_password"

//Debug mode is enabling by second parameter
EasyGraphs easygraphs(DEVICE_TOKEN, true);

void setup() {
  Serial.begin(9600);

  easygraphs.initWIFI(WIFI_SSID, WIFI_PASSWORD);

  easygraphs.setDeviceName("your_device_name"); //Default value is: "ESP8266"
  easygraphs.setDeviceType("your_device_type"); //Default value is: "ESP8266"
  easygraphs.setDevicePlatform("your_device_platform"); //Default value is: "Arduino"

  /* Also, you can set up the position of your device
    easygraphs.setDeviceLatitude(40.730610);
    easygraphs.setDeviceLongitude(-73.935242);
  */
}

void loop(){

  //Add parameters to batch
  easygraphs.addParameter("data1", 10);
  easygraphs.addParameter("data2", 15);

  //Publish you data via api
  if (!easygraphs.publish()) {
    //Print the error details
    Serial.println(easygraphs.getError().message);
  }

  delay(2000);
}
