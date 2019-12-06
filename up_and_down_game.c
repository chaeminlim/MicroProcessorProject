#include "stm32f10x_lib.h"
#include"system_func.h"
#include"lcd.h"
#include "key_pad.h"
#include "UART_lib.h"

void up_and_down_game() { // keypad, LCD, LED, array FND(제한 시간을 넣고 싶다면.)
	// 정답 값을 랜덤으로 생성 (범위: 0 ~ 100)
	srand(time(NULL));
	int answer = rand()%101;

	// LCD - 인사와 시작
	char *str = "Welcome to Up-And-Down Game!";
	display(str, sizeof(str)/sizeof(char));

	while(1) {
		// LCD
		str = "Enter a number(0 ~ 100)";
		display(str, sizeof(str)/sizeof(char));

		// keypad - 숫자를 입력 받는다
		int num;
		setbuf(stdout, NULL);
		gets(num); // 임시. keypad로 대체해야 함


		if (num == answer) { // 정답
			// LCD
			str = "Congratulations! You win!";
			display(str, sizeof(str)/sizeof(char));

			// LED 축하
			congratulate();
			break;
		}
		else if (num > answer) { // UP
			str = "UP";
			display(str, sizeof(str)/sizeof(char));
		}
		else { // DOWN
			str = "DOWN";
			display(str, sizeof(str)/sizeof(char));
		}
	} // end while
} // end up_and_down_game()

void display(char* str, int size) {
	lcdPrintData(str, size);
} // end display(char* str, int size)

/* LED로 축하해 주는 함수 */
void congratulate() {
	// 뺄 부분은 빼기
	Init_STM32F103();

	GPIO_InitTypeDef GPIO_InitStructure;

	unsigned int LED_data = 0x0080;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0| GPIO_Pin_1 | GPIO_Pin_2 |
	GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7;

	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	for (int a = 0; a < 3; a++) { // 3번
		LED_data = 0x0080;
		for (int i = 0; i < 2; i++) { // 2번씩
			GPIO_ResetBits(GPIOA, LED_data);

			if(LED_data == 0x0080)
				LED_data = 0x0001;
			else
				LED_data <<= 1;

			GPIO_SetBits(GPIOA, LED_data);
			Delay(0xAFFFF);
		}

		LED_data = 0x0001;
		for (int j = 0; j < 2; j++) { // 2번씩
			GPIO_ResetBits(GPIOA, LED_data);

			if(LED_data == 0x0001)
				LED_data = 0x0080;
			else
				LED_data >>= 1;

			GPIO_SetBits(GPIOA, LED_data);
			Delay(0xAFFFF);
		}
	}
}

void Delay(vu32 nCount){
  for(; nCount != 0; nCount--);
}
