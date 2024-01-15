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

#endif  // __ROASTER_H