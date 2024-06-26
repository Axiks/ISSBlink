#include <stdio.h>
#include <Arduino.h>
#include <EEPROM.h>

// Display lib
#include <OledDisplay.h> 
#include <NokiaDisplay.h>

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

#include <models/Satelite.h>
#include <models/Prediction.h>
#include <models/SatelitePredictions.h>

// Config
#include <configuration.h>
#define CONFIG_VERSION "dem4"

//#define LED_PIN 22

// -- Method declarations.
//void handleRoot();
// -- Callback methods.
//void configSaved();
//bool formValidator(iotwebconf::WebRequestWrapper* webRequestWrapper);
//void wifiConnected();

#define STRING_LEN 128
#define NUMBER_LEN 32

bool timeLoad = false;
bool issLoad = false;

NokiaDisplay *nokiaDisplay;
OledDisplay *oledDisplay;


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

void configSaved()
{
  Serial.println("Configuration was updated.");
  nokiaDisplay -> printText("Restart");
  delay(1000);
  ESP.restart();
}

void clearEEPROM()
{
    Serial.println("Start clear EEPROM!");
    for (int i = 0; i < 512; i++) {
        EEPROM.write(i, 0);
    }
    EEPROM.commit();
    Serial.println("End clear EEPROM!");
}

class SateliteService{
    private:
      HTTPClient client;
      DynamicJsonDocument doc = DynamicJsonDocument(4096);
    public:
      SateliteService()
      {
        client.addHeader("Content-Type", "application/json");
      }

      SatelitePredictions loadSatelitePrediction(Satelite *satelite){
        String url = "https://api.n2yo.com/rest/v1/satellite/visualpasses/" + String(satelite->id) + '/' + String(satelite->lat) + '/' + String(satelite->lon) + '/' + String(satelite->alt) + '/' + String(satelite->predictionDays) + '/' + String(satelite->minVisiblityBrightness) + "/&apiKey=" + APIKEY;
        Serial.println("URL: " + url);
        client.begin(url);

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
                return NULL;
            }
            else{
                Serial.println("Parse Success!\nSize:");
                Serial.println(doc.size());
            }

            SatelitePredictions mySatelitePredictions = SatelitePredictions(satelite);
            
            int size = doc["passes"].size();
            String satName = doc["info"]["satname"];
            satelite -> name = satName;
            for(int i=0; i < size; i++){
                int startUTC = doc["passes"][i]["startUTC"];
                int maxUTC = doc["passes"][i]["maxUTC"];
                int endUTC = doc["passes"][i]["endUTC"];
                int duration = doc["passes"][i]["duration"];

                mySatelitePredictions.addPrediction(new Prediction(startUTC, maxUTC, endUTC, duration));

                Serial.println("\nStart UTC: ");
                Serial.println(String(hour(startUTC)) + ":" + String(minute(startUTC))+ ":" + String(second(startUTC)));
                Serial.println("Max UTC: ");
                Serial.println(String(hour(maxUTC)) + ":" + String(minute(maxUTC))+ ":" + String(second(maxUTC)));
                Serial.println("End UTC: ");
                Serial.println(String(hour(endUTC)) + ":" + String(minute(endUTC))+ ":" + String(second(endUTC)));
                Serial.println("Duration: ");
                Serial.println(String(duration));
            }
            Serial.println("End of added predictions!");
            Serial.println("Count of predictions: ");
            Serial.println(mySatelitePredictions.count());
            return mySatelitePredictions;
        }
        return NULL;
      }
};

class APControl{
    private:
      DNSServer dnsServer = DNSServer();
      const char apName[31] = "ISS_Blink_AP";
      const char apInitialPassword[31] = "12345678";

    public:
      IotWebConf iotWebConf;
      WebServer server;
      APControl()
      : server(80), iotWebConf(apName, &dnsServer, &server, apInitialPassword)
      {};

      void up(){
        // -- Init AP
        iotWebConf.setApTimeoutMs(1000);
        iotWebConf.setWifiConnectionTimeoutMs(5000);
        bool isInitConf = iotWebConf.init();
        Serial.println("AP configuration init status: " + String(isInitConf));
        if(isInitConf){
          nokiaDisplay -> printText("In AP mode");
        }
      }

      void down(){
        iotWebConf.goOffLine();
        //Turn OFF WiFi!
        WiFi.disconnect();
        WiFi.mode(WIFI_OFF);
      }
};

Satelite *mySatelite;
APControl *apControl;
SatelitePredictions *satelitePredictions;

void wifiConnected()
{
    Serial.println("Wifi Connected!");
    String ip = WiFi.localIP().toString();
    Serial.println(ip);
    nokiaDisplay -> printText(ip);

    // Load time
    loadTime();
    timeLoad = true;

    //Get ISS predictors
    SateliteService sateliteService;
    satelitePredictions = new SatelitePredictions(mySatelite);
    *satelitePredictions = sateliteService.loadSatelitePrediction(mySatelite);
    issLoad = true;

    // Off AP
    apControl -> down();
}

