#include "lwesp/lwesp.h"
#include "lwesp/lwesp_input.h"
#include "system/lwesp_ll.h"

#include "stm32l5xx_ll_bus.h"
#include "stm32l5xx_ll_dma.h"
#include "stm32l5xx_ll_gpio.h"
#include "stm32l5xx_ll_usart.h"

#define LWESP_USART                   USART2
#define LWESP_USART_IRQ               USART2_IRQn
#define LWESP_USART_IRQHANDLER        USART2_IRQHandler

#define LWESP_USART_DMA               DMA1
#define LWESP_USART_DMA_RX_IRQ        DMA1_Channel6_IRQn
#define LWESP_USART_DMA_RX_IRQHANDLER DMA1_Channel6_IRQHandler
#define LWESP_USART_DMA_RX_CH         LL_DMA_CHANNEL_6

#if !defined(LWESP_USART_DMA_RX_BUFF_SIZE)
#define LWESP_USART_DMA_RX_BUFF_SIZE 0x1000
#endif

#if !defined(LWESP_MEM_SIZE)
#define LWESP_MEM_SIZE 0x4000
#endif

static uint8_t usart_mem[LWESP_USART_DMA_RX_BUFF_SIZE];
static uint8_t is_running, initialized;
static size_t old_pos;

static void usart_ll_thread(void* arg);

static osThreadId_t usart_ll_thread_id;
static osMessageQueueId_t usart_ll_mbox_id;

static void usart_ll_thread(void* arg)
{
	size_t pos;

	LWESP_UNUSED(arg);

	while (1)
	{
		void* d;
		osMessageQueueGet(usart_ll_mbox_id, &d, NULL, osWaitForever);

		pos = sizeof(usart_mem) - LL_DMA_GetDataLength(LWESP_USART_DMA, LWESP_USART_DMA_RX_CH);
		if (pos != old_pos && is_running)
		{
			if (pos > old_pos)
			{
				lwesp_input_process(&usart_mem[old_pos], pos - old_pos);
			}
			else
			{
				lwesp_input_process(&usart_mem[old_pos], sizeof(usart_mem) - old_pos);
				if (pos > 0)
				{
					lwesp_input_process(&usart_mem[0], pos);
				}
			}
			old_pos = pos;
		}
	}
}

static void prv_configure_uart(uint32_t baudrate)
{
	if (!initialized)
	{
		LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_USART2);
		LL_AHB2_GRP1_EnableClock(LL_AHB2_GRP1_PERIPH_GPIOA);
		LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_DMAMUX1);
		LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_DMA1);

		NVIC_SetPriority(DMAMUX1_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 0, 0));
		NVIC_EnableIRQ(DMAMUX1_IRQn);

#pragma region GPIO
		LL_GPIO_InitTypeDef gpio_init = {
			.Pin = LL_GPIO_PIN_2 | LL_GPIO_PIN_3,
			.Alternate = LL_GPIO_AF_7,
			.Mode = LL_GPIO_MODE_ALTERNATE,
			.OutputType = LL_GPIO_OUTPUT_PUSHPULL,
			.Pull = LL_GPIO_PULL_UP,
			.Speed = LL_GPIO_SPEED_FREQ_VERY_HIGH
		};
		LL_GPIO_Init(GPIOA, &gpio_init);
#pragma endregion GPIO

#pragma region UART
		RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};
		PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART2;
		PeriphClkInit.Usart2ClockSelection = RCC_USART2CLKSOURCE_PCLK1;
		if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
		{
//			Error_Handler();
		}

		LL_USART_DeInit(LWESP_USART);

		LL_USART_InitTypeDef usart_init = {
			.BaudRate = baudrate,
			.TransferDirection = LL_USART_DIRECTION_TX_RX,
			.DataWidth = LL_USART_DATAWIDTH_8B,
			.StopBits = LL_USART_STOPBITS_1,
			.Parity = LL_USART_PARITY_NONE,
			.OverSampling = LL_USART_OVERSAMPLING_16,
			.PrescalerValue = LL_USART_PRESCALER_DIV1,
			.HardwareFlowControl = LL_USART_HWCONTROL_NONE
		};
		LL_USART_Init(LWESP_USART, &usart_init);
		LL_USART_ConfigAsyncMode(LWESP_USART);
		LL_USART_DisableFIFO(LWESP_USART);
		LL_USART_EnableDMAReq_RX(LWESP_USART);
		LL_USART_EnableIT_IDLE(LWESP_USART);

		NVIC_SetPriority(LWESP_USART_IRQ, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 5, 1));
		NVIC_EnableIRQ(LWESP_USART_IRQ);


#pragma endregion UART

