#include"key_pad.h"

u8 cnt=0;
u8 pass[4]={0};
u8 _loop=1;

u8 PASSWD[]="1234";



void init_keypad(void){
  
  GPIO_InitTypeDef GPIO_InitStructure;
        
  RCC_APB2PeriphClockCmd(ROW_DDR | COLUMN_DDR, ENABLE);  

  GPIO_InitStructure.GPIO_Pin = KEY_1 | KEY_2 | KEY_3;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_Init(COLUMN_PORT, &GPIO_InitStructure);
  
  GPIO_InitStructure.GPIO_Pin = ROW_1 | ROW_2 | ROW_3| ROW_4;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 
  GPIO_Init(ROW_PORT, &GPIO_InitStructure);

  lcdInit();

  lcdGotoXY(0,0);
  lcdPrintData("PASSWORD?",sizeof("PASSWORD?")-1);	
  lcdGotoXY(0,1);
  lcdDataWrite('>');
  
  Timer3_Delay_init();
}




void back_pass(void){	//현재 커서에서 그전 위치의 패스워드를 정정
	
  if(cnt!=0){		
    lcdGotoXY(cnt,1);
    lcdDataWrite(' ');
    lcdGotoXY(cnt,1);
    cnt--;
  }
  while(GPIO_ReadInputData(COLUMN_PORT)&COLUMN_MASK);     

	
}


void reset_check(void){	// 패스워드 체크 리셋
	
  lcdClear();
  delay_ms(10);
  lcdGotoXY(0,0);
  lcdPrintData("Reset!!",sizeof("Reset!!")-1);
  delay_ms(1000);
  lcdGotoXY(0,0);
  lcdPrintData("PASSWORD?",sizeof("PASSWORD?")-1);
  lcdGotoXY(0,1);
  lcdDataWrite('>');

  while(GPIO_ReadInputData(COLUMN_PORT)&COLUMN_MASK);
	
  cnt=0;	
}


void passcheck(u8 data){	// 4자리의 패스워드가 맞는지 확인하는 함수
	
  if(cnt!=3){	//4자리입력이 아닌경우
	lcdDataWrite('*');
	pass[cnt++]=data;	
  }
  else if(cnt==3){
	lcdDataWrite('*');
	pass[cnt]=data;

	if(passwd_check(pass)!=0){	// 비밀번호 1234 와 다른 경우
		lcdGotoXY(0,0);
		lcdPrintData("WrongPassWord!",sizeof("WrongPassWord!")-1);
		lcdGotoXY(0,1);
		lcdPrintData("Plz Check U Pass",sizeof("Plz Check U Pass")-1);
		
		cnt=0;
		delay_ms(1000);
		lcdClear();
		delay_ms(60);
		lcdPrintData("PASSWORD?",sizeof("PASSWORD?")-1);
		lcdGotoXY(0,1);
		lcdDataWrite('>');
	}
	else{	//패스워드가 맞는경우
		lcdClear();
		lcdGotoXY(0,0);
		lcdPrintData("Hello MCU_WORLD",sizeof("Hello MCU_WORLD")-1);
		cnt=0;
		_loop=0;
		delay_ms(2);

	}

  }
  while(GPIO_ReadInputData(COLUMN_PORT)&COLUMN_MASK);
	
}

u8 get_passwd(void){
	//ROW_PORT	=	ROW_1;
        GPIO_ResetBits(ROW_PORT,ROW_KEY);
        GPIO_SetBits(ROW_PORT,ROW_1);
        	
	if(GPIO_ReadInputDataBit(COLUMN_PORT,KEY_1)){
		passcheck('1');
		}
	else if(GPIO_ReadInputDataBit(COLUMN_PORT,KEY_2)){
		passcheck('2');
		}
	else if(GPIO_ReadInputDataBit(COLUMN_PORT,KEY_3)){
		passcheck('3');
		}
	
	delay_ms(1);	
	
	 GPIO_ResetBits(ROW_PORT,ROW_KEY);
         GPIO_SetBits(ROW_PORT,ROW_2);

	if(GPIO_ReadInputDataBit(COLUMN_PORT,KEY_1)){		
		passcheck('4');
		}
	else if(GPIO_ReadInputDataBit(COLUMN_PORT,KEY_2)){		
		passcheck('5');
		}
	else if(GPIO_ReadInputDataBit(COLUMN_PORT,KEY_3)){		
		passcheck('6');
		}
	
	delay_ms(1);	
	
	GPIO_ResetBits(ROW_PORT,ROW_KEY);
        GPIO_SetBits(ROW_PORT,ROW_3);

	if(GPIO_ReadInputDataBit(COLUMN_PORT,KEY_1)){		
		passcheck('7');
		}
	else if(GPIO_ReadInputDataBit(COLUMN_PORT,KEY_2)){		
		passcheck('8');
		}
	else if(GPIO_ReadInputDataBit(COLUMN_PORT,KEY_3)){		
		passcheck('9');
		}

	delay_ms(1);	
	
	GPIO_ResetBits(ROW_PORT,ROW_KEY);
        GPIO_SetBits(ROW_PORT,ROW_4);
	delay_ms(1);

	if(GPIO_ReadInputDataBit(COLUMN_PORT,KEY_1)){		
		reset_check();
		}
	else if(GPIO_ReadInputDataBit(COLUMN_PORT,KEY_2)){		
		passcheck('0');
		}
	else if(GPIO_ReadInputDataBit(COLUMN_PORT,KEY_3)){		
		back_pass();
		}
	delay_ms(1);	
                
	return _loop;
}

u8 passwd_check(u8 pass[]){
	u8 _error=0;	
	
	u8 i;
	for(i=0;i<4;i++){
	if(pass[i]!=PASSWD[i])
		_error++;
	}
	return _error;
	
}


