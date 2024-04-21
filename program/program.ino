/*
 Based on: https://esp32io.com/tutorials/esp32-http-request
 */

#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

Adafruit_SSD1306 display(128, 64, &Wire, -1);

const char WIFI_SSID[] = "YOUR_SSID";         // CHANGE IT         // CHANGE IT
const char WIFI_PASSWORD[] = "PASSWORD"; // CHANGE IT

String HOST_NAME   = "https://hydro-back.imgw.pl"; // CHANGE IT
String PATH_NAME   = "/station/hydro/status?id=152210170";      // CHANGE IT

void setup() {
  Serial.begin(9600); 

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  display.clearDisplay();
  display.display();

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.println("Connecting");
  while(WiFi.status() != WL_CONNECTED) {
    delay(1000);
  }
  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());
  
  HTTPClient http;

  http.begin(HOST_NAME + PATH_NAME); //HTTP
  int httpCode = http.GET();

  // httpCode will be negative on error
  if(httpCode > 0) {
    // file found at server
    if(httpCode == HTTP_CODE_OK) {
      String payload = http.getString();
      JsonDocument doc;
      deserializeJson(doc, payload);

      //String currentState = doc["status"]["currentState"];
      String location = doc["status"]["description"];
      String time = doc["status"]["currentState"]["date"];
      String water_level = doc["status"]["currentState"]["value"];
      displayText(water_level, time, location);
      Serial.println(payload);

      Serial.println(time + " " + water_level);
    } else {
      // HTTP header has been send and Server response header has been handled
      Serial.printf("[HTTP] GET... code: %d\n", httpCode);
    }
  } else {
    Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
  }

  http.end();
  
}

void loop() {
}

void displayText(String l, String t, String location)
{
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println(location);
  display.setCursor(0, 10);
  display.print("Current: ");
  display.print(l);
  display.println(" cm");
  display.setCursor(0, 55);
  display.println(t);
  display.display();
}
