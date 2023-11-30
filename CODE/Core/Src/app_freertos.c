/* USER CODE BEGIN Header */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "lwesp/lwesp_includes.h"
#include "lwesp/lwesp.h"
#include "lwesp/apps/lwesp_mqtt_client_api.h"
#include "base64.h"
#include "i2c.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */
static lwesp_mqtt_client_p mqtt_client;
static char data[80];

char ID[17];

static struct DeviceConfig
{
	char Name[64];
} Config __attribute__((aligned(16)));

static char nameconfig[16 + 13];

#define EEPROM_ADDRESS 0xA0
#define CONFIG_ADDRESS 0

/* USER CODE END Variables */
/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
	.name = "defaultTask",
	.priority = (osPriority_t)osPriorityNormal,
	.stack_size = 128 * 4
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */
void init_thread(void* argument);

/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void* argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void)
{
	/* USER CODE BEGIN Init */
	base64_encode((char*)UID_BASE, 12, ID);
	HAL_I2C_Mem_Read(&hi2c2, EEPROM_ADDRESS, CONFIG_ADDRESS, I2C_MEMADD_SIZE_8BIT, (uint8_t*)&Config, sizeof(Config), 1000);
	/* USER CODE END Init */

	/* USER CODE BEGIN RTOS_MUTEX */
	/* add mutexes, ... */
	/* USER CODE END RTOS_MUTEX */

	/* USER CODE BEGIN RTOS_SEMAPHORES */
	/* add semaphores, ... */
	/* USER CODE END RTOS_SEMAPHORES */

	/* USER CODE BEGIN RTOS_TIMERS */
	/* start timers, add new ones, ... */
	/* USER CODE END RTOS_TIMERS */

	/* USER CODE BEGIN RTOS_QUEUES */
	/* add queues, ... */
	/* USER CODE END RTOS_QUEUES */

	/* Create the thread(s) */
	/* creation of defaultTask */
	defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

	/* USER CODE BEGIN RTOS_THREADS */
	const osThreadAttr_t attr = {
		.stack_size = 512
	};
	osThreadNew(init_thread, NULL, &attr);
	/* USER CODE END RTOS_THREADS */

	/* USER CODE BEGIN RTOS_EVENTS */
	/* add events, ... */
	/* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void* argument)
{
	/* USER CODE BEGIN StartDefaultTask */
	/* Infinite loop */
//	uint32_t i = 0;
	for (;;)
	{
		HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);

		if (mqtt_client != NULL && lwesp_mqtt_client_is_connected(mqtt_client))
		{
//			sprintf(data, "Current cycle %lu\n", i);
//			i++;
//			if (strlen(Config.Name) <= 0)
//			{
//				char* n = "Device name not set";
//				lwesp_mqtt_client_publish(mqtt_client, "/test_data", n, strlen(n), LWESP_MQTT_QOS_AT_MOST_ONCE, 0, (void*)1);
//
//			}
//			else
//			{
//				lwesp_mqtt_client_publish(mqtt_client, "/test_data", Config.Name, strlen(Config.Name), LWESP_MQTT_QOS_AT_MOST_ONCE, 0, (void*)1);
//			}
		}

		osDelay(500);
	}
	/* USER CODE END StartDefaultTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */
lwespr_t init_callback(lwesp_evt_t* evt)
{
	switch (lwesp_evt_get_type(evt))
	{
		case LWESP_EVT_AT_VERSION_NOT_SUPPORTED:
		{
			lwesp_sw_version_t v_min, v_curr;

			lwesp_get_min_at_fw_version(&v_min);
			lwesp_get_current_at_fw_version(&v_curr);

			printf("Current ESP[8266/32[-C3]] AT version is not supported by the library\r\n");
			printf("Minimum required AT version is: %08X\r\n", (unsigned)v_min.version);
			printf("Current AT version is: %08X\r\n", (unsigned)v_curr.version);
			break;
		}
		case LWESP_EVT_INIT_FINISH:
		{
			printf("Library initialized!\r\n");
			break;
		}
		case LWESP_EVT_RESET_DETECTED:
		{
			printf("Device reset detected!\r\n");
			break;
		}
		default:
			break;
	}
	return lwespOK;
}

static lwesp_mqtt_client_info_t mqtt_client_info = {
	.id = "EWEMOS",
//	.user = "test",
//	.pass = "test",
	.keep_alive = 60
//	.use_ssl = 1
};

