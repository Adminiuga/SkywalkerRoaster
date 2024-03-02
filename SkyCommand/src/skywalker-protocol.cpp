#include <Arduino.h>

#include "skywalker-protocol.h"

#define SWPROT_TX_1_LENGTH_US       1500
#define SWPROT_TX_0_LENGTH_US       650
#define SWPROT_TX_PREAMBLE_LOW_US   7500UL
#define SWPROT_TX_PREAMBLE_HIGH_US  3800UL
#define SWPROT_RX_PREAMBLE_LOW_US   7000U
#define SWPROT_RX_1_LENGTH_US       1200U
#define MESAGE_SEND_INTERVAL_US     200000UL


/*
 * Base Protocol constuctor: clear up the buffer
 */
_SWProtocolBase::_SWProtocolBase(uint8_t *buffer, size_t bufferSize) : buffer(buffer), bufferSize(bufferSize) {
    _clearBuffer();
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
 * Zero out the buffer, aka shutdown
 */
void _SWProtocolBase::_clearBuffer() {
    for (unsigned int i = 0; i < bufferSize; i++) {
        buffer[i] = 0;
    }
};

void _SWProtocolBase::shutdown() {
    return _clearBuffer();
}


/*
 * Initialize Controller transmission
 */
void SWControllerTx::begin(void) {
  pinMode(pin, OUTPUT);
}


/*
 * update checksum based on the buffer content
 */
void _SWProtocolTx::updateCRC() {
    buffer[bufferSize - 1] = calculateCRC();
}


/*
 * pulse the pin for 1 or 0
 */
void _SWProtocolTx::sendBit(uint8_t value) {
  //Assuming pin is HIGH when we get it
  digitalWrite(pin, LOW);
  delayMicroseconds(value ? SWPROT_TX_1_LENGTH_US : SWPROT_TX_0_LENGTH_US);
  digitalWrite(pin, HIGH);
  delayMicroseconds(SWPROT_TX_1_LENGTH_US >> 1);  //delay between bits
  //we leave it high
}


/*
 * send preamble
 */
void _SWProtocolTx::sendPreamble() {
    //Assuming pin is HIGH when we get it
    digitalWrite(pin, LOW);
    delayMicroseconds(SWPROT_TX_PREAMBLE_LOW_US);
    digitalWrite(pin, HIGH);
    delayMicroseconds(SWPROT_TX_PREAMBLE_HIGH_US);
    //we leave it high
}


/*
 * Send current buffer
 */
void _SWProtocolTx::sendMessage() {
    // Update crc, at the time of sending
    updateCRC();
    // send Preamble
    sendPreamble();

    // send Message
    for (uint8_t i = 0; i < bufferSize; i++) {
        for (uint8_t bit = 0; bit < 8; bit++) {
            sendBit(bitRead(buffer[i], bit));
        }
    }
}


/*
 * Set byte in buffer. returns true if successful
 */
bool _SWProtocolTx::setByte(uint8_t idx, uint8_t value) {
    if (idx > bufferSize) return false;

    buffer[idx] = value;
    return true;
}


/*
 * Process loop iteration
 */
void _SWProtocolTx::loopTick() {
    if ((micros() - lastTick) < MESAGE_SEND_INTERVAL_US) return;

    sendMessage();
    lastTick = micros();
}


/*
 * verify CRC, return true of crc matches buffer content
 */
bool _SWProtocolRx::verifyCRC() {
    return buffer[bufferSize - 1] == calculateCRC();
}


/*
 * get byte from buffer. returns true if successful
 */
bool _SWProtocolRx::getByte(uint8_t idx, uint8_t *value) {
    if (idx > bufferSize) return false;

    *value = buffer[idx];
    return true;
}