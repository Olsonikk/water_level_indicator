#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "epd_driver.h"
#include "OpenSans18B.h"
#include <stdio.h>


uint8_t *framebuffer = NULL;
// put function declarations here:
const char WIFI_SSID[] = "WiFi name";         // CHANGE IT         // CHANGE IT
const char WIFI_PASSWORD[] = "PASS"; // CHANGE IT

String HOST_NAME   = "https://hydro-back.imgw.pl"; // CHANGE IT
String PATH_NAME   = "/station/hydro/status?id=151160170";      // CHANGE IT

void WiFi_init()
{
  int attempts = 0;
  Serial.print("\r\nConnecting to: " + String(WIFI_SSID));
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    while(WiFi.status() != WL_CONNECTED) {
      Serial.print(".");
      delay(1000);
    }
    Serial.println();
    Serial.println("Connected!");
    Serial.println(WiFi.localIP());
}

void eink_init()
{
  epd_init();
  epd_poweron();
  epd_clear();
}

void fillCircle(int x, int y, int r, uint8_t color) {
  epd_fill_circle(x, y, r, color, framebuffer);
}


void setup() {
  const char *location;
  const char *time;
  const char *comb;
  float water_level;
  float alarm_level;
  char level_chr[20];
  char alarm_chr[20];

  framebuffer= (uint8_t *)ps_calloc(sizeof(uint8_t), EPD_WIDTH * EPD_HEIGHT / 2);
  Serial.begin(115200);
  delay(1000);
  WiFi_init();
  
  HTTPClient http;

  http.begin(HOST_NAME + PATH_NAME); //HTTP
  int httpCode = http.GET();

  if(httpCode > 0)
  {
    if(httpCode == HTTP_CODE_OK) {
      String payload = http.getString();
      JsonDocument doc;
      deserializeJson(doc, payload);

      //String currentState = doc["status"]["currentState"];
      location = doc["status"]["description"];
      
      time = doc["status"]["currentState"]["date"];
      water_level = doc["status"]["currentState"]["value"];
      alarm_level = doc["status"]["alarmValue"];

      Serial.println(payload);

      //Serial.println(time + " " + water_level);
    } else {
      Serial.printf("[HTTP] GET... code: %d\n", httpCode);
    }
  } else {
    Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
  }

  http.end();
  WiFi.disconnect();
  WiFi.mode(WIFI_OFF);
  int32_t x0 = 20;
  int32_t y0 = 30;
  sprintf(level_chr, "%.2f", water_level);
  sprintf(alarm_chr, "%.2f", alarm_level);
  // const int16_t x1 = 60;
  // const int16_t y1 = 60;
  eink_init();
  delay(100);
  write_string((GFXfont *)&OpenSans18B, (String(location) + "   " + String(time)).c_str(), &x0, &y0, NULL);
  Serial.println(level_chr);
  Serial.println(alarm_chr);
  delay(1000);
  Serial.println(String(location) + " " + String(time));
  Serial.println(water_level);
  //Serial.println(abc);
  x0=50;
  y0=80;
  write_string((GFXfont *)&OpenSans18B, ("Aktualny stan: " + String(level_chr) + " cm").c_str(), &x0, &y0, NULL);
  delay(1000);
  x0=50;
  y0+=10;
  write_string((GFXfont *)&OpenSans18B, ("Stan alarmowy: " + String(alarm_level) + " cm").c_str(), &x0, &y0, NULL);
  delay(1000);
  //write_string((GFXfont *)&OpenSans18B, buf, &x0, &y0, NULL);
  //writeln((GFXfont *)&OpenSans18B, "pivo", &x0, &y0, NULL);
  //epd_draw_grayscale_image(epd_full_screen(), framebuffer);
  //delay(1000);

  // x0 = 50;
  // y0 = 100;
  // int r=25;
  // fillCircle(x0, y0, r, 0x44);
  epd_poweroff();

  delay(1000);


  
}

void loop() {
  //memset(framebuffer, 0xFF, EPD_WIDTH * EPD_HEIGHT / 2);
  Serial.println("finito");
  //epd_poweron();
  //epd_draw_rect(random(10,550), random(10, EPD_HEIGHT), random(10, 160), random(10, 120), 0, framebuffer);
  //epd_draw_grayscale_image(epd_full_screen(), framebuffer);
  delay(5000);
  
  //epd_poweroff();

  // delay(10000);
  //epd_clear();
}

