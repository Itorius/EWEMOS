#include <stdio.h>
#include <math.h>
#include "fdcan.h"
#include "FreeRTOS.h"
#include "cmsis_os.h"
#include "lwesp/apps/lwesp_mqtt_client.h"
#include "CAN.h"
#include "MQTT.h"

FDCAN_RxHeaderTypeDef RxHeader;
FDCAN_TxHeaderTypeDef TxHeader;

CANMessage RxMessage;
CANMessage TxMessage;

volatile uint8_t deviceIndex = 0;
volatile Device devices[64];

void CANTask(void* arg);

osThreadId_t can_task;

void can_init()
{
	const osThreadAttr_t cantask_attr = {
		.name = "CAN task",
		.priority = (osPriority_t)osPriorityNormal,
		.stack_size = 1024
	};
	can_task = osThreadNew(CANTask, NULL, &cantask_attr);

	TxHeader.IdType = FDCAN_STANDARD_ID;
	TxHeader.TxFrameType = FDCAN_DATA_FRAME;
	TxHeader.ErrorStateIndicator = FDCAN_ESI_ACTIVE;
	TxHeader.BitRateSwitch = FDCAN_BRS_OFF;
	TxHeader.FDFormat = FDCAN_FD_CAN;
	TxHeader.TxEventFifoControl = FDCAN_NO_TX_EVENTS;
	TxHeader.MessageMarker = 0;

	TxHeader.Identifier = 0x01;
	TxHeader.DataLength = FDCAN_DLC_BYTES_8;

	TxMessage.ID = 0;
	TxMessage.type = CONF_REQUEST;

	HAL_FDCAN_AddMessageToTxFifoQ(&hfdcan1, &TxHeader, (uint8_t*)&TxMessage);
}

void ProcessMessage()
{
	switch (RxMessage.type)
	{
		case CONF_DATA:
		{
			volatile Device* device;
			if (!(device = find_device(RxMessage.ID)))
			{
				device = &devices[deviceIndex++];
				device->ID = RxMessage.ID;
			}

			device->config = RxMessage.data.config;
			device->status = CONNECTED;
			device->lastHeartbeat = osKernelGetTickCount() / portTICK_PERIOD_MS;

			printf("#GRN#Received config from %lu\n", device->ID);

			TxHeader.Identifier = 0x01;
			TxHeader.DataLength = FDCAN_DLC_BYTES_8;

			TxMessage.ID = RxMessage.ID;
			TxMessage.type = CONF_ACK;

			HAL_FDCAN_AddMessageToTxFifoQ(&hfdcan1, &TxHeader, (uint8_t*)&TxMessage);

			MQTT_SendConfig(device);

			break;
		}
		case HEARTBEAT:
		{
			Device* device;
			if ((device = find_device(RxMessage.ID)))
			{
				MQTT_SendStatusChange(device, CONNECTED);

				device->lastHeartbeat = osKernelGetTickCount() / portTICK_PERIOD_MS;
				device->status = CONNECTED;

				// note: maybe also heartbeat_ack so sensor knows the controller is online?

				printf("#GRN#Received heartbeat from %lu\n", device->ID);
			}

			break;
		}
		case DATA:
		{
			Device* device;
			if ((device = find_device(RxMessage.ID)))
			{
				MQTT_SendData(device, &RxMessage.data.dataBlob);
			}

			break;
		}
		default:
			break;
	}
}

void CANTask(void* argument)
{
	int period_ms = 100;
	TimeOut_t timeout;
	TickType_t period_tick = pdMS_TO_TICKS(period_ms);
	vTaskSetTimeOutState(&timeout);

	for (;;)
	{
		if (xTaskCheckForTimeOut(&timeout, &period_tick) != pdFALSE)
		{
			vTaskSetTimeOutState(&timeout);
			period_tick = pdMS_TO_TICKS(period_ms);

			for (int i = 0; i < deviceIndex; ++i)
			{
				if (devices[i].status != CONNECTED)
					continue;

				if (osKernelGetTickCount() - devices[i].lastHeartbeat > 5 * devices[i].config.Interval)
//				if (osKernelGetTickCount() - devices[i].lastHeartbeat > 5000)
				{
					MQTT_SendStatusChange(&devices[i], NOT_CONNECTED);

					devices[i].status = NOT_CONNECTED;

					printf("#RED#Failed to receive heartbeat from %lu, last heartbeat at %lu seconds\n", devices[i].ID, devices[i].lastHeartbeat / 1000);
				}
				else
				{
				}
			}
		}
	}
}

void CAN_SendSensorConfig(MqttMessageConfig config)
{
	Device* device;
	if ((device = find_device(RxMessage.ID)))
	{
		device->config.Interval = config.Interval;
		memcpy(device->config.Name, config.Name, 32);

		TxHeader.Identifier = 0x01;
		TxHeader.DataLength = FDCAN_DLC_BYTES_48;

		TxMessage.ID = config.ID;
		TxMessage.type = CONF_DATA;
		TxMessage.data.config = device->config;

		HAL_FDCAN_AddMessageToTxFifoQ(&hfdcan1, &TxHeader, (uint8_t*)&TxMessage);

		MQTT_SendConfig(device);
	}
}

void HAL_FDCAN_RxFifo0Callback(FDCAN_HandleTypeDef* hfdcan, uint32_t RxFifo0ITs)
{
	if ((RxFifo0ITs & FDCAN_IT_RX_FIFO0_NEW_MESSAGE) != RESET)
	{
		/* Retrieve Rx messages from RX FIFO0 */
		if (HAL_FDCAN_GetRxMessage(hfdcan, FDCAN_RX_FIFO0, &RxHeader, (uint8_t*)&RxMessage) != HAL_OK)
		{
			/* Reception Error */
			Error_Handler();
		}

		ProcessMessage();

		if (HAL_FDCAN_ActivateNotification(hfdcan, FDCAN_IT_RX_FIFO0_NEW_MESSAGE, 0) != HAL_OK)
		{
			/* Notification Error */
			Error_Handler();
		}
	}
}

Device* find_device(uint32_t id)
{
	for (uint8_t i = 0; i < 64; ++i)
	{
		if (devices[i].status != UNCONFIGURED && devices[i].ID == id)
		{
			return &devices[i];
		}
	}

	return 0;
}