#ifndef _SW_STATE_H
#define _SW_STATE_H

typedef struct {
  uint8_t heat;
  uint8_t vent;
  uint8_t cool;
  uint8_t filter;
  uint8_t drum;
} t_StateCommanded;

typedef struct {
    double chanTemp[TEMPERATURE_CHANNELS_NUMBER]; // Physical temp channels
} t_StateReported;

typedef struct {
    uint8_t chanMapping[TEMPERATURE_CHANNELS_NUMBER];
    char CorF;
} t_Config;

typedef struct {
    uint32_t tc4LastTick;
    bool isSynchronized;
#ifdef USE_THERMOCOUPLE
    uint8_t tcStatus;
#endif
} t_Status;

typedef struct {
    t_StateCommanded    commanded;
    t_StateReported     reported;
    t_Config            cfg;
    t_Status            status;
} t_State;

#endif  // _SW_STATE_H