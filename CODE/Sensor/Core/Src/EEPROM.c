#include "stm32l5xx_hal.h"

#define EEPROM_ADR 0xA0
#define PAGE_SIZE 16
#define MIN(a, b) (((a)<(b))?(a):(b))

void EEPROM_Write(I2C_HandleTypeDef* i2c, uint16_t address, uint8_t* data, uint16_t size)
{
	// bug: writes only one page
	uint16_t page = address / PAGE_SIZE;
	uint16_t offset = address - page * PAGE_SIZE;
	uint16_t remainingLength = size;

	while (remainingLength > 0)
	{
		uint32_t toWrite = MIN(remainingLength, PAGE_SIZE - offset);
		HAL_I2C_Mem_Write(i2c, EEPROM_ADR, address, I2C_MEMADD_SIZE_8BIT, data, toWrite, 500);
		HAL_Delay(5);
		offset = 0;
		address += toWrite;
		data += toWrite;
		remainingLength -= toWrite;
	}
}