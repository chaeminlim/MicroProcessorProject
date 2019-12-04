/*! \file lcd.c \brief Character LCD driver for HD44780/SED1278 displays. */
//*****************************************************************************
//
// File Name	: 'lcd.c'
// Title		: Character LCD driver for HD44780/SED1278 displays
//					(usable in mem-mapped, or I/O mode)
// Author		: Pascal Stang
// Created		: 11/22/2000
// Revised		: 7/25/2008 
// Version		: 1.x
// Target MCU	: STM32F10x
//
// This code is distributed under the GNU Public License
//		which can be found at http://www.gnu.org/licenses/gpl.txt
//
//*****************************************************************************

#include "stm32f10x_lib.h"
#include "lcd.h"

/*************************************************************/
/********************** LOCAL FUNCTIONS **********************/
/*************************************************************/
GPIO_InitTypeDef GPIO_LCD;

void lcdInitHW(void)
{
	// initialize I/O ports
	// if I/O interface is in use
#ifdef LCD_PORT_INTERFACE
	// initialize LCD control lines

      RCC_APB2PeriphClockCmd(LCD_CTRL_PORT_CLK, ENABLE);

      GPIO_LCD.GPIO_Pin = LCD_CTRL_RS | LCD_CTRL_RW | LCD_CTRL_E;
      GPIO_LCD.GPIO_Speed = GPIO_Speed_50MHz;
      GPIO_LCD.GPIO_Mode = GPIO_Mode_Out_PP;
      GPIO_Init(LCD_CTRL_PORT, &GPIO_LCD);

      GPIO_ResetBits(LCD_CTRL_PORT, LCD_CTRL_RS | LCD_CTRL_RW |LCD_CTRL_E );	//초기 0
		
	// initialize LCD data port to input
	// initialize LCD data lines to pull-up
	#ifdef LCD_DATA_4BIT
          RCC_APB2PeriphClockCmd(LCD_DATA_CLK, ENABLE);

          GPIO_LCD.GPIO_Pin = 0x00F0;
          GPIO_LCD.GPIO_Speed = GPIO_Speed_50MHz;
          GPIO_LCD.GPIO_Mode = GPIO_Mode_Out_PP;
          GPIO_Init(LCD_DATA, &GPIO_LCD);

          GPIO_SetBits(LCD_DATA, 0x00f0 );	//초기 풀업
	#else
          RCC_APB2PeriphClockCmd(LCD_DATA_CLK, ENABLE);

          GPIO_LCD.GPIO_Pin = 0x00FF;
          GPIO_LCD.GPIO_Speed = GPIO_Speed_50MHz;
          GPIO_LCD.GPIO_Mode = GPIO_Mode_Out_PP;
          GPIO_Init(LCD_DATA, &GPIO_LCD);

          GPIO_SetBits(LCD_DATA, 0x00FF );	//초기 풀업
	#endif
#else
	// enable external memory bus if not already enabled

#endif
}

void lcdBusyWait(void)
{
	// wait until LCD busy bit goes to zero
	// do a read from control register
#ifdef LCD_PORT_INTERFACE
	GPIO_ResetBits(LCD_CTRL_PORT, LCD_CTRL_RS);	// set RS to "control"
	#ifdef LCD_DATA_4BIT
		// set data I/O lines to input (4bit)
		// set pull-ups to on (4bit)
          

          GPIO_LCD.GPIO_Pin = 0x00F0;
          GPIO_LCD.GPIO_Speed = GPIO_Speed_50MHz;
          GPIO_LCD.GPIO_Mode = GPIO_Mode_IN_FLOATING;
          GPIO_Init(LCD_DATA, &GPIO_LCD);

          GPIO_SetBits(LCD_DATA, 0x00F0 );	//초기 풀업
	#else
          GPIO_LCD.GPIO_Pin = 0x00FF;
          GPIO_LCD.GPIO_Speed = GPIO_Speed_50MHz;
          GPIO_LCD.GPIO_Mode = GPIO_Mode_IN_FLOATING;
          GPIO_Init(LCD_DATA, &GPIO_LCD);

          GPIO_SetBits(LCD_DATA, 0x00FF );	//초기 풀업
	#endif
	
        GPIO_SetBits(LCD_CTRL_PORT, LCD_CTRL_RW | LCD_CTRL_E); // set R/W to "read"
 
	LCD_DELAY;		// wait

	while((GPIO_ReadInputDataBit(LCD_DATA, 1<<LCD_BUSY)))
	{
        //data = GPIO_ReadInputDataBit(LCD_DATA, 1<<LCD_BUSY);
          GPIO_ResetBits(LCD_CTRL_PORT, LCD_CTRL_E);	// clear "E" line
		LCD_DELAY;					// wait
		LCD_DELAY;					// wait
		GPIO_SetBits(LCD_CTRL_PORT, LCD_CTRL_E);	// set "E" line
		LCD_DELAY;					// wait
		LCD_DELAY;					// wait
		#ifdef LCD_DATA_4BIT	// do an extra clock for 4 bit reads
			GPIO_ResetBits(LCD_CTRL_PORT, LCD_CTRL_E);	// clear "E" line
			LCD_DELAY;					// wait
			LCD_DELAY;					// wait
			GPIO_SetBits(LCD_CTRL_PORT, LCD_CTRL_E);// set "E" line
			LCD_DELAY;				// wait
			LCD_DELAY;				// wait
		#endif
	}
	GPIO_ResetBits(LCD_CTRL_PORT, LCD_CTRL_E);// clear "E" line
	//	leave data lines in input mode so they can be most easily used for other purposes
#else
	
#endif
        
}

