#include "UART_lib.h"


extern char MainBuffer[];
extern char ReceiveBuffer[];
extern int BufferValid;
extern int ReceiveLength;
extern int MainLength;

void Uart_Init_Setting(void)
{
	/* Enable USART1 and GPIOA clock */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 | RCC_APB2Periph_GPIOA, ENABLE);
	/* Configure the GPIOs */
	UART_GPIO_Configuration();
	/* Configure the USART1 */
	USART_Configuration();
	/* NVIC Configuration */
	UART_NVIC_Configuration();
}
void USART_Configuration(void)
{
	USART_InitTypeDef USART_InitStructure;

	/* USART1 configuration ------------------------------------------------------*/
	USART_InitStructure.USART_BaudRate = 9600;        // Baud Rate
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(USART1, &USART_InitStructure);
	/* Enable USART1 */
	USART_Cmd(USART1, ENABLE);
	/* Enable the USART1 Receive interrupt: this interrupt is generated when the
	 USART1 receive data register is not empty */
	// USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
}

void UART_NVIC_Configuration(void)
{
/*
	NVIC_InitTypeDef NVIC_InitStructure;

	//Enable the USARTx Interrupt 
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQChannel;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
*/
}
int UART_RxAvailable(void)
{
	if (USART_GetFlagStatus(USART1, USART_FLAG_RXNE) != RESET)// 수신 버퍼가 비어있지 않으면 // USART_IT_RXNE ??
		return TRUE;
	else
		return FALSE;
}
void UART_Send(const char* pucBuffer, int ulCount)
{
	while (ulCount--)
	{
		while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
		USART_SendData(USART1, (char)* pucBuffer++);
	}
}

void UART_Send_Char(u8 c)
{
  USART_SendData(USART1, c);
  while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
}

void UART_Get(void)
{
	char rx = (char)USART_ReceiveData(USART1);
	if ((rx == '\r') || (rx == '\n'))
	{
		if (ReceiveLength != 0) // Line has some content?
		{
			StringCopy(MainBuffer, ReceiveBuffer, ReceiveLength);
			// Copy to static line buffer from dynamic receive buffer
			MainBuffer[ReceiveLength] = 0; // Add terminating NUL
			BufferValid = 1; // flag new line valid for processing
			MainLength = ReceiveLength;
			ReceiveLength = 0; // Reset content pointer
		}
	}
	else
	{
		if (ReceiveLength == BUFSIZE) // If overflows pull back to start
			ReceiveLength = 0;
		ReceiveBuffer[ReceiveLength++] = rx; // Copy to buffer and increment
	}
}
void UART_GPIO_Configuration(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	/* Configure USART1 Tx (PA.09) as alternate function push-pull */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	/* Configure USART1 Rx (PA.10) as input floating */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
}


void StringCopy(char* destination, char* source, int length )
{
  for (int i = 0; i < length ; i++ )
  {
    destination[i] = source[i];
  }
  destination[length] = '\0';
}

