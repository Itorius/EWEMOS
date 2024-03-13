#ifndef LWESP_MQTT_MESSAGE_H
#define LWESP_MQTT_MESSAGE_H

#include <stdint-gcc.h>
#include "data_types.h"

typedef struct
{
	char Name[64];
	uint32_t ConfigSet;
} MqttMessageName __attribute__((aligned(4)));

typedef struct
{
	uint32_t ID;
	SensorType Type;
	uint16_t ConfigSet;
	uint32_t Interval;
	char Name[32];
} MqttMessageConfig __attribute__((aligned(4)));

typedef struct
{
	uint32_t ID;
	uint32_t State;
} MqttMessageState __attribute__((aligned(4)));

typedef struct
{
	uint32_t ID;
	uint8_t Data[56];
} MqttMessageData __attribute__((aligned(4)));

#endif