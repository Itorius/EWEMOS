#ifndef EWEMOSSENSOR_MAX6675_H
#define EWEMOSSENSOR_MAX6675_H

#include "stm32l5xx_hal.h"

float MAX6675_Read_Temp(SPI_HandleTypeDef* hspi1, GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin);

#endif //EWEMOSSENSOR_MAX6675_H
