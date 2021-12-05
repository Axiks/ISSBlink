#include <stdio.h>
#include <Arduino.h>
#include <EEPROM.h>

// Oled display lib
#include <SPI.h>
#include <Wire.h>
#include "SSD1306Wire.h"

// Wifi lib
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

#include <IotWebConf.h>
#include <IotWebConfUsing.h>
#include <IotWebConfOptionalGroup.h>

//Time sync lib
#include <TimeLib.h>
#include <NTPClient.h>

// Config
#include <configuration.h>
#define CONFIG_VERSION "dem4"

#define SDA_PIN 23
#define SCL_PIN 19
#define LED_PIN 22

SSD1306Wire display(0x3c, SDA_PIN, SCL_PIN, GEOMETRY_128_32);

DynamicJsonDocument doc(2048);

const char thingName[] = "ISS_Blink_AP";
const char wifiInitialApPassword[] = "12345678";
// -- Method declarations.
void handleRoot();
// -- Callback methods.
void configSaved();
bool formValidator(iotwebconf::WebRequestWrapper* webRequestWrapper);
void wifiConnected();

#define STRING_LEN 128
#define NUMBER_LEN 32

char satelliteId[NUMBER_LEN];
char nekoLat[STRING_LEN];
char nekoLon[STRING_LEN];
char nekoObserverAlt[NUMBER_LEN];
char minimumVisiblityBrightness[NUMBER_LEN];
char numberOfDaysOfPrediction[NUMBER_LEN];

DNSServer dnsServer;
WebServer server(80);
IotWebConf iotWebConf(thingName, &dnsServer, &server, wifiInitialApPassword);

IotWebConfParameterGroup group1 = IotWebConfParameterGroup("group1", "ISS config");
IotWebConfNumberParameter satelliteIdParam = IotWebConfNumberParameter("Satellite Id", "sateliteIdParam", satelliteId, NUMBER_LEN, "25544", "1..100000", "min='1' max='100000' step='1'");
IotWebConfTextParameter nekoLatParam = IotWebConfTextParameter("Lat", "nekoLatParam", nekoLat, STRING_LEN, "0.000");
IotWebConfTextParameter nekoLonParam = IotWebConfTextParameter("Lon", "nekoLonParam", nekoLon, STRING_LEN, "0.000");
IotWebConfNumberParameter nekoObserverAltParam = IotWebConfNumberParameter("Observer alt", "nekoObserverAltParam", nekoObserverAlt, NUMBER_LEN, "100", "0..9000 metres", "min='0' max='9000' step='1'");
IotWebConfNumberParameter minimumVisiblityBrightnessParam = IotWebConfNumberParameter("Minimum visiblity brightness", "minimumVisiblityBrightnessParam", minimumVisiblityBrightness, NUMBER_LEN, "100", "1..1000", "min='1' max='1000' step='1'");
IotWebConfNumberParameter numberOfDaysOfPredictionParam = IotWebConfNumberParameter("Number of days of prediction", "numberOfDaysOfPrediction", numberOfDaysOfPrediction, NUMBER_LEN, "2", "max 10", "min='1' max='10' step='1'");

bool timeLoad = false;
bool issLoad = false;

String formTime(int datetime){
  String time = String(hour(datetime)) + ":" + String(minute(datetime))+ ":" + String(second(datetime));
  char buf[32];
  sprintf(buf, "%.2d:%.2d:%.2d",
      hour(datetime), minute(datetime), second(datetime));
  return buf;
}

void loadTime(){
  WiFiUDP ntpUDP;
  NTPClient timeClient(ntpUDP);
  timeClient.begin();
  timeClient.update();
  setTime(timeClient.getEpochTime());
  Serial.println("Now time");
  formTime(now());
}

