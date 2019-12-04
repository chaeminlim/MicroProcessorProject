#include "stm32f10x_lib.h"
#include"system_func.h"
#include"lcd.h"

#define BUFSIZE 128
#define TRUE 1
#define FALSE 0

// 프로토타입 선언
void UART_NVIC_Configuration(void);
void UART_GPIO_Configuration(void);
void USART_Configuration(void);
void EXTI_Configuration(void);
void GPIO_Setting_Output(u16 GPIO_Pin_n);
void GPIO_Setting_Input(u16 GPIO_Pin_n);
int UARTRead(char* buffer);
void UARTSend(const  char* pucBuffer, int ulCount);
void Uart_Init_Setting(void);
void StringCopy(char* destination, char* source, int size);
void MemCopy(char* destination, char* source, int length );
void UARTGet(void);
int RxAvailable(void);
//프로토타입 끝

// uart 통신을 위한 버퍼 및 변수들
char MainBuffer[BUFSIZE];
char ReceiveBuffer[BUFSIZE];
int BufferValid = 0;
int ReceiveLength = 0;
int MainLength = 0;
// uart 변수 끝


int main(void)
{

	Init_STM32F103();
	Uart_Init_Setting();

	const char start_string1[100] = "Start\n";
	UARTSend(start_string1, 6);

	while (1)
	{
          if (BufferValid)
          {
            UARTSend(MainBuffer, MainLength);
            BufferValid = 0;
          }
          else
          {
			  if (RxAvailable())
			  {
				  UARTGet();
			  }
          }
        }
        
}


void EXTI_Configuration(void)
{
	EXTI_InitTypeDef EXTI_InitStructure;

}


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

void GPIO_Setting_Output(u16 GPIO_Pin_n)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_n;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
}
void GPIO_Setting_Input(u16 GPIO_Pin_n)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_n;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
}

/*******************************************************************************
* Function Name  : GPIO_Configuration
* Description    : Configures the different GPIO ports
*******************************************************************************/
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

/*******************************************************************************
* Function Name  : USART_Configuration
* Description    : Configures the USART1
*******************************************************************************/
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


void UARTSend(const char* pucBuffer, int ulCount)
{
	while (ulCount--)
	{
		while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
		USART_SendData(USART1, (char)* pucBuffer++);
	}
}

int RxAvailable(void)
{
	if (USART_GetFlagStatus(USART1, USART_FLAG_RXNE) != RESET)// 수신 버퍼가 비어있지 않으면 // USART_IT_RXNE ??
		return TRUE;
	else
		return FALSE;
}

void UARTGet(void)
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

void StringCopy(char* destination, char* source, int length )
{
  for (int i = 0; i < length ; i++ )
  {
    destination[i] = source[i];
  }
}

/*
void USART1_IRQHandler(void)
{
	// RXNE handler
	if (USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)
	{
		char rx = (char)USART_ReceiveData(USART1);
		if ((rx == '\r') || (rx == '\n'))
		{
			if (ReceiveLength != 0) // Line has some content?
			{
				memcpy((void*)MainBuffer, ReceiveBuffer, ReceiveLength);
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
}
*/

/*
void putch(u8 c)
{
  USART_SendData(USART1, c);
  while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
}

void puts(u8 *s)
{
  while (*s != '\0')
  {
	putch(*s);
	s ++;
  }
}
*/

