#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "epd_driver.h"
#include "OpenSans18B.h"
#include "OpenSans10B.h"
#include <stdio.h>
#include "credentials.h"

#define CHART_WIDTH 860
#define CHART_HEIGHT 310

#define BLACK 0x00

uint8_t *framebuffer = NULL;

struct Reading {
  String date;
  float value;
};

Reading lastReadings[7];
const char *location;
const char *curr_time;
float water_level;
float alarm_level;

//int water_levels_arr[7] = {369, 420, 421, 500, 492, 470, 460};   //
int min_water_level;                                    // Scaling gathered data section
int max_water_level;                                    //

String HOST_NAME   = "https://hydro-back.imgw.pl";
String PATH_NAME   = "/station/hydro/status?id=152140010";

String PATH_HISTORY_NAME   = "/station/hydro/status/data?id=152140010&hoursInterval=7";

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

bool isFullHour(const char* dateTime) {
  return dateTime[14] == '0' && dateTime[15] == '0';
}

void fetchData()
{
  

  HTTPClient http, http2;

  http.begin(HOST_NAME + PATH_NAME); //HTTP
  http2.begin(HOST_NAME + PATH_HISTORY_NAME);

  int httpCode = http.GET();
  int http2Code = http2.GET();

  if(httpCode > 0 && http2Code > 0)
  {
    if(httpCode == HTTP_CODE_OK && http2Code == HTTP_CODE_OK) {
      String payload = http.getString();
      String payload2 = http2.getString();

      JsonDocument doc, doc2;
      deserializeJson(doc, payload);
      deserializeJson(doc2, payload2);

      //String currentState = doc["status"]["currentState"];
      location = doc["status"]["description"];
      
      curr_time = doc["status"]["currentState"]["date"];
      water_level = doc["status"]["currentState"]["value"];
      alarm_level = doc["status"]["alarmValue"];

      Serial.println(payload2);
      JsonArray operational = doc2["operational"];
      int readingsCount = operational.size();

      for (int i = 0; i < 6; i++)
      {
      JsonObject reading = operational[i];
      lastReadings[i].date = reading["date"].as<String>();
      lastReadings[i].value = reading["value"].as<float>();
      Serial.print(lastReadings[i].date + " ");
      Serial.println(lastReadings[i].value);
      }
      Serial.println("AAAAAAAAAAA: " + String(readingsCount));
      JsonObject reading = operational[readingsCount - 1];
      lastReadings[6].date = reading["date"].as<String>();
      lastReadings[6].value = reading["value"].as<float>();
      Serial.print(lastReadings[6].date + " ");
      Serial.println(lastReadings[6].value);
    } else {
      Serial.printf("[HTTP] GET... code: %d\n", httpCode);
    }
  } else {
    Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
  }
  http.end();
}

void eink_init()
{
  epd_init();
  epd_poweron();
  epd_clear();
}


void drawLegend(int *coords_x, int *coords_y)
{
  int tmp_x = *coords_x - 40;
  int tmp_y = *coords_y;
  Serial.print("LEGENDA:");
  Serial.println(tmp_x);
  Serial.println(tmp_y);
  write_string((GFXfont *)&OpenSans10B, "500", &tmp_x, &tmp_y, framebuffer);
  Serial.print("LEGENDA2:");
  Serial.println(tmp_x);
  Serial.println(tmp_y);
  tmp_x = *coords_x - 40;
  tmp_y = *coords_y - CHART_HEIGHT + 10;
  write_string((GFXfont *)&OpenSans10B, "900", &tmp_x, &tmp_y, framebuffer);



}

void findMinMax(Reading lastReadings[7], int &min_val, int &max_val)
{
  min_val = lastReadings[0].value;
  max_val = lastReadings[0].value;

  for(int i=0; i < 7; i++)
  {
    if (lastReadings[i].value < min_val)
    {
      min_val = lastReadings[i].value;  // Znaleziono nową wartość minimalną
    }
    if (lastReadings[i].value > max_val)
    {
      max_val = lastReadings[i].value;
    }
  }
}

void scale_values(const int *y_min, Reading lastReadings[7]) 
// chart y_min and y_max are actually 502 and 192. The values in the arr need to be scaled so minimal value is scaled into 502 and max value into 192
// 502 = a*y_min + b
// 192 = a*y_max + b
// a = -2.37     b = 1376.53
{
  float a = (float)-CHART_HEIGHT/ (max_water_level - min_water_level);
  
  float b = *y_min - a * min_water_level;
  delay(1000);
  Serial.print("a: ");
  Serial.println(a, 3);
  Serial.println(b, 3);
  for(int i; i < 7; i++)
  {
    lastReadings[i].value = a * lastReadings[i].value + b; 
  }
}