void lcdControlWrite(u8 data) 
{
// write the control byte to the display controller
#ifdef LCD_PORT_INTERFACE
	lcdBusyWait();				// wait until LCD not busy
	GPIO_ResetBits(LCD_CTRL_PORT, LCD_CTRL_RS | LCD_CTRL_RW);// set RS to "control"
	// set R/W to "write"
	#ifdef LCD_DATA_4BIT
		// 4 bit write
		GPIO_SetBits(LCD_CTRL_PORT, LCD_CTRL_E);// set "E" line
		
                GPIO_LCD.GPIO_Pin = 0x00F0;
                GPIO_LCD.GPIO_Speed = GPIO_Speed_50MHz;
                GPIO_LCD.GPIO_Mode = GPIO_Mode_Out_PP;
                GPIO_Init(LCD_DATA, &GPIO_LCD);

                GPIO_ResetBits(LCD_DATA,0x00F0);
                GPIO_SetBits(LCD_DATA, (data&0xF0) );
                
		LCD_DELAY;					// wait
		LCD_DELAY;					// wait
		GPIO_ResetBits(LCD_CTRL_PORT, LCD_CTRL_E);	// clear "E" line
		LCD_DELAY;					// wait
		LCD_DELAY;					// wait
		GPIO_SetBits(LCD_CTRL_PORT, LCD_CTRL_E);	// set "E" line
		
                GPIO_ResetBits(LCD_DATA,0x00F0);
                GPIO_SetBits(LCD_DATA, (data<<4)& 0xF0 );                

		LCD_DELAY;					// wait
		LCD_DELAY;					// wait
		GPIO_ResetBits(LCD_CTRL_PORT, LCD_CTRL_E);	// clear "E" line
	#else
		// 8 bit write
		GPIO_SetBits(LCD_CTRL_PORT, LCD_CTRL_E);	// set "E" line
		
                GPIO_LCD.GPIO_Pin = 0x00FF;
                GPIO_LCD.GPIO_Speed = GPIO_Speed_50MHz;
                GPIO_LCD.GPIO_Mode = GPIO_Mode_Out_PP;
                GPIO_Init(LCD_DATA, &GPIO_LCD);
		
                GPIO_ResetBits(LCD_DATA, 0x00FF);
                GPIO_SetBits(LCD_DATA, data);		// output data, 8bits
		LCD_DELAY;				// wait
		LCD_DELAY;				// wait
		GPIO_ResetBits(LCD_CTRL_PORT, LCD_CTRL_E);// clear "E" line
	#endif
	

#else

#endif
}

