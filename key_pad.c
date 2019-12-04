#include"key_pad.h"

void Init_keypad(void){
  
  GPIO_InitTypeDef GPIO_InitStructure;
        
  RCC_APB2PeriphClockCmd(ROW_DDR | COLUMN_DDR, ENABLE);  

  GPIO_InitStructure.GPIO_Pin = KEY_1 | KEY_2 | KEY_3;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_Init(COLUMN_PORT, &GPIO_InitStructure);
  
  GPIO_InitStructure.GPIO_Pin = ROW_1 | ROW_2 | ROW_3| ROW_4;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 
  GPIO_Init(ROW_PORT, &GPIO_InitStructure);

  Timer3_Delay_init();
}



int GetButtonInput(void) 
{
	//ROW_PORT	=	ROW_1;
	GPIO_ResetBits(ROW_PORT, ROW_KEY);
	GPIO_SetBits(ROW_PORT, ROW_1);

	if (GPIO_ReadInputDataBit(COLUMN_PORT, KEY_1)) return 1;
	else if (GPIO_ReadInputDataBit(COLUMN_PORT, KEY_2))	return 2;
	else if (GPIO_ReadInputDataBit(COLUMN_PORT, KEY_3)) return 3;

	delay_ms(1);

	GPIO_ResetBits(ROW_PORT, ROW_KEY);
	GPIO_SetBits(ROW_PORT, ROW_2);

	if (GPIO_ReadInputDataBit(COLUMN_PORT, KEY_1)) return 4;
	else if (GPIO_ReadInputDataBit(COLUMN_PORT, KEY_2)) return 5;
	else if (GPIO_ReadInputDataBit(COLUMN_PORT, KEY_3)) return 6;

	delay_ms(1);

	GPIO_ResetBits(ROW_PORT, ROW_KEY);
	GPIO_SetBits(ROW_PORT, ROW_3);

	if (GPIO_ReadInputDataBit(COLUMN_PORT, KEY_1)) return 7;
	else if (GPIO_ReadInputDataBit(COLUMN_PORT, KEY_2)) return 8;
	else if (GPIO_ReadInputDataBit(COLUMN_PORT, KEY_3)) return 9;

	delay_ms(1);

	GPIO_ResetBits(ROW_PORT, ROW_KEY);
	GPIO_SetBits(ROW_PORT, ROW_4);
	delay_ms(1);

	if (GPIO_ReadInputDataBit(COLUMN_PORT, KEY_1)) return 10;
	else if (GPIO_ReadInputDataBit(COLUMN_PORT, KEY_2)) return 0;
	else if (GPIO_ReadInputDataBit(COLUMN_PORT, KEY_3)) return 11;
	delay_ms(1);

        return -1;
}


