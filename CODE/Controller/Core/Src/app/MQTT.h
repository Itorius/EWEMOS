#ifndef LWESP_MQTT_H
#define LWESP_MQTT_H

#include "CAN.h"
#include "lwesp/apps/lwesp_mqtt_client.h"

void MQTT_SendStatusChange(const Device* device, ConStatus status);

void MQTT_SendData(const Device* device, void* data);

void MQTT_SendName(char* name);

void MQTT_SendConfig(const Device* device);

#endif