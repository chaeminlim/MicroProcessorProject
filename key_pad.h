
#ifndef KEY_PAD_H
#define KEY_PAD_H

#include "stm32f10x_lib.h"
#include "lcd.h" 
#include "user_delay.h"

#define ROW_PORT	GPIOC
#define ROW_DDR         RCC_APB2Periph_GPIOC
#define ROW_MASK	0x0078

#define ROW_1	GPIO_Pin_3
#define ROW_2	GPIO_Pin_4
#define ROW_3	GPIO_Pin_5
#define ROW_4	GPIO_Pin_6

#define ROW_KEY (ROW_1 | ROW_2 | ROW_3 | ROW_4)

#define COLUMN_PORT	    GPIOC
#define COLUMN_DDR         RCC_APB2Periph_GPIOC

//  COLUMN 
#define KEY_1	GPIO_Pin_0  
#define KEY_2	GPIO_Pin_1
#define KEY_3	GPIO_Pin_2

#define COLUMN_MASK  (KEY_1 | KEY_2|KEY_3)

void init_keypad(void);
u8 passwd_check(u8 pass[]);
void passcheck(u8 data);
void reset_check(void);
void back_pass(void);
u8 get_passwd(void);

#endif