void iss(){
    HTTPClient client;
    // String satteliteId = atoi(satteliteId);//"25544";
    // String lat = "1.100";
    // String lon = "11.200";
    // String observer_alt = "0"; // Observer's altitude above sea level in meters
    String days = "2"; // Number of days of prediction (max 10)
    // String min_visibility = "100"; //300
    String url = "https://api.n2yo.com/rest/v1/satellite/visualpasses/" + String(satelliteId) + '/' + nekoLat + '/' + nekoLon + '/' + nekoObserverAlt + '/'+ days +'/' + minimumVisiblityBrightness + "/&apiKey=" + APIKEY;
    Serial.println(url);
    client.begin(url);
    client.addHeader("Content-Type", "application/json");

    int httpCode = client.GET();

    if(httpCode > 0) {
        String payload = client.getString();
        Serial.println("\nStatus code: " + String(httpCode));
        Serial.println(payload);

        char json[2048];
        payload.toCharArray(json, 2048);

        DeserializationError error = deserializeJson(doc, json);

        if(error){
            Serial.println("DeserializationError: ");
            Serial.println(error.f_str());
            return;
        }
        else{
            Serial.println("Parse Success!\nSize:");
            Serial.println(doc.size());
        }
        
        int size = doc["passes"].size();
        for(int i=0; i < size; i++){
            int startUTC = doc["passes"][i]["startUTC"];
            int maxUTC = doc["passes"][i]["maxUTC"];
            int endUTC = doc["passes"][i]["endUTC"];
            int duration = doc["passes"][i]["duration"];

            Serial.println("\nStart UTC: ");
            Serial.println(String(hour(startUTC)) + ":" + String(minute(startUTC))+ ":" + String(second(startUTC)));
            Serial.println("Max UTC: ");
            Serial.println(String(hour(maxUTC)) + ":" + String(minute(maxUTC))+ ":" + String(second(maxUTC)));
            Serial.println("End UTC: ");
            Serial.println(String(hour(endUTC)) + ":" + String(minute(endUTC))+ ":" + String(second(endUTC)));
            Serial.println("Duration: ");
            Serial.println(String(duration));
        }
    }
}

/**
 * Handle web requests to "/" path.
 */
// void handleRoot()
// {
//   // -- Let IotWebConf test and handle captive portal requests.
//   if (iotWebConf.handleCaptivePortal())
//   {
//     // -- Captive portal request were already served.
//     return;
//   }
//   String s = "<!DOCTYPE html><html lang=\"en\"><head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1, user-scalable=no\"/>";
//   s += "<title>IotWebConf 01 Minimal</title></head><body>";
//   s += "Go to <a href='config'>configure page</a> to change settings.";
//   s += "</body></html>\n";

//   server.send(200, "text/html", s);
// }

/**
 * Handle web requests to "/" path.
 */
void handleRoot()
{
  // -- Let IotWebConf test and handle captive portal requests.
  if (iotWebConf.handleCaptivePortal())
  {
    // -- Captive portal request were already served.
    return;
  }
  String s = "<!DOCTYPE html><html lang=\"en\"><head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1, user-scalable=no\"/>";
  s += "<title>IotWebConf 03 Custom Parameters</title></head><body>Hello world!";
  s += "<ul>";
  s += "<li>Satellite Id: ";
  s += atoi(satelliteId);
    s += "<li>Lat: ";
  s += atoi(nekoLat);
    s += strlen(nekoLat);
    s += "<li>Lon: ";
  s += atoi(nekoLon);
     s += "<li>Observer alt: ";
  s += atoi(nekoObserverAlt);
    s += "<li>Min visibility: ";
  s += atoi(minimumVisiblityBrightness);
  s += "</ul>";
  s += "Go to <a href='config'>configure page</a> to change values.";
  s += "</body></html>\n";

  server.send(200, "text/html", s);
}

void configSaved()
{
  Serial.println("Configuration was updated.");
  display.setFont(ArialMT_Plain_16);
  display.drawString(0, 16, "Restart");
  ESP.restart();
}

void clearEEPROM()
{
    for (int i = 0; i < 512; i++) {
        EEPROM.write(i, 0);
    }
    EEPROM.commit();
    delay(500);
}

bool formValidator(iotwebconf::WebRequestWrapper* webRequestWrapper)
{
  Serial.println("Validating form.");
  bool valid = true;

/*
  int l = webRequestWrapper->arg(stringParam.getId()).length();
  if (l < 3)
  {
    stringParam.errorMessage = "Please provide at least 3 characters for this test!";
    valid = false;
  }
*/
  return valid;
}

void wifiConnected()
{
    Serial.println("Wifi Connected!");
    String ip = WiFi.localIP().toString();

    // Display
    display.clear();
    display.setTextAlignment(TEXT_ALIGN_LEFT);
    display.setFont(ArialMT_Plain_10);
    display.drawString(0, 0, "Wifi Connected!");
    display.setFont(ArialMT_Plain_16);
    display.drawString(0, 16, ip);
    // write the buffer to the display
    display.display();

    // Load time
    loadTime();
    timeLoad = true;

    // //Get ISS predictors
    iss();
    issLoad = true;
}

int x, minX;
void displayScrollText(String message)
{
  while(true){
    
  }

  display.clear();
  display.setFont(ArialMT_Plain_10);
  display.drawString(x, 0, message);

  display.display();
  x=x+8; // scroll speed, make more positive to slow down the scroll
  if(x < minX) x= display.width();
}

