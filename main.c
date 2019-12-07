#include "stm32f10x_lib.h"
#include"system_func.h"
//#include"lcd.h"
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
void DelayFND(int min, int tensec, int sec);
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
int time_1m = 0, time_10s = 0, time_1s = 0;
// 타이머 시간 변수 끝

// 그 외 변수
unsigned int LED_data = 0x8000;
u16 FND_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7 ;
u16 FND_COM_pin = GPIO_Pin_8 |GPIO_Pin_9|GPIO_Pin_10 |GPIO_Pin_11;
unsigned char FND_DATA_TBL[]={0x3F,0X06,0X5B,0X4F,0X66,0X6D,0X7C,0X07,0X7F, 0X67,0X77,0X7C,0X39,0X5E,0X79,0X71,0X08,0X80};
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
char updownStr[] = "!UpDown Game!\n";
char putStr[] = "Put number(0~100)\n";
char gratsStr[] = "Grats! You win!\n";
char upStr[] = "UP\n";
char downStr[] = "DOWN\n";
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
gpioa 0 - 7 led
gpioa 9 10 -> uart tx rx

gpiob 0 1 -> switch

gpioc 0 ~ 6 -> keypad
gbioc 8 ~ 11 -> c0 ~ c3
gpioa 0 ~ 7 A ~ H

*/
int main(void)
{   
	Initializer();
	while(1)
	{
          UART_Send("GameStart\n", 11); 
//		twenty_question_quiz();
		up_and_down_game();
	}
}

void up_and_down_game() 
{ // keypad, LCD, LED, array FND(제한 시간을 넣고 싶다면.)
	// 정답 값을 랜덤으로 생성 (범위: 0 ~ 100)
	int answer = 50;

	// LCD - 인사와 시작
	UART_Send(updownStr, sizeof(updownStr));
	DelayFND(0, 1, 0);
	while (1) 
	{
		// LCD
		UART_Send(putStr, sizeof(putStr));

		// keypad - 숫자를 입력 받는다
		//int keypadInput = keypaduse();
                int keypadone;
                int keypadten;
                while(1)
                {
                  keypadten = GetKeypadInput();
                  delay_ms(300);
                  if (keypadten != -1)
                    break; 
                }
                
                while(1)
                {
                  keypadone = GetKeypadInput();
                  delay_ms(300);
                  if (keypadone != -1)
                    break; 
                }
                
                char buff[3];
                
                buff[0] = keypadten + '0';
                buff[1] = keypadone + '0';
                buff[2] = '\0';
                UART_Send(buff, 3);
                
                int keypadInput = keypadone + keypadten * 10;
		
                if (keypadInput == answer)
		{ // 정답
			// LCD
			UART_Send(gratsStr, sizeof(gratsStr));
			// LED 축하
			Show_LED();
			break;
		}
		else if (keypadInput > answer) 
		{ // UP

			UART_Send(upStr, sizeof(upStr));

		}
		else 
		{ // DOWN
			UART_Send(downStr, sizeof(downStr));
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
	// set keypad
	Init_keypad();
	// init lcd
	//lcdInit();
	// switch init
	Switch_Configuration();
	// led setting
	LED_Configuration();
	//fnd
	FND_Configuration();

}
void Show_LED()
{
	for (int a = 0; a < 3; a++)
	{ // 3번
		for (int i = 0; i < 24; i++)
		{
			GPIO_ResetBits(GPIOB, LED_data);

			if (LED_data == 0x8000)
				LED_data = 0x0100;
			else
				LED_data <<= 1;
			GPIO_SetBits(GPIOB, LED_data);
			delay_ms(200);
		}

		for (int j = 0; j < 24; j++)
		{
			GPIO_ResetBits(GPIOB, LED_data);

			if (LED_data == 0x0100)
				LED_data = 0x8000;
			else
				LED_data >>= 1;

			GPIO_SetBits(GPIOB, LED_data);
			delay_ms(200);
		}
	}
}
void FND_Configuration()
{
	Timer_Configuration(2, 7200, 10000);

	GPIO_InitTypeDef GPIO_InitStructure;
        
	GPIO_InitStructure.GPIO_Pin = FND_Pin;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
        
        GPIO_InitStructure.GPIO_Pin = FND_COM_pin;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
        
}
void LED_Configuration()
{
	GPIO_InitTypeDef GPIO_InitStructure;

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_11 | GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15 ;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOB, &GPIO_InitStructure );
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
	UART_Send(welcomeStr, sizeof(welcomeStr));
	delay_ms(1500);

	UART_Send(startStr, sizeof(startStr));
	delay_ms(1500);

	int tries = 0;
        
	while (1) 
	{

		// 20개 힌트 다 줬으면 실패함을 알려줌(LCD)
		if (tries >= HINTS_SIZE)
                {
			UART_Send(failStr, sizeof(failStr));
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
	UART_Send(resetStr, sizeof(resetStr));
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
		UART_Send("A:\n", 4);

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

					keypadbuffer[keypadorder] = keypadinput;

				}
				else if (keypadorder == 1)
				{
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

void DelayFND(int min, int tensec, int sec)
{
	time_1m = min;
	time_10s = tensec;
	time_1s = sec;
	while (1) 
	{
		if (time_1m + time_10s + time_1s == 0) break;

		GPIO_SetBits(GPIOC, FND_COM_pin);
		GPIO_ResetBits(GPIOC, GPIO_Pin_8);

		GPIO_ResetBits(GPIOA, FND_Pin);
		GPIO_SetBits(GPIOA, FND_DATA_TBL[time_1s]);
		Delay(0x1FFF);

		GPIO_SetBits(GPIOC, FND_COM_pin);
		GPIO_ResetBits(GPIOC, GPIO_Pin_9);

		GPIO_ResetBits(GPIOA, FND_Pin);
		GPIO_SetBits(GPIOA, FND_DATA_TBL[time_10s]);
		Delay(0x1FFF);


		GPIO_SetBits(GPIOC, FND_COM_pin);
		GPIO_ResetBits(GPIOC, GPIO_Pin_10);

		GPIO_ResetBits(GPIOA, FND_Pin);
		GPIO_SetBits(GPIOA, FND_DATA_TBL[time_1m] | 0x80);
		Delay(0x1FFF);

	}//end while
}
             