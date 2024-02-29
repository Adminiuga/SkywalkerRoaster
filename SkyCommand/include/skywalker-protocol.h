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
    public:
        virtual void begin() {};
};


class _SWProtocolTx: protected _SWProtocolBase {
    protected:
        uint32_t pin;
        _SWProtocolTx(uint32_t txpin): pin(txpin) {};
        void updateCRC();
    public:
        bool setByte(uint8_t idx, uint8_t *value);
};


class _SWProtocolRx: protected _SWProtocolBase {
    protected:
        uint32_t pin;
        _SWProtocolRx(uint32_t rxpin): pin(rxpin) {};
        bool verifyCRC();
    public:
        bool getByte(uint8_t idx, uint8_t *value);
};


class _SWRoaster: public _SWProtocolBase {
    protected:
        _SWRoaster(): _SWProtocolBase(bufMemory, MESSAGE_LENGTH_ROASTER) {};
        uint8_t bufMemory[MESSAGE_LENGTH_ROASTER];
};


class _SWController: public _SWProtocolBase {
    protected:
        _SWController(): _SWProtocolBase(bufMemory, MESSAGE_LENGTH_CONTROLLER) {};
        uint8_t bufMemory[MESSAGE_LENGTH_CONTROLLER];
};


class SWRoasterRx: public _SWRoaster, public _SWProtocolRx {
    public:
        SWRoasterRx(): _SWProtocolRx(CONTROLLER_PIN_RX) {};
};


class SWControllerTx: public _SWController , public _SWProtocolTx{
    public:
        SWControllerTx(): _SWProtocolTx(CONTROLLER_PIN_TX) {};
        void begin();
};



#endif  // __SKYWALKER_PROTOCOL_H