u8 lcdControlRead(void)
{
// read the control byte from the display controller
	u8 data;
#ifdef LCD_PORT_INTERFACE
	lcdBusyWait();				// wait until LCD not busy
	#ifdef LCD_DATA_4BIT
		 GPIO_LCD.GPIO_Pin = 0x00F0;
                GPIO_LCD.GPIO_Speed = GPIO_Speed_50MHz;
                GPIO_LCD.GPIO_Mode = GPIO_Mode_IN_FLOATING;
                GPIO_Init(LCD_DATA, &GPIO_LCD);

                GPIO_SetBits(LCD_DATA, 0x00F0 );//초기 풀업
	#else
		 GPIO_LCD.GPIO_Pin = 0x00FF;
                GPIO_LCD.GPIO_Speed = GPIO_Speed_50MHz;
                GPIO_LCD.GPIO_Mode = GPIO_Mode_IN_FLOATING;
                GPIO_Init(LCD_DATA, &GPIO_LCD);

          GPIO_SetBits(LCD_DATA, 0x00FF );	//초기 풀업
	#endif
	GPIO_ResetBits(LCD_CTRL_PORT, LCD_CTRL_RS);	// set RS to "control"
	GPIO_SetBits(LCD_CTRL_PORT, LCD_CTRL_RW);	// set R/W to "read"
	
        #ifdef LCD_DATA_4BIT
		// 4 bit read
		GPIO_SetBits(LCD_CTRL_PORT, LCD_CTRL_E);	// set "E" line
		LCD_DELAY;						// wait
		LCD_DELAY;						// wait
		data = (GPIO_ReadInputData(LCD_DATA))&0xF0;	// input data, high 4 bits
		GPIO_ResetBits(LCD_CTRL_PORT, LCD_CTRL_E);	// clear "E" line
		LCD_DELAY;						// wait
		LCD_DELAY;						// wait
		GPIO_SetBits(LCD_CTRL_PORT, LCD_CTRL_E);	// set "E" line
		LCD_DELAY;						// wait
		LCD_DELAY;						// wait
		data |= (GPIO_ReadInputData(LCD_DATA))>>4;	// input data, low 4 bits
		GPIO_ResetBits(LCD_CTRL_PORT, LCD_CTRL_E);	// clear "E" line
	#else
		// 8 bit read
		GPIO_SetBits(LCD_CTRL_PORT, LCD_CTRL_E);	// set "E" line
		LCD_DELAY;						// wait
		LCD_DELAY;						// wait
		data = (GPIO_ReadInputData(LCD_DATA));		// input data, 8bits
		GPIO_ResetBits(LCD_CTRL_PORT, LCD_CTRL_E);	// clear "E" line
	#endif
	//	leave data lines in input mode so they can be most easily used for other purposes
#else
	
	//GPIO_ResetBits(MCUCR, SRW);			// disable RAM waitstate
#endif
	return data;
}

void lcdDataWrite(u8 data) 
{
// write a data byte to the display
#ifdef LCD_PORT_INTERFACE
	lcdBusyWait();						// wait until LCD not busy
	GPIO_SetBits(LCD_CTRL_PORT, LCD_CTRL_RS);		// set RS to "data"
	GPIO_ResetBits(LCD_CTRL_PORT, LCD_CTRL_RW);		// set R/W to "write"
	#ifdef LCD_DATA_4BIT
		// 4 bit write
		GPIO_SetBits(LCD_CTRL_PORT, LCD_CTRL_E);	// set "E" line
        	
                GPIO_LCD.GPIO_Pin = 0x00F0;
                GPIO_LCD.GPIO_Speed = GPIO_Speed_50MHz;
                GPIO_LCD.GPIO_Mode = GPIO_Mode_Out_PP;
                GPIO_Init(LCD_DATA, &GPIO_LCD);

		GPIO_ResetBits(LCD_DATA,0x00F0);
                GPIO_SetBits(LCD_DATA, (data&0xF0) );	// output data, high 4 bits

		LCD_DELAY;					// wait
		LCD_DELAY;					// wait
		GPIO_ResetBits(LCD_CTRL_PORT, LCD_CTRL_E);	// clear "E" line
		LCD_DELAY;					// wait
		LCD_DELAY;					// wait
		GPIO_SetBits(LCD_CTRL_PORT, LCD_CTRL_E);	// set "E" line

		GPIO_ResetBits(LCD_DATA,0x00F0);
                GPIO_SetBits(LCD_DATA, (data<<4)&0xF0);	             

		LCD_DELAY;					// wait
		LCD_DELAY;					// wait
		GPIO_ResetBits(LCD_CTRL_PORT, LCD_CTRL_E);	// clear "E" line
	#else
		// 8 bit write
		GPIO_SetBits(LCD_CTRL_PORT, LCD_CTRL_E);	// set "E" line
		 GPIO_LCD.GPIO_Pin = 0x00FF;
                GPIO_LCD.GPIO_Speed = GPIO_Speed_50MHz;
                GPIO_LCD.GPIO_Mode = GPIO_Mode_Out_PP;
                GPIO_Init(LCD_DATA, &GPIO_LCD);

		GPIO_ResetBits(LCD_DATA,0x00FF);
                GPIO_SetBits(LCD_DATA, data&0xFF);	   
		LCD_DELAY;					// wait
		LCD_DELAY;					// wait
		GPIO_ResetBits(LCD_CTRL_PORT, LCD_CTRL_E);	// clear "E" line
	#endif
	

#else

#endif
}

