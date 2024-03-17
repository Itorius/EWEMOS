/* USER CODE BEGIN Header */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "lwesp/lwesp_includes.h"
#include "lwesp/lwesp.h"
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

char ID[16];

struct DeviceConfig
{
	char Name[64];
} Config __attribute__((aligned(4)));

#define EEPROM_ADDRESS 0xA0
#define CONFIG_ADDRESS 0

/* USER CODE END Variables */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */
void init_thread(void* argument);

/* USER CODE END FunctionPrototypes */

extern void CANTask(void* argument);

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

	if (HAL_I2C_Mem_Read(&hi2c2, EEPROM_ADDRESS, CONFIG_ADDRESS, I2C_MEMADD_SIZE_8BIT, (uint8_t*)&Config, 16, 1000) != HAL_OK)
	{
		char* name = "NAME_ERROR";
		memcpy(Config.Name, name, strlen(name));
		printf("Failed to read config from EEPROM\n");
	}
	else printf("Sucessfully read config from EEPROM\n");

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

	/* USER CODE BEGIN RTOS_THREADS */
	const osThreadAttr_t attr = {
		.name = "Init task",
		.priority = (osPriority_t)osPriorityNormal,
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
//void StartDefaultTask(void* argument)
//{
//	/* USER CODE BEGIN StartDefaultTask */
//	/* Infinite loop */
//	for (;;)
//	{
////		if (mqtt_client != NULL && lwesp_mqtt_client_is_connected(mqtt_client))
////		{
////		}
//
//		osDelay(1000);
//	}
//	/* USER CODE END StartDefaultTask */
//}

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

extern void mqtt_init();

void sta_join_callback(lwespr_t status, void* arg)
{
	lwesp_ip_t ip = {};
	lwesp_sta_copy_ip(&ip, NULL, NULL, NULL);
	printf("Joined Wi-Fi with IP %d:%d:%d:%d\n", ip.addr.ip4.addr[0], ip.addr.ip4.addr[1], ip.addr.ip4.addr[2], ip.addr.ip4.addr[3]);

	mqtt_init();
}

extern void can_init();

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

	can_init();

	while (1)
	{
		if (!lwesp_sta_is_joined())
		{
			printf("Attempting to join Wi-Fi\r\n");
			if (lwesp_sta_join("SauronNet.Horcicka1", "10821082", NULL, sta_join_callback, 0, 1) == lwespOK)
			{
			}
		}

		osDelay(5000);
	}
}

int __io_putchar(int ch)
{
	ITM_SendChar(ch);
	return ch;
}
/* USER CODE END Application */