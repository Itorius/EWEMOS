#ifndef EWEMOSSENSOR_DATA_TYPES_H
#define EWEMOSSENSOR_DATA_TYPES_H

#include <stdint-gcc.h>

typedef enum
{
	Temperature = 0,
	Pressure = 1,
	Humidity = 2,
	SensorType_MAX = UINT16_MAX
} SensorType;

typedef enum
{
	FLOAT = 1,
	UINT32 = 2,
	DataType_MAX = UINT16_MAX
} DataType;

typedef enum
{
	UNCONFIGURED = 0,
	CONNECTED = 1,
	NOT_CONNECTED = 2,
	ConnectionStatus_MAX = UINT32_MAX
} ConnectionStatus;

typedef struct
{
	char Name[32];
	SensorType Type;
	uint32_t Interval;
} DeviceConfig __attribute__((aligned(4)));

#endif