void drawChart(const int coords_x, const int coords_y, const int size_x, const int size_y)
{
  findMinMax(lastReadings, min_water_level, max_water_level);
  epd_draw_rect(coords_x, coords_y, size_x, size_y, BLACK, framebuffer);
  epd_draw_rect(coords_x-1, coords_y-1, size_x+2, size_y+2, BLACK, framebuffer);
  
  const int segments_y = 6;
  const int segments_x = 4;
  const int dist_between_hori_lines = CHART_WIDTH/segments_y;  //distance between each vertical line on the graph
  const int dist_between_vert_lines = CHART_HEIGHT/segments_x;  //distance between each horizontal line on the graph

  int32_t move_y = coords_y;
  int32_t move_x = coords_x;

  for(int i=1; i < segments_y; i++)
  {
    move_x = coords_x + i*dist_between_hori_lines - 12;
    move_y = coords_y + CHART_HEIGHT + 20;
    write_string((GFXfont *)&OpenSans10B, (String(-(segments_y-i)) + "h").c_str(), &move_x , &move_y, framebuffer);     // x axis
    epd_draw_vline(coords_x + i * dist_between_hori_lines, coords_y, CHART_HEIGHT, BLACK, framebuffer);
  }
  float scale_vert=(float) max_water_level;
  move_y = coords_y + 10;
  move_x = coords_x - 50;
  write_string((GFXfont *)&OpenSans10B, String(max_water_level).c_str(), &move_x , &move_y, framebuffer);  //max vert
  move_y += CHART_HEIGHT;
  
  for(int i=1; i < segments_x; i++)
  {
    move_y = coords_y + i * dist_between_vert_lines + 5;
    move_x = coords_x - 50;
    scale_vert -= (float)(max_water_level - min_water_level) / (float) segments_x ;
    Serial.println("AD: " + String(6.0f / (float) segments_x));
    Serial.println(scale_vert);
    write_string((GFXfont *)&OpenSans10B, String(scale_vert).c_str(), &move_x , &move_y, framebuffer);    //y axis
    epd_draw_hline(coords_x, coords_y + i * dist_between_vert_lines, CHART_WIDTH, BLACK, framebuffer);
  }
  move_y = coords_y + CHART_HEIGHT + 5;
  move_x = coords_x - 50;
  write_string((GFXfont *)&OpenSans10B, String(min_water_level).c_str(), &move_x , &move_y, framebuffer); // min vert

  int chart_x = coords_x;
  
  int chart_y = CHART_HEIGHT + coords_y;

  scale_values(&chart_y,lastReadings);

  // for (int i = 0; i < 7; i++)
  // {
  // Serial.print("Element ");
  // Serial.print(i);
  // Serial.print(": ");
  // Serial.println(water_levels_arr[i]);
  // }
  // Serial.print("PRZED:");
  // Serial.println(chart_x);
  // Serial.println(chart_y);
  
  //filling the chart with actual values
  // epd_fill_circle(chart_x+dist_between_hori_lines, water_levels_arr[0], 8, BLACK, framebuffer);
  // epd_fill_circle(chart_x+2*dist_between_hori_lines, water_levels_arr[1], 8, BLACK, framebuffer);
  // epd_fill_circle(chart_x+3*dist_between_hori_lines, water_levels_arr[2], 8, BLACK, framebuffer);

  for (int i = 0; i < 6; i++)
  {
    epd_fill_circle(chart_x+i*dist_between_hori_lines, lastReadings[i].value, 8, BLACK, framebuffer);
    epd_write_line(chart_x+i*dist_between_hori_lines, lastReadings[i].value, chart_x+(i+1)*dist_between_hori_lines, lastReadings[i+1].value, 0x44, framebuffer);
  }
  
  // Serial.print("PO wykresie:");
  // Serial.println(chart_x);
  // Serial.println(chart_y);
  //drawLegend(&chart_x,&chart_y, 100, 400);
}

void setup() {
  

  char level_chr[20];
  char alarm_chr[20];


  framebuffer= (uint8_t *)ps_calloc(sizeof(uint8_t), EPD_WIDTH * EPD_HEIGHT / 2);
  memset(framebuffer, 0xFF, EPD_WIDTH * EPD_HEIGHT / 2);
  Serial.begin(115200);
  delay(1000);
  WiFi_init();
  fetchData();
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
  write_string((GFXfont *)&OpenSans18B, (String(location) + "   " + String(curr_time)).c_str(), &x0, &y0, NULL);
  //write_string((GFXfont *)&OpenSans18B, "BRZEG DOLNY     2024", &x0, &y0, NULL);
  Serial.println(level_chr);
  Serial.println(alarm_chr);
  delay(100);
  Serial.println(String(location) + " " + String(curr_time));
  Serial.println(water_level);
  //Serial.println(abc);
  x0=50;
  y0=80;
  write_string((GFXfont *)&OpenSans18B, ("Current level: " + String(level_chr) + " cm").c_str(), &x0, &y0, NULL);
  //Serial.println("x y po stan aktualny: " + String(x0) + " " + String(y0));
  delay(100);
  x0=50;
  y0+=10;
  //Serial.println("x y przed stan alarmowy: " + String(x0) + " " + String(y0));
  write_string((GFXfont *)&OpenSans18B, ("Alarm level: " + String(alarm_level) + " cm").c_str(), &x0, &y0, NULL);
  //y0+=10;
  x0=70;
  //Serial.println("x y do wykresu: " + String(x0) + " " + String(y0));
  delay(1000);
  drawChart(x0,y0, CHART_WIDTH, CHART_HEIGHT);
  //write_string((GFXfont *)&OpenSans18B, buf, &x0, &y0, NULL);
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

