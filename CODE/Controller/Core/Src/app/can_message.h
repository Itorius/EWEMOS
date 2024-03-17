#ifndef LWESP_CAN_MESSAGE_H
#define LWESP_CAN_MESSAGE_H

#include <stdint-gcc.h>
#include "data_types.h"

typedef enum
{
	CONF_REQUEST = 0x00,
	CONF_ACK = 0x01,
	CONF_DATA = 0x02,
	DATA = 0x03,
	HEARTBEAT = 0x04,
	CANMessageType_MAX = UINT32_MAX
} CANMessageType;

typedef struct
{
	DataType type;
	uint16_t dataLength;
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