#ifndef LWESP_MQTT_H
#define LWESP_MQTT_H

#include "can_message.h"

void MQTT_SendStatusChange(const Device* device, ConnectionStatus status);

void MQTT_SendData(const Device* device, const DataBlob* dataBlob);

void MQTT_SendName(char* name);

void MQTT_SendConfig(const Device* device);

#endif