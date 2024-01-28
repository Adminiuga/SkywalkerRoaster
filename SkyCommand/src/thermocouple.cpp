#include <Arduino.h>
#include <MAX6675.h>
#define MAX_CS_PIN 10

#include "roaster.h"

extern double tc_temp_c;

static MAX6675 thermoCouple(MAX_CS_PIN, &SPI);

uint8_t processThermoCouple(void) {
    static ustick_t lastTick = 0;
    ustick_t tick = micros();
    if ((tick - lastTick)
        < (THERMOCOUPLE_UPDATE_INTERVAL_MS * 1000)) {
        // not yet our time
        return 0;
    }

    lastTick = tick;
    int status = thermoCouple.read();
    if (status == 0) {
        tc_temp_c = thermoCouple.getTemperature();
    }

    return status;
}


void setupThermoCouple(void) {
  SPI.begin();
  thermoCouple.begin();
}


double convertCelcius2Fahrenheit(double tempC) {
    return (tempC * 9.0 / 5.0) + 32;
}
