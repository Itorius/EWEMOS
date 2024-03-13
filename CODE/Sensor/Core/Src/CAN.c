#include <memory.h>
#include "CAN.h"

void CAN_SendHeartbeat(uint32_t ID)
{
	TxHeader.Identifier = *(uint8_t*)&ID;
	TxHeader.DataLength = FDCAN_DLC_BYTES_8;

	TxMessage.ID = ID;
	TxMessage.type = HEARTBEAT;

	HAL_FDCAN_AddMessageToTxFifoQ(&hfdcan1, &TxHeader, (uint8_t*)&TxMessage);
}

void CAN_SendData(uint32_t ID, DataType type, const void* data)
{
	TxHeader.Identifier = *(uint8_t*)&ID;
	TxHeader.DataLength = FDCAN_DLC_BYTES_16;

//	memset(&TxMessage, 0, sizeof(TxMessage));
	TxMessage.ID = ID;
	TxMessage.type = DATA;
	TxMessage.data.dataBlob.type = type;
	memcpy(TxMessage.data.dataBlob.data, data, sizeof(data));

	HAL_FDCAN_AddMessageToTxFifoQ(&hfdcan1, &TxHeader, (uint8_t*)&TxMessage);
}