static void mqtt_connect_callback(lwesp_mqtt_client_p client, lwesp_mqtt_evt_t* evt)
{
	lwesp_mqtt_evt_type_t evt_type = lwesp_mqtt_client_evt_get_type(client, evt);
	switch (evt_type)
	{
		case LWESP_MQTT_EVT_CONNECT:
		{
			lwesp_mqtt_conn_status_t status = lwesp_mqtt_client_evt_connect_get_status(client, evt);

			if (status == LWESP_MQTT_CONN_STATUS_ACCEPTED)
			{
				printf("MQTT accepted!\r\n");

				lwesp_mqtt_client_publish(client, nameconfig, Config.Name, 64, LWESP_MQTT_QOS_EXACTLY_ONCE, 0, 0);
				lwesp_mqtt_client_subscribe(client, nameconfig, LWESP_MQTT_QOS_AT_MOST_ONCE, nameconfig);
			}
			else
			{
				printf("MQTT server connection was not successful: %d\r\n", (int)status);
			}
			break;
		}
		case LWESP_MQTT_EVT_SUBSCRIBE:
		{
			const char* arg = lwesp_mqtt_client_evt_subscribe_get_argument(client, evt);
			lwespr_t res = lwesp_mqtt_client_evt_subscribe_get_result(client, evt);

			if (res == lwespOK)
			{
				printf("Successfully subscribed to %s topic\r\n", arg);
			}
			break;
		}
		case LWESP_MQTT_EVT_PUBLISH_RECV:
		{
			const char* topic = lwesp_mqtt_client_evt_publish_recv_get_topic(client, evt);
			size_t topic_len = lwesp_mqtt_client_evt_publish_recv_get_topic_len(client, evt);
			const uint8_t* payload = lwesp_mqtt_client_evt_publish_recv_get_payload(client, evt);
			size_t payload_len = lwesp_mqtt_client_evt_publish_recv_get_payload_len(client, evt);

			memset(Config.Name, 0, 64);
			memcpy(Config.Name, payload, LWESP_MIN(payload_len, 64));
			for (int i = 0; i < 4; ++i)
			{
				HAL_I2C_Mem_Write(&hi2c2, EEPROM_ADDRESS, 16 * i, I2C_MEMADD_SIZE_8BIT, ((uint8_t*)Config.Name) + 16 * i, 16, 1000);
				osDelay(5);
			}

//			osDelay(500);
//			uint8_t test[64];
//			memset(test, 0, 64);
//			HAL_I2C_Mem_Read(&hi2c2, EEPROM_ADDRESS, 0x00, I2C_MEMADD_SIZE_8BIT, (uint8_t*)test, 64, 1000);
//			printf("fesfe");
			break;
		}
		case LWESP_MQTT_EVT_PUBLISH:
		{
			uint32_t val = (uint32_t)(uintptr_t)lwesp_mqtt_client_evt_publish_get_argument(client, evt);

			printf("Publish event, user argument on message was: %d\r\n", (int)val);
			break;
		}
		case LWESP_MQTT_EVT_DISCONNECT:
		{
			printf("MQTT client disconnected!\r\n");
			break;
		}
		default:
			break;
	}
}

void sta_join_callback(lwespr_t status, void* arg)
{
	lwesp_ip_t ip = { };
	lwesp_sta_copy_ip(&ip, NULL, NULL, NULL);
	mqtt_client = lwesp_mqtt_client_new(256, 128);;

	mqtt_client_info.id = ID;

	sprintf(data, "/%s/config/name", ID);
	memccpy(nameconfig, data, 0, strlen(data));

	lwesp_mqtt_client_connect(mqtt_client, "192.168.1.16", 1883, mqtt_connect_callback, &mqtt_client_info);
}

void init_thread(void* arg)
{
	printf("Initializing LwESP\r\n");
	if (lwesp_init(init_callback, 1) != lwespOK)
	{
		printf("Cannot initialize LwESP!\r\n");
	}
	else
	{
		printf("LwESP initialized!\r\n");
	}

	while (1)
	{
		if (!lwesp_sta_is_joined())
		{
			if (lwesp_sta_join("SauronNet.Horcicka1", "10821082", NULL, sta_join_callback, 0, 0) == lwespOK)
			{
			}
		}

		osDelay(5000);
	}
}
/* USER CODE END Application */

