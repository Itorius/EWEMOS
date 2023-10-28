//#include <memory.h>
//#include <stdio.h>
//#include "AT.h"
//#include "stm32l5xx_hal.h"
//#include "main.h"
//
//extern UART_HandleTypeDef huart2;;
//
//#define MAX_BUFFER 256
//volatile char ringBuffer[MAX_BUFFER];
//volatile uint8_t pos = 0;
//
//int TransmitCommand(const char* command, const char* response)
//{
//	HAL_UART_Transmit(&huart2, (uint8_t*)command, strlen(command), HAL_MAX_DELAY);
//
//	pos = 0;
//	while (!strstr((char*)ringBuffer, response) && HAL_UART_Receive(&huart2, (uint8_t*)ringBuffer + (pos++), 1, 10000) == HAL_OK)
//	{
//
//	}
//
//	int success = strstr((char*)ringBuffer, response) != NULL;
//	memset((uint8_t*)ringBuffer, 0, MAX_BUFFER);
//	return success;
//}
//
//int TransmitCommandOK(const char* command)
//{
//	return TransmitCommand(command, "\r\nOK\r\n");
//}
//
//void ATInit()
//{
//	TransmitCommandOK("ATE0\r\n");
//
//	if (!TransmitCommandOK("AT\r\n"))
//	{
//	}
//
//	if (!TransmitCommandOK("AT+GMR\r\n"))
//	{
//	}
//}
//
//void WifiInit()
//{
//	TransmitCommandOK("AT+CWMODE=1,0\r\n");
//
//	if (!TransmitCommandOK("AT+CWJAP=\"SauronNet.Horcicka1\",\"10821082\"\r\n"))
//	{
//	}
//
//	if (!TransmitCommand("AT+CIPSNTPCFG=1,2,\"si.pool.ntp.org\",\"time.google.com\",\"us.pool.ntp.org\"\r\n", "+TIME_UPDATED"))
//	{
//	}
//
//	if (!TransmitCommandOK("AT+CIPSNTPTIME?\r\n"))
//	{
//	}
//}
//
//extern const char* MCUCode;
//
//void MQTTInit()
//{
//	char str[80];
//	memset(str, 0, 80);
//	sprintf(str, "AT+MQTTUSERCFG=0,5,\"%s\",\"test\",\"test\",0,0,\"\"\r\n", MCUCode);
//
//	if (!TransmitCommandOK(str))
//	{
//	}
//
//	if (!TransmitCommandOK("AT+MQTTCONN=0,\"itorius.com\",8883,1\r\n"))
//	{
//	}
//}