#include <Arduino.h>

#include "skywalker-protocol.h"

void SWControllerTx::begin(void) {
  pinMode(CONTROLLER_PIN_TX, OUTPUT);
}