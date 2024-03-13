#ifndef LWESP_CAN_H
#define LWESP_CAN_H

#include "mqtt_message.h"

typedef enum
{
	UNCONFIGURED = 0x0,
	CONNECTED = 0x1,
	NOT_CONNECTED = 0x2
} ConStatus;

typedef struct
{
	uint32_t ID;
	uint32_t lastHeartbeat;
	ConStatus status;
	DeviceConfig config;
} Device;

Device* find_device(uint32_t ID);

void CAN_SendSensorConfig(MqttMessageConfig config);

#endif