#ifndef LWESP_CAN_H
#define LWESP_CAN_H

#include "mqtt_message.h"

typedef struct
{
	uint32_t ID;
	uint32_t lastHeartbeat;
	ConnectionStatus status;
	DeviceConfig config;
} Device;

Device* find_device(uint32_t ID);

void CAN_SendSensorConfig(MqttMessageConfig config);

#endif