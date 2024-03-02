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


class _SWProtocolBase {
    protected:
        uint8_t *buffer;
        size_t bufferSize;
        uint8_t calculateCRC();
        // constructor for ProtocolTx/Rx child classes
        _SWProtocolBase() {};
        _SWProtocolBase(uint8_t *buffer, size_t bufferSize);
        void _clearBuffer();
    public:
        virtual void begin() {};
        virtual void loopTick() {};
        void shutdown();
};


class _SWProtocolTx: public _SWProtocolBase {
    protected:
        uint32_t pin;
        uint32_t lastTick;
        _SWProtocolTx(uint32_t txpin, uint8_t *buffer, size_t bufferSize):
            _SWProtocolBase(buffer, bufferSize), pin(txpin) {};
        void updateCRC();
        void sendBit(uint8_t value);
        void sendPreamble();
    public:
        void loopTick();
        bool setByte(uint8_t idx, uint8_t value);
        void sendMessage();
};


class _SWProtocolRx: public _SWProtocolBase {
    protected:
        uint32_t pin;
        uint32_t lastSuccRx;
        _SWProtocolRx(uint32_t rxpin, uint8_t *buffer, size_t bufferSize);
        bool receiveFrame();
        bool verifyCRC();
    public:
        bool getByte(uint8_t idx, uint8_t *value);
        bool isSynchronized();
        void loopTick();

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