void setup() {
    pinMode (LED_PIN, OUTPUT);
    Serial.begin(115200);

    // Initialising the UI will init the display too.
    display.init();
    display.flipScreenVertically();
    display.setTextAlignment(TEXT_ALIGN_LEFT);

    display.setFont(ArialMT_Plain_10);
    display.drawString(0, 0, "Start");
    // write the buffer to the display
    display.display();

    group1.addItem(&satelliteIdParam);
    group1.addItem(&nekoLatParam);
    group1.addItem(&nekoLonParam);
    group1.addItem(&nekoObserverAltParam);
    group1.addItem(&minimumVisiblityBrightnessParam);
    group1.addItem(&numberOfDaysOfPredictionParam);
    iotWebConf.addParameterGroup(&group1);

    iotWebConf.setConfigSavedCallback(&configSaved);
    iotWebConf.setFormValidator(&formValidator);
    iotWebConf.getApTimeoutParameter()->visible = true;

    // Init AP
    bool initConf = iotWebConf.init();
    Serial.println("Init configuration status: " + String(initConf));

    // -- Set up required URL handlers on the web server.
    server.on("/", handleRoot);
    server.on("/config", []{ iotWebConf.handleConfig(); });
    server.on("/reset", []{ clearEEPROM(); });
    server.onNotFound([](){ iotWebConf.handleNotFound(); });
    iotWebConf.setWifiConnectionCallback(&wifiConnected);
    // iotWebConf.checkConnection

    // // //Turn OFF WiFi!
    // WiFi.disconnect();
    // // //   WiFi.mode(WIFI_OFF);
}

int ledState = LOW;
unsigned long previousMillis = 0;
const long interval = 1000;

int currentPassesIndex = 0;

void loop() {
    // -- doLoop should be called as frequently as possible.
    iotWebConf.doLoop();

    // clear the display
    display.clear();
    display.setTextAlignment(TEXT_ALIGN_LEFT);

    if(timeLoad){
        String timeNowFormated = "time " + formTime(now()+(60*60));
        display.setFont(ArialMT_Plain_10);
        display.drawString(0, 0, timeNowFormated);
        // display.display();
    }

    if(issLoad){
      if(doc["passes"].size() > 0){
        // DisplaycurrentPassesIndex 
        display.setFont(ArialMT_Plain_10);
        display.drawString(120, 0, String(doc["passes"].size() - currentPassesIndex));

        // Display time AVG
        time_t tStart = doc["passes"][currentPassesIndex]["startUTC"];
        time_t tMax = doc["passes"][currentPassesIndex]["maxUTC"];
        time_t tEnd = doc["passes"][currentPassesIndex]["endUTC"];
        // Serial.println("Start passes time: " + formTime(tStart+(60*60)));
        // Serial.println("Max passes time: " + formTime(tMax+(60*60)));
        // Serial.println("End passes time: " + formTime(tEnd+(60*60)));

        int avgStart = tStart - now();
        int avgMax = tMax - now();
        int avgEnd = tEnd - now();

        if(now() <= tStart){
          display.setFont(ArialMT_Plain_16);
          display.drawString(0, 16, "- " + formTime(avgStart));
        }
        else if(now() > tStart && now() <= tMax){
          display.setFont(ArialMT_Plain_16);
          display.drawString(0, 16, "MAX - " + formTime(avgMax));
        }
        else if(now() > tMax && now() <= tEnd){
          display.setFont(ArialMT_Plain_16);
          display.drawString(0, 16, "+ " + formTime(avgEnd));
        }

        // display.setFont(ArialMT_Plain_10);
        // display.setTextAlignment(TEXT_ALIGN_RIGHT);
        // display.drawString(128, 54, String(millis()));
        // write the buffer to the display
        // display.display();

        // Led blink
        // Blink before arrival
        int startTimeBlink = tStart - (60*30);

        unsigned long currentMillis = millis();
        if(now() >= startTimeBlink && now() < tStart){
            if (currentMillis - previousMillis >= interval) {
                // save the last time you blinked the LED
                previousMillis = currentMillis;
                // if the LED is off turn it on and vice-versa:
                if (ledState == LOW) {
                    ledState = HIGH;
                } else {
                    ledState = LOW;
                }
                // set the LED with the ledState of the variable:
                digitalWrite(LED_PIN, ledState);
            }
        }
        else if(now() >= tStart && now() < tEnd){
            // Shine during the flight
            digitalWrite(LED_PIN, HIGH);
        }
        else if(now() >= tEnd){
            previousMillis = 0;
            ledState = LOW;
            digitalWrite(LED_PIN, LOW);
            // Change the tracking date
            if(currentPassesIndex + 1 < doc["passes"].size()){
              currentPassesIndex++;
            }
            else {
              String satName = doc["info"]["satname"];
              display.setFont(ArialMT_Plain_16);
              display.drawString(0, 16, "End prediction for " + satName + "!");
            }
        }
      }
      else{
        String satName = doc["info"]["satname"];
        String message = "Not " + satName + "!";
        display.setFont(ArialMT_Plain_16);
        display.drawString(0, 16, message);
      }
    }
    display.display();

    delay(200);
}