#ifndef EWEMOSSENSOR_CAN_H
#define EWEMOSSENSOR_CAN_H

#include "data_types.h"
#include "stm32l5xx_hal.h"

extern CANMessage TxMessage;
extern FDCAN_TxHeaderTypeDef TxHeader;
extern FDCAN_HandleTypeDef hfdcan1;

void CAN_SendHeartbeat(uint32_t ID);

void CAN_SendData(uint32_t ID, DataType type, const void* data);

#endif