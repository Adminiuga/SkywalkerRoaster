#include <Arduino.h>
#include <MAX6675.h>
#ifndef MAX_CS_PIN
#define MAX_CS_PIN PIN_SPI_SS
#endif  // MAX_CS_PIN

#include "roaster.h"
#include "thermocouple.h"
#include "tick-timer.h"

extern t_State state;
static MAX6675 thermoCouple(MAX_CS_PIN, &SPI);
static TimerMS timer(THERMOCOUPLE_UPDATE_INTERVAL_MS);

uint8_t processThermoCouple(void) {

    if ( ! (timer.hasTicked())) {
        // not yet our time
        return 0;
    }
    timer.reset();

    int status = thermoCouple.read();
    if (status == 0) {
        if (state.cfg.CorF == 'C') {
            TEMPERATURE_TC(state.reported) = thermoCouple.getTemperature();
        } else {
            TEMPERATURE_TC(state.reported) = convertCelcius2Fahrenheit(thermoCouple.getTemperature());
        }
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
