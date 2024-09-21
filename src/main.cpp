#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "epd_driver.h"
#include "OpenSans18B.h"
#include <stdio.h>
#include "credentials.h"

#define CHART_WIDTH 860
#define CHART_HEIGHT 310

#define BLACK 0x00

uint8_t *framebuffer = NULL;
// put function declarations here:


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


void drawChart(const int coords_x, const int coords_y, const int size_x, const int size_y)
{
  epd_draw_rect(coords_x, coords_y, size_x, size_y, BLACK, framebuffer);
  epd_draw_rect(coords_x-1, coords_y-1, size_x+2, size_y+2, BLACK, framebuffer);
  
  const int segments = 6;
  const int dist_between_y_lines = CHART_WIDTH/segments;

  for(int i=1; i < segments; i++)
  {
    epd_draw_vline(coords_x + i * dist_between_y_lines, coords_y, CHART_HEIGHT, BLACK, framebuffer);
  }

  int chart_x = coords_x;
  int chart_y = CHART_HEIGHT + coords_y;
  Serial.print("ASD");
  Serial.println(chart_x);
  Serial.println(chart_y);
  
  epd_fill_circle(chart_x+dist_between_y_lines, chart_y - 100, 10, BLACK, framebuffer);
  epd_fill_circle(chart_x+2*dist_between_y_lines, chart_y - 250, 10, BLACK, framebuffer);
  
  epd_fill_circle(chart_x+3*dist_between_y_lines, chart_y - 270, 10, BLACK, framebuffer);
  

  epd_write_line(chart_x+dist_between_y_lines, chart_y - 100, chart_x+2*dist_between_y_lines, chart_y - 250, 0x44, framebuffer); //
  epd_write_line(chart_x+2*dist_between_y_lines, chart_y - 250, chart_x+3*dist_between_y_lines, chart_y - 270, BLACK, framebuffer);
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
  memset(framebuffer, 0xFF, EPD_WIDTH * EPD_HEIGHT / 2);
  Serial.begin(115200);
  delay(1000);
  // WiFi_init();
  
  // HTTPClient http;

  // http.begin(HOST_NAME + PATH_NAME); //HTTP
  // int httpCode = http.GET();

  // if(httpCode > 0)
  // {
  //   if(httpCode == HTTP_CODE_OK) {
  //     String payload = http.getString();
  //     JsonDocument doc;
  //     deserializeJson(doc, payload);

  //     //String currentState = doc["status"]["currentState"];
  //     location = doc["status"]["description"];
      
  //     time = doc["status"]["currentState"]["date"];
  //     water_level = doc["status"]["currentState"]["value"];
  //     alarm_level = doc["status"]["alarmValue"];

  //     Serial.println(payload);

  //     //Serial.println(time + " " + water_level);
  //   } else {
  //     Serial.printf("[HTTP] GET... code: %d\n", httpCode);
  //   }
  // } else {
  //   Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
  // }

  // http.end();
  // WiFi.disconnect();
  // WiFi.mode(WIFI_OFF);
  int32_t x0 = 20;
  int32_t y0 = 30;
  sprintf(level_chr, "%.2f", water_level);
  sprintf(alarm_chr, "%.2f", alarm_level);
  // const int16_t x1 = 60;
  // const int16_t y1 = 60;
  eink_init();
  delay(100);
  //write_string((GFXfont *)&OpenSans18B, (String(location) + "   " + String(time)).c_str(), &x0, &y0, NULL);
  write_string((GFXfont *)&OpenSans18B, "BRZEG DOLNY     2024", &x0, &y0, NULL);
  Serial.println(level_chr);
  Serial.println(alarm_chr);
  delay(100);
  Serial.println(String(location) + " " + String(time));
  Serial.println(water_level);
  //Serial.println(abc);
  x0=50;
  y0=80;
  write_string((GFXfont *)&OpenSans18B, ("Aktualny stan: " + String(level_chr) + " cm").c_str(), &x0, &y0, NULL);
  Serial.println("x y po stan aktualny: " + String(x0) + " " + String(y0));
  delay(100);
  x0=50;
  y0+=10;
  Serial.println("x y przed stan alarmowy: " + String(x0) + " " + String(y0));
  write_string((GFXfont *)&OpenSans18B, ("Stan alarmowy: " + String(alarm_level) + " cm").c_str(), &x0, &y0, NULL);
  //y0+=10;
  x0=70;
  Serial.println("x y do wykresu: " + String(x0) + " " + String(y0));
  delay(1000);
  drawChart(x0,y0, CHART_WIDTH, CHART_HEIGHT);
  //write_string((GFXfont *)&OpenSans18B, buf, &x0, &y0, NULL);
  //writeln((GFXfont *)&OpenSans18B, "pivo", &x0, &y0, NULL);
  epd_draw_grayscale_image(epd_full_screen(), framebuffer);
  //delay(1000);

  // x0 = 50;
  // y0 = 100;
  // int r=25;

  delay(1000);


  epd_poweroff_all();
}

void loop() {
  
  
  //epd_poweroff();

  // delay(10000);
  //epd_clear();
}

