#ifndef EWEMOSSENSOR_DATA_TYPES_H
#define EWEMOSSENSOR_DATA_TYPES_H

#include <stdint-gcc.h>

typedef enum
{
	Temperature = 0,
	Pressure = 1,
	Humidity = 2,
	SensorType_MAX = 0xFFFF
} SensorType;

typedef struct
{
	char Name[32];
	SensorType Type;
	uint32_t Interval;
} DeviceConfig __attribute__((aligned(4)));

typedef enum
{
	CONF_REQUEST = 0x00,
	CONF_ACK = 0x01,
	CONF_DATA = 0x02,
	DATA = 0x03,
	HEARTBEAT = 0x04,
	TESTMESSAGE = 0x100,
	CANMessageType_MAX = 0xFFFF
} CANMessageType;

typedef enum
{
	FLOAT = 1,
	UINT32 = 2,
	DataType_MAX = UINT32_MAX
} DataType;

typedef struct
{
	DataType type;
	uint8_t data[52];
} DataBlob __attribute__((aligned(4)));

typedef struct
{
	CANMessageType type;
	uint32_t ID;
	union
	{
		uint8_t bytes[56];
		DeviceConfig config;
		DataBlob dataBlob;
	} data;
} CANMessage;

#endif