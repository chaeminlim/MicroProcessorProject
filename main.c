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
void Initializer();
void Timer_Configuration(int TimerType, int Prescaler, int Period);
void EXTI_Interrupt_Configuration(u8 EXTIx_IRQChannel, u32 EXTI_Linex, u8 GPIO_PortSourceGPIOx, u8 GPIO_PinSourcex);
void GPIO_Setting_Output(u16 GPIO_Pin_n, GPIO_TypeDef* GPIOx);
void GPIO_Setting_Input(u16 GPIO_Pin_n, GPIO_TypeDef* GPIOx);
void Switch_Configuration();
void FND_Configuration();
void Delay(vu32 nCount);
char NumToChar(int num);
void twenty_question_quiz();
void up_and_down_game();
void Uart_GetData();
void Show_LED();
int StringCompare(char* str1, char* str2, int size);
int keypaduse(); // keypad 입력완료버튼 누를때까지 반복 하며 사용자의 업다운 퀴즈 답안값을 반환하는 함수.
void LED_Configuration();
void PrintLCDData(char* data, u8 nBytes);
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

// 그 외 변수
unsigned int LED_data = 0x0080;

//

// 스무고개 변수

int answerButtonClicked = 0;
int resetButtonClicked = 0;

char answerStr[] = "rainbow";
char welcomeStr[] = "Welcome to Twenty-Question-Quiz!\n";
char startStr[] = "Here we go!\n";
char failStr[] = "You failed. try again!\n";
char switchStr[] = "know the answer / show hint\n";
char gratStr[] = "Correct!\n";
char resetStr[] = "If you want to challenge again, please press reset button.\n";
char typeAnsStr[] = "What is Your Answer?\n";
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
gpiob  5 6 7-> lcd rs rw e
gpioc 0 ~ 6 -> keypad
gpiob 0 1 -> switch
gpioa 8 ~ 15 fnd
gpiob 8 ~ 10
gpioc 7 - 14 led
*/
int main(void)
{   
	Initializer();
	while(1)
	{
//		twenty_question_quiz();
		up_and_down_game();
	}
}

void up_and_down_game() 
{ // keypad, LCD, LED, array FND(제한 시간을 넣고 싶다면.)
	// 정답 값을 랜덤으로 생성 (범위: 0 ~ 100)
	int answer = 50;

	// LCD - 인사와 시작
	char* str = "!UpDown Game!";
	lcdPrintData(str, sizeof(str) / sizeof(char));
        delay_ms(5000);
	while (1) 
	{
		// LCD
		str = "Put number(0~100)";
		lcdPrintData(str, sizeof(str) / sizeof(char));

		// keypad - 숫자를 입력 받는다
		int keypadInput = keypaduse();

		if (keypadInput == answer)
		{ // 정답
			// LCD
			str = "Grats! You win!";
			PrintLCDData(str, sizeof(str) / sizeof(char));
			// LED 축하
			Show_LED();
			break;
		}
		else if (keypadInput > answer) 
		{ // UP
			str = "UP";
			PrintLCDData(str, sizeof(str) / sizeof(char));

		}
		else 
		{ // DOWN
			str = "DOWN";
			PrintLCDData(str, sizeof(str) / sizeof(char));
		}
	} // end while
} // end up_and_down_game()

