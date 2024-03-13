#include "cmsis_os2.h"
#include "i2c.h"

#define EEPROM_ADR 0xA0
#define EEPROM_I2C &hi2c2
#define PAGE_SIZE 16
#define MIN(a, b) (((a)<(b))?(a):(b))

void EEPROM_Write(uint16_t address, uint8_t* data, uint16_t size)
{
	uint16_t page = address / PAGE_SIZE;
	uint16_t offset = address - page * PAGE_SIZE;
	uint16_t remainingLength = size;

	while (remainingLength > 0)
	{
		uint32_t toWrite = MIN(remainingLength, PAGE_SIZE - offset);
		HAL_I2C_Mem_Write(EEPROM_I2C, EEPROM_ADR, address, I2C_MEMADD_SIZE_8BIT, data, toWrite, 500);
		osDelay(5);
		offset = 0;
		address += toWrite;
		data += toWrite;
		remainingLength -= toWrite;
	}
}