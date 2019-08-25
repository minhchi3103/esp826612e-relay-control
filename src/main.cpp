#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ESP_EEPROM.h>

AsyncWebServer server(80);

const char *ssid = "Nhax";
const char *password = "0986177993010605";

int countdown = 10;

struct MyEEPROMStruct {
    bool L1; 
    bool L2; 
    bool L3; 
    bool L4;
    char DeviceName[13];
} deviceInfo;

void notFound(AsyncWebServerRequest *request)
{
    request->send(404, "text/plain", "Not found");
}
void deviceInfoAndStatus(AsyncWebServerRequest *request)
{
    request->send(200, "application/json", 
            "{\"device_name\":\"" +String(deviceInfo.DeviceName)+"\","
            +"\"device_status\":{"
            +"\"den1\":\""+deviceInfo.L1+"\","
            +"\"den2\":\""+deviceInfo.L2+"\","
            +"\"den3\":\""+deviceInfo.L3+"\","
            +"\"den4\":\""+deviceInfo.L4+"\""
            + "}}");
}
void setPinState()
{
    /**
     * Turn on relay when HIGH
     * Turn off relay when LOW
     */
    digitalWrite(D1,!deviceInfo.L1);
    digitalWrite(D2,!deviceInfo.L2);
    digitalWrite(D3,!deviceInfo.L3);
    digitalWrite(D4,!deviceInfo.L4);
}
void saveState(){
    EEPROM.begin(sizeof(MyEEPROMStruct));
    EEPROM.put(0, deviceInfo);
    boolean ok2 = EEPROM.commitReset();
    Serial.println((ok2) ? "Second commit OK" : "Commit failed");
}
void setup()
{
    /**
     * Config serial, wifi and pin mode
     */
    Serial.begin(115200);
    WiFi.mode(WIFI_STA);
    pinMode(D1,OUTPUT);
    pinMode(D2,OUTPUT);
    pinMode(D3,OUTPUT);
    pinMode(D4,OUTPUT);
    setPinState();
    /**
     * Set default device info
     */
    deviceInfo.L1 = LOW;
    deviceInfo.L2 = LOW;
    deviceInfo.L3 = LOW;
    deviceInfo.L4 = LOW;
    char tmp[]="phongkhach"; //maximum 12 character
    std::copy(std::begin(tmp), std::end(tmp), std::begin(deviceInfo.DeviceName));
    saveState();
    /**
     * Async webserver
     */

    // Get device info and status
    server.on("/", HTTP_GET, deviceInfoAndStatus);

    // Send a GET request to <IP>/get?message=<message>
    server.on("/set", HTTP_GET, [](AsyncWebServerRequest *request) {
        if (request->hasParam("den1"))
        {
            deviceInfo.L1 = request->getParam("den1")->value()=="1"?1:0;
        }
        if (request->hasParam("den2"))
        {
            deviceInfo.L2 = request->getParam("den2")->value()=="1"?1:0;
        }
        if (request->hasParam("den3"))
        {
            deviceInfo.L3 = request->getParam("den3")->value()=="1"?1:0;
        }
        if (request->hasParam("den4"))
        {
            deviceInfo.L4 = request->getParam("den4")->value()=="1"?1:0;
        }
        setPinState();
        deviceInfoAndStatus(request);
    });

    server.onNotFound(notFound);

    server.begin();
}

void loop()
{
    if (WiFi.waitForConnectResult() != WL_CONNECTED)
    {
        countdown = 10;
        delay(5000);
        Serial.println("Try to connect");
        WiFi.begin(ssid, password);
        delay(5000);
    }
    if (WiFi.waitForConnectResult() == WL_CONNECTED && countdown>0)
    {
        Serial.print("IP Address: ");
        Serial.println(WiFi.localIP().toString());
        countdown--;
    }
}