u8 lcdDataRead(void)
{
// read a data byte from the display
	u8 data;
#ifdef LCD_PORT_INTERFACE
	lcdBusyWait();				// wait until LCD not busy
	#ifdef LCD_DATA_4BIT
         	GPIO_LCD.GPIO_Pin = 0x00F0;
                GPIO_LCD.GPIO_Speed = GPIO_Speed_50MHz;
                GPIO_LCD.GPIO_Mode = GPIO_Mode_IN_FLOATING;
                GPIO_Init(LCD_DATA, &GPIO_LCD);

                GPIO_SetBits(LCD_DATA, 0x00F0 );	//초기 풀업
	#else
		GPIO_LCD.GPIO_Pin = 0x00FF;
                GPIO_LCD.GPIO_Speed = GPIO_Speed_50MHz;
                GPIO_LCD.GPIO_Mode = GPIO_Mode_IN_FLOATING;
                GPIO_Init(LCD_DATA, &GPIO_LCD);

                GPIO_SetBits(LCD_DATA, 0x00FF );	//초기 풀업
	#endif
	GPIO_SetBits(LCD_CTRL_PORT, LCD_CTRL_RS | LCD_CTRL_RW);	// set RS to "data"

	#ifdef LCD_DATA_4BIT
		// 4 bit read
		GPIO_SetBits(LCD_CTRL_PORT, LCD_CTRL_E);	// set "E" line
		LCD_DELAY;					// wait
		LCD_DELAY;					// wait
                data = (GPIO_ReadInputData(LCD_DATA))&0xF0;	// input data, high 4 bits
		GPIO_ResetBits(LCD_CTRL_PORT, LCD_CTRL_E);	// clear "E" line
		LCD_DELAY;					// wait
		LCD_DELAY;					// wait
		GPIO_SetBits(LCD_CTRL_PORT, LCD_CTRL_E);	// set "E" line
		LCD_DELAY;					// wait
		LCD_DELAY;					// wait
		data |= (GPIO_ReadInputData(LCD_DATA)>>4)&0x0F;
		GPIO_ResetBits(LCD_CTRL_PORT, LCD_CTRL_E);	// clear "E" line
	#else
		// 8 bit read
		GPIO_SetBits(LCD_CTRL_PORT, LCD_CTRL_E);	// set "E" line
		LCD_DELAY;					// wait
		LCD_DELAY;					// wait
		data = GPIO_ReadInputData(LCD_DATA);
		GPIO_ResetBits(LCD_CTRL_PORT, LCD_CTRL_E);	// clear "E" line
	#endif
	
#else
	
	
#endif
	return data;
}



/*************************************************************/
/********************* PUBLIC FUNCTIONS **********************/
/*************************************************************/

void lcdInit()
{
	u32 i=434782; 
        // 하드웨어 초기화 
	lcdInitHW(); 
	// LCD function set
	lcdControlWrite(LCD_FUNCTION_DEFAULT);
	// clear LCD
	lcdControlWrite(1<<LCD_CLR);
	while(i--);	// wait 60ms
	// set entry mode
	lcdControlWrite(1<<LCD_ENTRY_MODE | 1<<LCD_ENTRY_INC);
	// set display to on
	//lcdControlWrite(1<<LCD_ON_CTRL | 1<<LCD_ON_DISPLAY | 1<<LCD_ON_BLINK);
	lcdControlWrite(1<<LCD_ON_CTRL | 1<<LCD_ON_DISPLAY );
	// move cursor to home
	lcdControlWrite(1<<LCD_HOME);
	// set data address to 0
	lcdControlWrite(1<<LCD_DDRAM | 0x00);
}

void lcdHome(void)
{
	// move cursor to home
	lcdControlWrite(1<<LCD_HOME);
} 

void lcdClear(void)
{
	// clear LCD
	lcdControlWrite(1<<LCD_CLR);
}

void lcdGotoXY(u8 x, u8 y)
{
	 u8 DDRAMAddr;

	// remap lines into proper order
	switch(y)
	{
	case 0: DDRAMAddr = LCD_LINE0_DDRAMADDR+x; break;
	case 1: DDRAMAddr = LCD_LINE1_DDRAMADDR+x; break;
	case 2: DDRAMAddr = LCD_LINE2_DDRAMADDR+x; break;
	case 3: DDRAMAddr = LCD_LINE3_DDRAMADDR+x; break;
	default: DDRAMAddr = LCD_LINE0_DDRAMADDR+x;
	}

	// set data address
	lcdControlWrite(1<<LCD_DDRAM | DDRAMAddr);
}



void lcdPrintData(char* data, u8 nBytes)
{
	 u8 i;

	// check to make sure we have a good pointer
	if (!data) return;

	// print data
	for(i=0; i<nBytes; i++)
	{
		lcdDataWrite(data[i]);
	}
}

void lcdPrint(char* data)
{
	 u8 i;

	// check to make sure we have a good pointer
	if (!data) return;

	// print data
	for(i=0; data[i]!='\0'; i++)
	{
		lcdDataWrite(data[i]);
	}
}
