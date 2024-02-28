#include <Arduino.h>

#include "skywalker-protocol.h"


/*
 * Base Protocol constuctor: clear up the buffer
 */
_SWProtocolBase::_SWProtocolBase(uint8_t *buffer, size_t bufferSize) : buffer(buffer), bufferSize(bufferSize) {
    for (unsigned int i = 0; i < bufferSize; i++) {
        buffer[i] = 0;
    }
};

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
void _SWProtocolTx::updateCRC() {
    buffer[bufferSize] = calculateCRC();
}


/*
 * verify CRC, return true of crc matches buffer content
 */
bool _SWProtocolRx::verifyCRC() {
    return buffer[bufferSize] == calculateCRC();
}


/*
 * Initialize Controller transmission
 */
void SWControllerTx::begin(void) {
  pinMode(CONTROLLER_PIN_TX, OUTPUT);
}


/*
 * set buffer byte*/