#include "stm32f10x_lib.h"
#include"system_func.h"
#include"lcd.h"
#include "key_pad.h"
#include "UART_lib.h"

#define HINTS_SIZE 20
#define BUFSIZE 128
#define TRUE 1
#define FALSE 0

// 프로토타입 선언
void Timer_Configuration(int TimerType, int Prescaler, int Period);
void EXTI_Interrupt_Configuration(u8 EXTIx_IRQChannel, u32 EXTI_Linex, u8 GPIO_PortSourceGPIOx, u8 GPIO_PinSourcex);
void GPIO_Setting_Output(u16 GPIO_Pin_n, GPIO_TypeDef* GPIOx);
void GPIO_Setting_Input(u16 GPIO_Pin_n, GPIO_TypeDef* GPIOx);
void Switch_Configuration();
void FND_Configuration();
void Delay(vu32 nCount);
char NumToChar(int num);
void twenty_question_quiz();
void Uart_GetData();
int StringCompare(char* str1, char* str2, int size);
//프로토타입 끝

// uart 통신을 위한 버퍼 및 변수들
char returnStringBuffer[20];
char MainBuffer[BUFSIZE];
char ReceiveBuffer[BUFSIZE]; 
int BufferValid = 0;
int ReceiveLength = 0;
int MainLength = 0;
// uart 변수 끝

//  타이머 시간 변수
unsigned char time_10m = 0, time_1m = 0, time_10s = 0, time_1s = 0;
// 타이머 시간 변수 끝

// 스무고개 변수

int answerButtonClicked = 0;
int resetButtonClicked = 0;

char answerStr[] = "rainbow";
char welcomeStr[] = "Welcome to Twenty-Question-Quiz!";
char startStr[] = "Here we go!";
char failStr[] = "You failed. try again!";
char switchStr[] = "know the answer / show hint";
char gratStr[] = "Correct!";
char resetStr[] = "If you want to challenge again, please press reset button.";
char hints[HINTS_SIZE][80] =
{
	"1. It isn't alive.",
	"2. It is untouchable.",
	"3. It isn't an emotion.",
	"4. It is rare.",
	"5. It is beautiful.",
	"6. Generally, it has arc shape.",
	"7. It has red color.",
	"8. It has many colors.",
	"9. it has different number of colors between cultures.",
	"10. It is in the air.",
	"11. It appears the opposite side of the sun.",
	"12. It's duration is short.",
	"13. Sometimes, it has its sister or sisters.",
	"14. It appears at bible as a gift of god.",
	"15. It can be made artificially.",
	"16. Principle of its generation is explained first in 1637.",
	"17. If there is 'it' in the west sky, it will rain after a while.",
	"18. It has more colors that cannot be seen by human.",
	"19. You can also see 'it' with a piece of prism.",
	"20. It can be seen in the sky after raining."
};

//
/*
선 연결법
gpioa 0 ~ 7 -> lcd d0 ~ d7
gpioa 9 10 -> uart tx rx
gpiob 0 1 2 -> lcd rs rw e
gpioc 0 ~ 6 -> keypad
gpioc 78 -> switch
*/
int main(void)
{

//// setting for initializing
        // init stm32
        Init_STM32F103();
        // clock
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOB |RCC_APB2Periph_GPIOA| RCC_APB2Periph_AFIO, ENABLE);
        // set uart
        Uart_Init_Setting();
        // set timer
        Timer_Configuration(2, 1200, 10000);
        // set keypad
        Init_keypad();
        // init lcd
        lcdInit();
        // switch init
        Switch_Configuration();

//// end initializing
        while(1)
        {
          	twenty_question_quiz();
        }

/*initialzing message 
	const char start_string1[100] = "Start\n";
	UART_Send(start_string1, 6);
 message end 
 */
/*
        // keypad test code
        while(1)
        {
          int keypadinput = GetKeypadInput();
          if(keypadinput != -1)
          {
            
            UART_Send_Char(NumToChar(keypadinput));
            delay_ms(1000);

          }
          
        }
	// keypad test code end
*/


}
void FND_Configuration()
{
}

void Switch_Configuration()
{
  GPIO_Setting_Input(GPIO_Pin_7, GPIOC);
  GPIO_Setting_Input(GPIO_Pin_8, GPIOC);
  EXTI_Interrupt_Configuration(EXTI0_IRQChannel, EXTI_Line7, GPIO_PortSourceGPIOC, GPIO_PinSource7);
  EXTI_Interrupt_Configuration(EXTI1_IRQChannel, EXTI_Line8, GPIO_PortSourceGPIOC, GPIO_PinSource8);
  
}

