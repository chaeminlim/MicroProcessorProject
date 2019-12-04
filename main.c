#include "stm32f10x_lib.h"
#include"system_func.h"
#include"lcd.h"
#include "key_pad.h"

#define BUFSIZE 128
#define TRUE 1
#define FALSE 0

// 프로토타입 선언
void UART_NVIC_Configuration(void);
void UART_GPIO_Configuration(void);
void USART_Configuration(void);
void Timer_Configuration(int Prescaler, int Period);
void EXTI_Interrupt_Configuration(u8 EXTIx_IRQChannel, u32 EXTI_Linex, u8 GPIO_PortSourceGPIOx, u8 GPIO_PinSourcex);
void GPIO_Setting_Output(u16 GPIO_Pin_n, GPIO_TypeDef* GPIOx);
void GPIO_Setting_Input(u16 GPIO_Pin_n, GPIO_TypeDef* GPIOx);
int UARTRead(char* buffer);
void UARTSend(const  char* pucBuffer, int ulCount);
void Uart_Init_Setting(void);
void StringCopy(char* destination, char* source, int size);
void MemCopy(char* destination, char* source, int length );
void UARTGet(void);
int UART_RxAvailable(void);
void Delay(vu32 nCount);

//프로토타입 끝

// uart 통신을 위한 버퍼 및 변수들
char MainBuffer[BUFSIZE];
char ReceiveBuffer[BUFSIZE];
int BufferValid = 0;
int ReceiveLength = 0;
int MainLength = 0;
// uart 변수 끝

//  타이머 시간 변수
unsigned char time_10m = 0, time_1m = 0, time_10s = 0, time_1s = 0;
// 타이머 시간 변수 끝

int main(void)
{
//// setting for initializing
	// init stm32
	Init_STM32F103();
	// set uart
	Uart_Init_Setting();
	// set timer
	//Timer_Configuration(1200, 10000);
	
//// end initializing
	
	// initialzing message 
	const char start_string1[100] = "Start\n";
	UARTSend(start_string1, 6);
	// message end


	
	while (1)
	{
          if (BufferValid)
          {
            UARTSend(MainBuffer, MainLength);
            BufferValid = 0;
          }
          else
          {
			  if (UART_RxAvailable())
			  {
				  UARTGet();
			  }
          }
    }
        
}


void EXTI_Interrupt_Configuration(u8 EXTIx_IRQChannel, u32 EXTI_Linex, u8 GPIO_PortSourceGPIOx, u8 GPIO_PinSourcex)
{
	EXTI_InitTypeDef EXTI_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);

	NVIC_InitStructure.NVIC_IRQChannel = EXTIx_IRQChannel;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	GPIO_EXTILineConfig(GPIO_PortSourceGPIOx, GPIO_PinSourcex);
	EXTI_InitStructure.EXTI_Line = EXTI_Linex;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);

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
void GPIO_Setting_Output(u16 GPIO_Pin_n, GPIO_TypeDef* GPIOx)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_n;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOx, &GPIO_InitStructure);
}
void GPIO_Setting_Input(u16 GPIO_Pin_n, GPIO_TypeDef* GPIOx)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_n;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOx, &GPIO_InitStructure);
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

void Timer_Configuration(int Prescaler, int Period)
{
	TIM_TimeBaseInitTypeDef TIM2_TimeBaseInitStruct;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

	TIM2_TimeBaseInitStruct.TIM_Prescaler = Prescaler - 1; // 1200
	TIM2_TimeBaseInitStruct.TIM_CounterMode = TIM_CounterMode_Up;
	TIM2_TimeBaseInitStruct.TIM_Period = Period - 1; // 10000
	TIM2_TimeBaseInitStruct.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseInit(TIM2, &TIM2_TimeBaseInitStruct);

	NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQChannel;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	
	TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);
	TIM_Cmd(TIM2, ENABLE);

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
void UARTSend(const char* pucBuffer, int ulCount)
{
	while (ulCount--)
	{
		while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
		USART_SendData(USART1, (char)* pucBuffer++);
	}
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
void Delay(vu32 nCount)
{
	for (; nCount != 0; nCount--);
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

             