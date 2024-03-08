#include <Arduino.h>

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)

#include "lcd.h"
#include "roaster.h"

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

void setupLCD(void) {
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
      Serial.println(F("SSD1306 allocation failed"));
      for(;;);
  }
}

void welcomeLCD(void) {
  display.clearDisplay();

  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0,0);
  display.println(F("Skyduino Roaster"));
  display.print(F("Version: "));
  display.println(F(SKYDUINO_VERSION));
  display.display();
}

void updateLCD(void) {
  static ustick_t lcd_last_tick = micros();
  static bool invert_text = false;
  ustick_t tick = micros();

  if ((tick - lcd_last_tick) < (unsigned long)(UPDATE_LCD_PERIOD_MS * 1000)) {
    // no need to update yet
    return;
  }

  lcd_last_tick = tick;
  display.clearDisplay();
  display.setCursor(0,0);
  if (!(state.status.isSynchronized)) {
    if (invert_text) {
      display.setTextColor(SSD1306_BLACK, SSD1306_WHITE);
    } else {
      display.setTextColor(SSD1306_WHITE, SSD1306_BLACK);
    }
    invert_text = !invert_text;
    display.setTextSize(2);
    display.println(F("Sync Loss"));
    display.display();
    return;
  }
  
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.print(F("Temp: "));
  uint8_t mapping = 0;
  for (uint8_t i = 0; i < 2; i++) {
    mapping = state.cfg.chanMapping[i];
    if (mapping
        && (mapping >= 1)
        && (mapping <= TEMPERATURE_CHANNELS_MAX)) {
        display.print(state.reported.chanTemp[mapping - 1]);
        display.print(F(" "));
    } else {
      display.print(F("0.0 "));
    }
  }
  if (state.cfg.CorF == 'F') {
    display.println(F("F"));
  } else {
    display.println(F("C"));
  }
  

  // New line
  display.print(F("Heat: "));
  display.print(state.commanded.heat, 16);
  display.print(F(" Vent: "));
  display.print(state.commanded.vent, 16);
  display.println();

  // New line
  display.print(F("Fltr: "));
  display.print(state.commanded.filter, 16);
  display.print(F(" Cool: "));
  display.print(state.commanded.cool, 16);
  display.println();

  // New line
  if (state.commanded.drum) {
    display.println(F("Drum is On"));
  } else {
    display.println(F("Drum is Off"));
  }
  display.display();
}
#else
#define setupLCD(x)
#define welcomeLCD(x)
#define updateLCD(x)
#endif