#pragma region DMA
		LL_DMA_InitTypeDef dma_init = {
			.PeriphRequest = LL_DMAMUX_REQ_USART2_RX,
			.Direction = LL_DMA_DIRECTION_PERIPH_TO_MEMORY,
			.Priority = LL_DMA_PRIORITY_LOW,
			.Mode = LL_DMA_MODE_CIRCULAR,
			.PeriphOrM2MSrcIncMode = LL_DMA_PERIPH_NOINCREMENT,
			.MemoryOrM2MDstIncMode = LL_DMA_MEMORY_INCREMENT,
			.PeriphOrM2MSrcDataSize = LL_DMA_PDATAALIGN_BYTE,
			.MemoryOrM2MDstDataSize = LL_DMA_PDATAALIGN_BYTE,
			.PeriphOrM2MSrcAddress = LL_USART_DMA_GetRegAddr(LWESP_USART, LL_USART_DMA_REG_DATA_RECEIVE),
			.MemoryOrM2MDstAddress = (uint32_t)usart_mem,
			.NbData = sizeof(usart_mem)
		};
		LL_DMA_Init(LWESP_USART_DMA, LWESP_USART_DMA_RX_CH, &dma_init);
		LL_DMA_EnableIT_HT(LWESP_USART_DMA, LWESP_USART_DMA_RX_CH);
		LL_DMA_EnableIT_TC(LWESP_USART_DMA, LWESP_USART_DMA_RX_CH);

		NVIC_SetPriority(LWESP_USART_DMA_RX_IRQ, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 5, 0));
		NVIC_EnableIRQ(LWESP_USART_DMA_RX_IRQ);
#pragma endregion DMA

		old_pos = 0;
		is_running = 1;

		LL_DMA_EnableChannel(LWESP_USART_DMA, LWESP_USART_DMA_RX_CH);
		LL_USART_Enable(LWESP_USART);
	}
	else
	{
//		osDelay(10);
//		LL_USART_Disable(LWESP_USART);
//		usart_init.BaudRate = baudrate;
//		LL_USART_Init(LWESP_USART, &usart_init);
//		LL_USART_Enable(LWESP_USART);
	}

	/* Create mbox and start thread */
	if (usart_ll_mbox_id == NULL)
	{
		usart_ll_mbox_id = osMessageQueueNew(10, sizeof(void*), NULL);
	}
	if (usart_ll_thread_id == NULL)
	{
		const osThreadAttr_t attr = { .stack_size = 1024 };
		usart_ll_thread_id = osThreadNew(usart_ll_thread, usart_ll_mbox_id, &attr);
	}
}

static size_t prv_send_data(const void* data, size_t len)
{
	const uint8_t* d = data;

	for (size_t i = 0; i < len; ++i, ++d)
	{
		LL_USART_TransmitData8(LWESP_USART, *d);
		while (!LL_USART_IsActiveFlag_TXE(LWESP_USART))
		{}
	}
	return len;
}

lwespr_t lwesp_ll_init(lwesp_ll_t* ll)
{
#if !LWESP_CFG_MEM_CUSTOM
	static uint8_t memory[LWESP_MEM_SIZE];
	const lwesp_mem_region_t mem_regions[] = { { memory, sizeof(memory) } };

	if (!initialized)
	{
		lwesp_mem_assignmemory(mem_regions, LWESP_ARRAYSIZE(mem_regions));
	}
#endif

	if (!initialized)
	{
		ll->send_fn = prv_send_data;
	}

	prv_configure_uart(ll->uart.baudrate);
	initialized = 1;
	return lwespOK;
}

lwespr_t lwesp_ll_deinit(lwesp_ll_t* ll)
{
	if (usart_ll_mbox_id != NULL)
	{
		osMessageQueueId_t tmp = usart_ll_mbox_id;
		usart_ll_mbox_id = NULL;
		osMessageQueueDelete(tmp);
	}
	if (usart_ll_thread_id != NULL)
	{
		osThreadId_t tmp = usart_ll_thread_id;
		usart_ll_thread_id = NULL;
		osThreadTerminate(tmp);
	}
	initialized = 0;
	LWESP_UNUSED(ll);
	return lwespOK;
}

void LWESP_USART_IRQHANDLER(void)
{
	void* d = (void*)1;

	if (LL_USART_IsEnabledIT_IDLE(USART2) && LL_USART_IsActiveFlag_IDLE(USART2))
	{
		LL_USART_ClearFlag_IDLE(USART2);
		osMessageQueuePut(usart_ll_mbox_id, &d, 0, 0);
	}
}

void LWESP_USART_DMA_RX_IRQHANDLER(void)
{
	void* d = (void*)1;

	if (LL_DMA_IsEnabledIT_HT(DMA1, LWESP_USART_DMA_RX_CH) && LL_DMA_IsActiveFlag_HT6(DMA1))
	{
		LL_DMA_ClearFlag_HT6(DMA1);
		osMessageQueuePut(usart_ll_mbox_id, &d, 0, 0);
	}

	/* Check transfer-complete interrupt */
	if (LL_DMA_IsEnabledIT_TC(DMA1, LWESP_USART_DMA_RX_CH) && LL_DMA_IsActiveFlag_TC6(DMA1))
	{
		LL_DMA_ClearFlag_TC6(DMA1);
		osMessageQueuePut(usart_ll_mbox_id, &d, 0, 0);
	}
}