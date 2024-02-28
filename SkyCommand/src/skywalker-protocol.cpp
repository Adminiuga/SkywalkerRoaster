#include <Arduino.h>

#include "skywalker-protocol.h"


/*
 * Calculate checksum of the buffer
 */
uint8_t _SWProtocolBase::calculateCRC() {
    uint8_t crc = 0;
    for (unsigned int i = 0; i < (bufferSize - 1); i++) {
        crc += buffer[i];
    }
    return crc;
}


/*
 * update checksum based on the buffer content
 */
void _SWProtocolBase::updateCRC() {
    buffer[bufferSize] = calculateCRC();
}


/*
 * verify CRC, return true of crc matches buffer content
 */
bool _SWProtocolBase::verifyCRC() {
    return buffer[bufferSize] == calculateCRC();
}


void SWControllerTx::begin(void) {
  pinMode(CONTROLLER_PIN_TX, OUTPUT);
}