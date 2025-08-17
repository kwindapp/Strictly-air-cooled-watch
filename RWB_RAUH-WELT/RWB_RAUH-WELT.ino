#include <TFT_eSPI.h>
#include <SPI.h>
#include <WiFi.h>
#include <WiFiManager.h>
#include "logo.h"
#include "backg.h"
#include "kiterRWB1.h"
#include "kiterRWB2.h"
#include "kiterRWB3.h"
#include "kiterRWB4.h"
#include "kiterRWB5.h"
#include "kiterRWB6.h"
#include "kiterRWB7.h"
#include "kiterRWB8.h"
#include <time.h>

// ---------------- TFT and Sprites ----------------
TFT_eSPI tft = TFT_eSPI();
TFT_eSprite txtSprite = TFT_eSprite(&tft);
TFT_eSprite background = TFT_eSprite(&tft);
TFT_eSprite logoSprite = TFT_eSprite(&tft);

TFT_eSprite kiterSprites[8] = {
  TFT_eSprite(&tft), TFT_eSprite(&tft), TFT_eSprite(&tft), TFT_eSprite(&tft),
  TFT_eSprite(&tft), TFT_eSprite(&tft), TFT_eSprite(&tft), TFT_eSprite(&tft)
};

// Kite bitmap info
const int kiteWidths[8]  = {kiterRWB1_WIDTH, kiterRWB2_WIDTH, kiterRWB3_WIDTH, kiterRWB4_WIDTH,
                             kiterRWB5_WIDTH, kiterRWB6_WIDTH, kiterRWB7_WIDTH, kiterRWB8_WIDTH};
const int kiteHeights[8] = {kiterRWB1_HEIGHT, kiterRWB2_HEIGHT, kiterRWB3_HEIGHT, kiterRWB4_HEIGHT,
                             kiterRWB5_HEIGHT, kiterRWB6_HEIGHT, kiterRWB7_HEIGHT, kiterRWB8_HEIGHT};
const uint16_t* kiteBitmaps[8] = {kiterRWB1, kiterRWB2, kiterRWB3, kiterRWB4,
                                   kiterRWB5, kiterRWB6, kiterRWB7, kiterRWB8};

// ---------------- Clock and Timezone ----------------
struct tm timeinfo;
int timezoneIndex = 0;

// Use TZ strings for automatic DST handling
struct Timezone {
  const char* name;
  const char* tzString;
};

Timezone timezones[] = {
  {"Zurich", "CET-1CEST,M3.5.0/2,M10.5.0/3"}, // Europe/Zurich
  {"UTC", "UTC0"},
  {"NewYork", "EST5EDT,M3.2.0/2,M11.1.0/2"},   // Eastern Time with DST
  {"Tokyo", "JST-9"}                           // Japan, no DST
};

const int numTimezones = sizeof(timezones)/sizeof(timezones[0]);

// ---------------- Animation ----------------
const int displayWidth = 320;
const int kiteCount = 8;
float kiteX[kiteCount] = { -80, -300, -500, -700, -1000, -1200, -1500, -1800 };
float kiteY[kiteCount] = { 30, 3, 20, 5, 10, -18 , 15, -18 };
float kiteSpeed[kiteCount] = { 0.5, 0.5, 0.5, 0.5, 0.5, 1.0, 0.5, 0.5 };

// ---------------- Button ----------------
const int buttonPin = 0;
bool buttonPressed = false;

// ---------------- Day Strings ----------------
const char* SDay[7] = {"Sun","Mon","Tue","Wed","Thu","Fri","Sat"};

// ---------------- Functions ----------------
void connectWiFi() {
  WiFiManager wm;
  wm.autoConnect("ESP32_Clock");
}

void initTime() {
  configTime(0, 0, "pool.ntp.org"); // keep UTC, we handle TZ with setenv
  delay(1000);
}

void setTimezone(int index) {
  setenv("TZ", timezones[index].tzString, 1);
  tzset();
}

void updateTime() {
  time_t now;
  time(&now);
  localtime_r(&now, &timeinfo);
}

void checkTimeZoneButton() {
  if (digitalRead(buttonPin) == LOW && !buttonPressed) {
    buttonPressed = true;
    timezoneIndex = (timezoneIndex + 1) % numTimezones;
    setTimezone(timezoneIndex);
    updateTime();
  } else if (digitalRead(buttonPin) == HIGH) {
    buttonPressed = false;
  }
}

// Update kite positions and keep sequence
void updateKitePositions(float delta) {
  for (int i = 0; i < kiteCount; i++) {
    kiteX[i] += kiteSpeed[i] * delta;

    if (kiteX[i] > displayWidth) {
      int prev = (i == 0) ? kiteCount - 1 : i - 1;
      kiteX[i] = kiteX[prev] - 400 - kiteWidths[i];
    }
  }
}

void drawKites() {
  for (int i = 0; i < kiteCount; i++) {
    kiterSprites[i].pushImage(0, 0, kiteWidths[i], kiteHeights[i], kiteBitmaps[i]);
    kiterSprites[i].pushToSprite(&background, (int)kiteX[i], (int)kiteY[i], TFT_BLACK);
  }
}

void drawTime() {
  txtSprite.fillSprite(TFT_BLACK);
  txtSprite.setTextColor(0xFEA0, TFT_BLACK);

  txtSprite.setTextFont(7);
  char timeStr[9];
  sprintf(timeStr, "%02d:%02d:%02d", timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
  txtSprite.drawString(timeStr, 50, 0);

  txtSprite.setTextFont(2);
  txtSprite.setTextColor(0xFEA0, TFT_BLACK);
  txtSprite.drawString(String(timeinfo.tm_mon+1)+"/"+String(timeinfo.tm_mday)+"/"+String(timeinfo.tm_year+1900), 5, 50);
  txtSprite.drawString(SDay[timeinfo.tm_wday], 80, 50);
 

  txtSprite.setTextColor(TFT_WHITE, TFT_BLACK);
  txtSprite.drawString("Akira Nakai", 130, 50);

  logoSprite.pushImage(0, 0, 20, 20, logo);
  logoSprite.pushToSprite(&background, 2, 0, TFT_BLACK);

  txtSprite.pushToSprite(&background, 0, 0, TFT_BLACK);
}

// ---------------- Setup ----------------
void setup() {
  tft.init();
  tft.setRotation(1);
  pinMode(buttonPin, INPUT_PULLUP);

  txtSprite.createSprite(320, 170);
  background.createSprite(320, 170);
  logoSprite.createSprite(20, 20);

  for (int i = 0; i < kiteCount; i++) {
    kiterSprites[i].createSprite(kiteWidths[i], kiteHeights[i]);
  }

  connectWiFi();
  initTime();
  setTimezone(timezoneIndex);
  updateTime();
}

// ---------------- Loop ----------------
void loop() {
  checkTimeZoneButton();

  static unsigned long lastTime = 0;
  unsigned long nowMs = millis();
  float delta = nowMs - lastTime;
  lastTime = nowMs;

  updateKitePositions(delta / 16.67);
  updateTime();

  background.pushImage(0, 0, 320, 170, backg);
  drawKites();
  drawTime();
  background.pushSprite(0, 0);

  delay(16);
}