class WebApp{
    private:
      WebServer *server;
      IotWebConf *iotWebConf;
      IotWebConfParameterGroup* group1 = new IotWebConfParameterGroup("group1", "ISS config");
      Satelite *satelite;

      char lat[7];
      char lon[7];
      char observerAlt[7];
      char minVisiblityBrightness[7];
      char predictionDays[7];
      
      void initUi(){
        group1->addItem(new IotWebConfTextParameter("Lat", "nekoLatParam", lat, STRING_LEN, "0.000"));
        group1->addItem(new IotWebConfTextParameter("Lon", "nekoLonParam", lon, STRING_LEN, "0.000"));
        group1->addItem(new IotWebConfNumberParameter("Observer alt", "nekoObserverAltParam", observerAlt, NUMBER_LEN, "100", "0..9000 metres", "min='0' max='9000' step='1'"));
        group1->addItem(new IotWebConfNumberParameter("Minimum visiblity brightness", "minimumVisiblityBrightnessParam", minVisiblityBrightness, NUMBER_LEN, "100", "1..1000", "min='1' max='1000' step='1'"));
        group1->addItem(new IotWebConfNumberParameter("Number of days of prediction", "numberOfDaysOfPrediction", predictionDays, NUMBER_LEN, "2", "max 10", "min='1' max='10' step='1'"));
        iotWebConf->addParameterGroup(group1);

        iotWebConf -> setWifiConnectionCallback(&configSaved);
        iotWebConf -> setConfigSavedCallback(&configSaved);
        //iotWebConf -> setFormValidator(&formValidator);
        iotWebConf -> getApTimeoutParameter()->visible = true;
        iotWebConf -> setWifiConnectionCallback(&wifiConnected);
      };

      // bool static formValidator(iotwebconf::WebRequestWrapper* webRequestWrapper)
      // {
      //   Serial.println("Validating form.");
      //   bool valid = true;

      // /*
      //   int l = webRequestWrapper->arg(stringParam.getId()).length();
      //   if (l < 3)
      //   {
      //     stringParam.errorMessage = "Please provide at least 3 characters for this test!";
      //     valid = false;
      //   }
      // */
      //   return valid;
      // }

      void handleRoot()
      {
        // -- Let IotWebConf test and handle captive portal requests.
        if (iotWebConf -> handleCaptivePortal())
        {
          // -- Captive portal request were already served.
          return;
        }
        String s = "<!DOCTYPE html><html lang=\"en\"><head><meta charset=\"ASCII\" name=\"viewport\" content=\"width=device-width, initial-scale=1, user-scalable=no\"/>";
        s += "<title>IotWebConf 01 Minimal</title></head><body>";
        s += "Go to <a href='config'>configure page</a> to change settings.";
        s += "</body></html>\n";

        server -> send(200, "text/html", s);
      }

      void initRoutes(){
        // -- Set up required URL handlers on the web server.
        server -> on("/", [&]{handleRoot();});
        server -> on("/config", [&]{ iotWebConf -> handleConfig(); });
        server -> on("/reset", []{ clearEEPROM(); });
        server -> onNotFound([&](){ iotWebConf -> handleNotFound(); });
      }

      DNSServer dnsServer;

    public:
      WebApp(WebServer *server, IotWebConf *iotWebConf, Satelite *satelite)
      : server(server), iotWebConf(iotWebConf), satelite(satelite)
      {
        initUi();
        initRoutes();
      }

      void start(){
        // -- Init AP
        bool initConf = iotWebConf->init();
        satelite -> predictionDays = atoi(predictionDays);
        satelite -> lat = atof(lat);
        satelite -> lon = atof(lon);
        satelite -> alt = atof(observerAlt);
        satelite -> minVisiblityBrightness = atof(minVisiblityBrightness);
      }
};

WebApp *myWebApp;
int startTime = 0;

void setup()
{
    Serial.begin(115200);
    //clearEEPROM();
    nokiaDisplay = new NokiaDisplay(18, 4, 0, 2, 15);
    //nokiaDisplay -> printText("Hello Neko Space!");
    nokiaDisplay -> printLogo();

    //OledDisplay oledDisplay = OledDisplay(23, 19);
    //oledDisplay.printText("Hello Neko Space!");

    mySatelite = new Satelite(25544, "ISS", 0.000, 0.000, 1, 100, 2);
    //satelitePredictions = new SatelitePredictions(mySatelite);

    apControl = new APControl();
    //apControl -> up();

    myWebApp = new WebApp(&apControl->server, &apControl->iotWebConf, mySatelite);
    myWebApp -> start();
    Serial.println("Satelite predictions days: " + String(mySatelite -> predictionDays));
    Serial.println("Satelite lat: " + String(mySatelite -> lat));
    Serial.println("Satelite lon: " + String(mySatelite -> lon));
    Serial.println("Satelite alt: " + String(mySatelite -> alt));
    Serial.println("Satelite minVisiblityBrightness: " + String(mySatelite -> minVisiblityBrightness));

    startTime = second();
}

