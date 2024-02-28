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
    public:
        _SWProtocolBase(uint8_t *buffer, size_t bufferSize) : buffer(buffer), bufferSize(bufferSize) {};
        virtual void begin() {};
        bool verifyCRC();
        void setCRC();
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
    public:
        virtual void begin();
};


class SWRoasterRx: public _SWRoaster {
};


class SWControllerTx: public _SWController {
    public:
        void begin();
};



#endif  // __SKYWALKER_PROTOCOL_H