void twenty_question_quiz() 
{ // 필요한 것: switch, LCD, array FND, LED

	// LCD - 인사와 시작
	lcdPrintData(welcomeStr, sizeof(welcomeStr));
        UART_Send("Welcome\n", 9);
        delay_ms(1500);

	lcdPrintData(startStr, sizeof(startStr));
        UART_Send("Start\n", 7);        
        delay_ms(1500);

	int tries = 0;
        
	while (1) 
	{

		// 20개 힌트 다 줬으면 실패함을 알려줌(LCD)
		if (tries >= HINTS_SIZE)
                {
			lcdPrintData(failStr, sizeof(failStr));
                        UART_Send("Fail\n", 6);
                        break;
                }

		// LCD - 처음에 힌트 한 번 줌 (array FND - 제한 시간 10초)
		lcdPrintData(hints[tries], sizeof(hints[tries++]));
		UART_Send("Hint!\n", 7);
                // array FND
                delay_ms(5000);

    
		// switch - (답 / 힌트) - LCD / 힌트 20개 다 줬으면 힌트 버튼 눌러도 효과 없도록 함
		lcdPrintData(switchStr, sizeof(switchStr));
                UART_Send("Ans/Hint?\n", 11);
                
                delay_ms(5000);
		// 답을 입력할 경우 - 성공시 축하(LCD, LED)하고 break / 실패시 continue
		if (answerButtonClicked) 
		{
                  answerButtonClicked = 0;
			// array FND - 제한 시간 표시
			// UART - 답 입력 받음
                        UART_Send("Give Answer\n", 13); 
			Uart_GetData();

			if (StringCompare(returnStringBuffer, answerStr, sizeof(answerStr)))
			{
				// LCD, LED - 축하
				lcdPrintData(gratStr, sizeof(gratStr));
                                UART_Send("Grats\n", 7);
                                break;
			}
			else continue;
		} // end if
		// LCD - 힌트를 보는 경우 (array FND - 제한 시간 10초)
		else 
		{
			// array FND - 제한 시간 10초
			lcdPrintData(hints[tries], sizeof(hints[tries++]));
                        UART_Send("AnotherHint\n", 13);
                        delay_ms(5000);
                        
		} //  end else

	} // end while

	// LCD - 또 도전하시려면 리셋버튼을 눌러주세요
	lcdPrintData(resetStr, sizeof(resetStr));
        UART_Send("Over\n", 6);
        delay_ms(5000);
        if(resetButtonClicked)
          return;
        return;
} // end twenty_question_quiz()

void Uart_GetData()
{
	while (1)
	{
		if (BufferValid)
		{
			StringCopy(returnStringBuffer, MainBuffer,MainLength);
			BufferValid = 0;
			return;
		}
		else
		{
			if (UART_RxAvailable())
			{
				UART_Get();
			}
		}
	}
}

int StringCompare(char* str1, char* str2, int size)
{
	for(int i = 0; i < size; i++)
	{
		if (str1[i] != str2[i]) return -1;
	}
	return 1;
}



void EXTI_Interrupt_Configuration(u8 EXTIx_IRQChannel, u32 EXTI_Linex, u8 GPIO_PortSourceGPIOx, u8 GPIO_PinSourcex)
{
	EXTI_InitTypeDef EXTI_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

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

void Timer_Configuration(int TimerType, int Prescaler, int Period)
{
  TIM_TypeDef* Timer;
   u8 TimerInterruptType;
   
  if (TimerType == 2)
  {  Timer = TIM2; TimerInterruptType =TIM2_IRQChannel; }
  else if (TimerType == 3)
  { Timer = TIM3; TimerInterruptType = TIM3_IRQChannel; }
  else if (TimerType == 4)
  { Timer = TIM4; TimerInterruptType = TIM4_IRQChannel; }
  else
    return;
  
  TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;
  NVIC_InitTypeDef NVIC_InitStructure;

  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

  TIM_TimeBaseInitStruct.TIM_Prescaler = Prescaler - 1; // 1200
  TIM_TimeBaseInitStruct.TIM_CounterMode = TIM_CounterMode_Up;
  TIM_TimeBaseInitStruct.TIM_Period = Period - 1; // 10000
  TIM_TimeBaseInitStruct.TIM_ClockDivision = TIM_CKD_DIV1;
  TIM_TimeBaseInit(Timer, &TIM_TimeBaseInitStruct);

  NVIC_InitStructure.NVIC_IRQChannel = TimerInterruptType;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);

  TIM_ITConfig(Timer, TIM_IT_Update, ENABLE);
  TIM_Cmd(Timer, ENABLE);

}
                                             
void Delay(vu32 nCount)
{
	for (; nCount != 0; nCount--);
}
                                             
char NumToChar(int num)
{
  return num + 0x30; 
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


             