void Initializer()
{
	// init stm32
	Init_STM32F103();
	// clock
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO, ENABLE);
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
	// led setting
	LED_Configuration();
}
void PrintLCDData(char* data, u8 nBytes)
{

	u8 i;
	u8 flag = 0;
	if (!data) return;

	for (i = 0; i < nBytes; i++)
	{
		lcdDataWrite(data[i]);
		`
	}
}
void Show_LED()
{
	for (int a = 0; a < 3; a++)
	{ // 3번
		for (int i = 0; i < 24; i++)
		{
			GPIO_ResetBits(GPIOA, LED_data);

			if (LED_data == 0x0080)
				LED_data = 0x0001;
			else
				LED_data <<= 1;
			GPIO_SetBits(GPIOA, LED_data);
			delay_ms(1000);
		}

		for (int j = 0; j < 24; j++)
		{
			GPIO_ResetBits(GPIOA, LED_data);

			if (LED_data == 0x0001)
				LED_data = 0x0080;
			else
				LED_data >>= 1;

			GPIO_SetBits(GPIOA, LED_data);
			delay_ms(1000);
		}
	}
}
void FND_Configuration()
{
}
void LED_Configuration()
{
	GPIO_InitTypeDef GPIO_InitStructure;

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10 | 
		GPIO_Pin_11| GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
}
void Switch_Configuration()
{
  GPIO_Setting_Input(GPIO_Pin_0, GPIOB);
  GPIO_Setting_Input(GPIO_Pin_1, GPIOB);
  EXTI_Interrupt_Configuration(EXTI0_IRQChannel, EXTI_Line0, GPIO_PortSourceGPIOB, GPIO_PinSource0);
  EXTI_Interrupt_Configuration(EXTI1_IRQChannel, EXTI_Line1, GPIO_PortSourceGPIOB, GPIO_PinSource1);  
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
		UART_Send(hints[tries], sizeof(hints[tries++]));
                // array FND
                delay_ms(10000);

    
		// switch - (답 / 힌트) - LCD / 힌트 20개 다 줬으면 힌트 버튼 눌러도 효과 없도록 함
		UART_Send(switchStr, sizeof(switchStr));
                
                delay_ms(5000);
		// 답을 입력할 경우 - 성공시 축하(LCD, LED)하고 break / 실패시 continue
		if (answerButtonClicked) 
		{
                  answerButtonClicked = 0;
			// array FND - 제한 시간 표시
			// UART - 답 입력 받음
            UART_Send(typeAnsStr, sizeof(typeAnsStr)); 
            Uart_GetData();

			if (StringCompare(returnStringBuffer, answerStr, sizeof(answerStr)))
			{
				// LCD, LED - 축하
				UART_Send(gratStr, sizeof(gratStr));
                                break;
			}
			else continue;
		} // end if
		// LCD - 힌트를 보는 경우 (array FND - 제한 시간 10초)
		else 
		{
			// array FND - 제한 시간 10초
			UART_Send(hints[tries], sizeof(hints[tries++]));
                        delay_ms(10000);
                        
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
int keypaduse() 
{ // keypad 입력완료버튼 누를때까지 반복 하며 사용자의 업다운 퀴즈 답안값을 반환하는 함수.

	int keypadinput;
	int keypadbuffer[2]; // 입력값 저장 버퍼
	int keypadorder, keypadoutput = 0; // keypad의 입력중인 자릿수를 가리키는 변수 / keypadcheck  입력값이 답인지 확인 하는 변수
	/*  123
		456
		789
		*0# 에서 * = 입력버튼 /  #  = delete 버튼
	*/
	for (keypadorder = 0; keypadorder < 3; keypadorder++) 
    {
		PrintLCDData("A:", 3);

		keypadinput = GetKeypadInput();
		delay_ms(500);
		if (keypadinput != -1)
		{

			// 숫자 지우기
			if (keypadinput == 11)
			{
				keypadorder--;
				if (keypadorder >= 0)
				{
					keypadbuffer[keypadorder] = 0;
				}
			}

			//숫자입력
			if (keypadinput >= 0 && keypadinput <= 9) 
{
				if (keypadorder == 0) 
				{
					lcdGotoXY(2, 0); 
					lcdDataWrite(keypadinput +  '0');
					keypadbuffer[keypadorder] = keypadinput;

				}
				else if (keypadorder == 1)
				{
					lcdGotoXY(3, 0);
					lcdDataWrite(keypadinput + '0');
					keypadbuffer[keypadorder] = keypadinput;

				}
				else 
				{
					keypadorder--;
				}
			}

			// 숫자 입력완료
			if (keypadinput == 10) 
			{
				if (keypadorder == 0) 
				{
					keypadorder--;
				}
				if (keypadorder == 1) 
				{
					keypadorder = 10;
				}
				else 
				{
					keypadorder = 3;
				}
			}
		}

		//keypad display
		if (keypadorder >= 10)
		{
			keypadoutput = keypadbuffer[0];
		}
		else 
		{
			keypadoutput = keypadbuffer[0] * 10;

			keypadoutput += keypadbuffer[1];
		}
				
		if (keypadinput == -1) 
		{
			keypadorder--;
		}
	}
	return keypadoutput;
}   // keypad  code end
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


             