// unsigned long previousMillis = 0;
// const long interval = 1000;

int currentPassesIndex = 0;
int loopDelay = 800;

void loop() {
    // -- doLoop should be called as frequently as possible.
    apControl -> iotWebConf.doLoop();

    if(timeLoad){
        String timeNowFormated = "t now " + formTime(now()+(2*60*60));
        nokiaDisplay -> printTime(timeNowFormated);
        //nokiaDisplay -> printText(timeNowFormated);
        //display.setFont(ArialMT_Plain_10);
        //display.drawString(0, 0, timeNowFormated);
        // display.display();
    }

    if(issLoad){
      //Serial.println("Satelite count: ");
      //Serial.println(satelitePredictions->count());
      if(satelitePredictions->count() > 0){
        //Serial.println("Satelite c");
        // DisplaycurrentPassesIndex 
        //display.setFont(ArialMT_Plain_10);
        //display.drawString(120, 0, String(doc["passes"].size() - currentPassesIndex));

        // Change the tracking date
        if(currentPassesIndex + 1 < satelitePredictions->count()){
          // implement load new prediction
          loopDelay = 1000;
        }
        else {
          nokiaDisplay -> printText("End prediction for " + mySatelite->name + "!");
        }

        if(satelitePredictions->predictions[currentPassesIndex]->getEndUTC() < now()){

          currentPassesIndex++;
        }

        // time to start
        int timeToStart = satelitePredictions->predictions[currentPassesIndex]->getStartUTC() - now();
        nokiaDisplay->printText("t -" + formTime(timeToStart));

        nokiaDisplay->printPredisctionsCount(satelitePredictions->count());

        // // Display satelite name
        // String satName = doc["info"]["satname"];
        nokiaDisplay->printSateliteName(mySatelite->name);

        // Display time AVG
        // time_t tStart = doc["passes"][currentPassesIndex]["startUTC"];
        // time_t tMax = doc["passes"][currentPassesIndex]["maxUTC"];
        // time_t tEnd = doc["passes"][currentPassesIndex]["endUTC"];
        // // Serial.println("Start passes time: " + formTime(tStart+(60*60)));
        // // Serial.println("Max passes time: " + formTime(tMax+(60*60)));
        // // Serial.println("End passes time: " + formTime(tEnd+(60*60)));

        // int avgStart = tStart - now();
        // int avgMax = tMax - now();
        // int avgEnd = tEnd - now();

        // if(now() <= tStart){
        //   display.setFont(ArialMT_Plain_16);
        //   display.drawString(0, 16, "- " + formTime(avgStart));
        // }
        // else if(now() > tStart && now() <= tMax){
        //   display.setFont(ArialMT_Plain_16);
        //   display.drawString(0, 16, "MAX - " + formTime(avgMax));
        // }
        // else if(now() > tMax && now() <= tEnd){
        //   display.setFont(ArialMT_Plain_16);
        //   display.drawString(0, 16, "+ " + formTime(avgEnd));
        // }

        // // display.setFont(ArialMT_Plain_10);
        // // display.setTextAlignment(TEXT_ALIGN_RIGHT);
        // // display.drawString(128, 54, String(millis()));
        // // write the buffer to the display
        // // display.display();

        // // Led blink
        // // Blink before arrival
        // int startTimeBlink = tStart - (60*30);

        // unsigned long currentMillis = millis();
        // if(now() >= startTimeBlink && now() < tStart){
        //     if (currentMillis - previousMillis >= interval) {
        //         // save the last time you blinked the LED
        //         previousMillis = currentMillis;
        //         // if the LED is off turn it on and vice-versa:
        //         if (ledState == LOW) {
        //             ledState = HIGH;
        //         } else {
        //             ledState = LOW;
        //         }
        //         // set the LED with the ledState of the variable:
        //         digitalWrite(LED_PIN, ledState);
        //     }
        // }
        // else if(now() >= tStart && now() < tEnd){
        //     // Shine during the flight
        //     digitalWrite(LED_PIN, HIGH);
        // }
        // else if(now() >= tEnd){
        //     previousMillis = 0;
        //     ledState = LOW;
        //     digitalWrite(LED_PIN, LOW);
        //     // Change the tracking date
        //     if(currentPassesIndex + 1 < doc["passes"].size()){
        //       currentPassesIndex++;
        //     }
        //     else {
        //       String satName = doc["info"]["satname"];
        //       display.setFont(ArialMT_Plain_16);
        //       display.drawString(0, 16, "End prediction for " + satName + "!");
        //     }
        // }
      }
      else{
        // String satName = doc["info"]["satname"];
        // String message = "Not " + satName + "!";
        // display.setFont(ArialMT_Plain_16);
        // display.drawString(0, 16, message);
      }
    }
    //display.display();

    delay(loopDelay);
    nokiaDisplay -> clearDisplay();
}