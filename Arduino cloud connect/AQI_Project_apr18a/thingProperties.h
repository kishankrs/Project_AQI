// Code generated by Arduino IoT Cloud, DO NOT EDIT.

#include <ArduinoIoTCloud.h>
#include <Arduino_ConnectionHandler.h>

const char DEVICE_LOGIN_NAME[]  = "03c3af77-7486-4361-a0e9-9b943698ecdb";

const char SSID[]               = SECRET_SSID;    // Network SSID (name)
const char PASS[]               = SECRET_OPTIONAL_PASS;    // Network password (use for WPA, or use as key for WEP)
const char DEVICE_KEY[]  = SECRET_DEVICE_KEY;    // Secret device password


int aQI;
int pM1;
int pM10;
int pM2_5;

void initProperties(){

  ArduinoCloud.setBoardId(DEVICE_LOGIN_NAME);
  ArduinoCloud.setSecretDeviceKey(DEVICE_KEY);
  ArduinoCloud.addProperty(aQI, READ, 5 * SECONDS, NULL);
  ArduinoCloud.addProperty(pM1, READ, 5 * SECONDS, NULL);
  ArduinoCloud.addProperty(pM10, READ, 5 * SECONDS, NULL);
  ArduinoCloud.addProperty(pM2_5, READ, 5 * SECONDS, NULL);

}

WiFiConnectionHandler ArduinoIoTPreferredConnection(SSID, PASS);
