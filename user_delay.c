#include"user_delay.h"

void Timer3_Delay_init(){
  
  TIM_TimeBaseInitTypeDef TIM3_TimeBaseInitStruct;
  
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3,ENABLE);
 
  TIM3_TimeBaseInitStruct.TIM_Prescaler = 7200 -1;  
  TIM3_TimeBaseInitStruct.TIM_CounterMode = TIM_CounterMode_Up;
  TIM3_TimeBaseInitStruct.TIM_Period = 10-1;  
  TIM3_TimeBaseInitStruct.TIM_ClockDivision = TIM_CKD_DIV1;
  TIM_TimeBaseInit(TIM3,&TIM3_TimeBaseInitStruct);
  
}

void delay_ms(u16 time){
  u16 i; 
  TIM_Cmd(TIM3,ENABLE);    
  for(i=0;i<time;i++){
     while(TIM_GetFlagStatus(TIM3,TIM_IT_Update)==RESET);
     TIM_ClearFlag(TIM3,TIM_FLAG_Update);
  }
  TIM_Cmd(TIM3,DISABLE);

}

