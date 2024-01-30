#ifndef __ROASTER_H
#define __ROASTER_H

#define ROASTER_PREAMBLE_LENGTH_US          7000U
#define ROASTER_ONE_LENGTH_US               1200U
#define ROASTER_MESSAGE_LENGTH              7U
#define ROASTER_CONTROLLER_MESSAGE_LENGTH   6U

// Roaster message structure
#define ROASTER_MESSAGE_BYTE_VENT           0
#define ROASTER_MESSAGE_BYTE_FILTER         1
#define ROASTER_MESSAGE_BYTE_COOL           2
#define ROATER_MESSAGE_BYTE_DRUM            3
#define ROASTER_MESSAGE_BYTE_HEAT           4
#define ROASTER_MESSAGE_BYTE_CRC            5

// TC4 protocol communication timeout in milliseconds
#define TC4_COMM_TIMEOUT_MS                 10000UL

// declare sync loss if fail to receive message n times
#define MESSAGE_RX_MAX_ATTEMPTS             5
// number of attempts to receive a preamble
#define MESSAGE_PREAMBLE_MAX_ATTEMPTS       60

// max number of temp channels
#define TEMPERATURE_CHANNELS_MAX            4
// physical channel for thermocouple temperature
#define TEMPERATURE_CHANNEL_THERMOCOUPLE    0
// physical channel for temperature reported by roaster
#define TEMPERATURE_CHANNEL_ROASTER         1

#define TEMPERATURE_ROASTER chanTempPhysical[TEMPERATURE_CHANNEL_ROASTER]
#define TEMPERATURE_TC      chanTempPhysical[TEMPERATURE_CHANNEL_THERMOCOUPLE]

typedef unsigned long ustick_t;

#ifdef __DEBUG__
#define DEBUG(...) Serial.print(__VA_ARGS__)
#define DEBUGLN(...) Serial.println(__VA_ARGS__)
#else
#define DEBUG(...)
#define DEBUGLN(...)
#endif
#ifdef __WARN__
#define WARN(...) Serial.print(__VA_ARGS__)
#define WARNLN(...) Serial.println(__VA_ARGS__)
#else
#define WARN(...)
#define WARNLN(...)
#endif

#endif  // __ROASTER_H