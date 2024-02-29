#ifndef __SKYWALKER_PROTOCOL_H
#define __SKYWALKER_PROTOCOL_H

#ifndef CONTROLLER_PIN_TX
#define CONTROLLER_PIN_TX           3  ; Send data to roaster (White USB-)
#endif // CONTROLLER_PIN_TX

#ifndef CONTROLLER_PIN_RX
#define CONTROLLER_PIN_RX           2  ; Recieve data from roaster (Green USB+)
#endif // CONTROLLER_PIN_RX

#define MESSAGE_LENGTH_ROASTER      7U
#define MESSAGE_LENGTH_CONTROLLER   6U
#define SWPROT_PREAMBLE_LENGTH_US   7000U
#define SWPROT_ONE_LENGTH_US        1200U


class _SWProtocolBase {
    protected:
        uint8_t *buffer;
        size_t bufferSize;
        uint8_t calculateCRC();
        // constructor for ProtocolTx/Rx child classes
        _SWProtocolBase() {};
        _SWProtocolBase(uint8_t *buffer, size_t bufferSize);
        void initializeBuffer();
    public:
        virtual void begin() {};
        void shutdown();
};


class _SWProtocolTx: public _SWProtocolBase {
    protected:
        uint32_t pin;
        _SWProtocolTx(uint32_t txpin, uint8_t *buffer, size_t bufferSize):
            _SWProtocolBase(buffer, bufferSize), pin(txpin) {};
        void updateCRC();
        void sendBit(uint8_t value);
        void sendPreamble();
    public:
        bool setByte(uint8_t idx, uint8_t value);
        void sendMessage();
};


class _SWProtocolRx: public _SWProtocolBase {
    protected:
        uint32_t pin;
        _SWProtocolRx(uint32_t rxpin, uint8_t *buffer, size_t bufferSize):
            _SWProtocolBase(buffer, bufferSize), pin(rxpin) {};
        bool verifyCRC();
    public:
        bool getByte(uint8_t idx, uint8_t *value);
};


class SWRoasterRx: public _SWProtocolRx {
    private:
        uint8_t bufMemory[MESSAGE_LENGTH_ROASTER];
    public:
        SWRoasterRx():
            _SWProtocolRx(CONTROLLER_PIN_RX, bufMemory, MESSAGE_LENGTH_ROASTER) {};
};


class SWControllerTx: public _SWProtocolTx{
    protected:
        uint8_t bufMemory[MESSAGE_LENGTH_CONTROLLER];
    public:
        SWControllerTx():
            _SWProtocolTx(CONTROLLER_PIN_TX, bufMemory, MESSAGE_LENGTH_CONTROLLER) {};
        void begin();
};


#endif  // __SKYWALKER_PROTOCOL_H