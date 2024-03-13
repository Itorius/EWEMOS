#include "MAX6675.h"

float MAX6675_Read_Temp(SPI_HandleTypeDef* hspi1, GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin)
{
	HAL_GPIO_WritePin(GPIOx, GPIO_Pin, GPIO_PIN_RESET);

	uint16_t data = 0;
	HAL_SPI_Receive(hspi1, (uint8_t*)&data, 1, 50);

	HAL_GPIO_WritePin(GPIOx, GPIO_Pin, GPIO_PIN_SET);

	uint8_t TCF = (data & 0x4) >> 2;
	float temp = (float)((data & 0x7FF8) >> 3) * 0.25f;

	osDelay(250);
	return temp;
}
