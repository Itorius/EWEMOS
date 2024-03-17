#ifndef LWESP_MQTT_MESSAGE_H
#define LWESP_MQTT_MESSAGE_H

#include <stdint-gcc.h>
#include "data_types.h"

// Total size 68 bytes
typedef struct
{
	char Name[64];
	uint32_t ConfigSet;
} MqttMessageName __attribute__((aligned(4)));

// Total size 44 bytes
typedef struct
{
	uint32_t ID;
	SensorType Type;
	uint16_t ConfigSet;
	uint32_t Interval;
	char Name[32];
} MqttMessageConfig __attribute__((aligned(4)));

// Total size 8 bytes
typedef struct
{
	uint32_t ID;
	ConnectionStatus State;
} MqttMessageState __attribute__((aligned(4)));

// Total size 60 bytes
typedef struct
{
	uint32_t ID;
	DataType Type;
	uint16_t Length;
	uint8_t Data[52];
} MqttMessageData __attribute__((aligned(4)));

#endif