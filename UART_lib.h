#include "stm32f10x_lib.h"
#define BUFSIZE 128
void UART_NVIC_Configuration(void);
void UART_GPIO_Configuration(void);
void USART_Configuration(void);
int UARTRead(char* buffer);
void UARTSend(const  char* pucBuffer, int ulCount);
void Uart_Init_Setting(void);
void UARTGet(void);
int UART_RxAvailable(void);
void StringCopy(char* destination, char* source, int size);
void MemCopy(char* destination, char* source, int length );