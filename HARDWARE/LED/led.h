#ifndef __LED_H
#define __LED_H
#include "sys.h"

//LED位操作		LED=1亮---LED=0灭
#define LED1 	PDout(12)	
#define LED2 	PDout(13)	
#define LED3 	PDout(14)	
#define LED4 	PDout(15)	

#define LED1_ON GPIO_SetBits(GPIOD,GPIO_Pin_12)
#define LED2_ON GPIO_SetBits(GPIOD,GPIO_Pin_13)
#define LED3_ON GPIO_SetBits(GPIOD,GPIO_Pin_14)
#define LED4_ON GPIO_SetBits(GPIOD,GPIO_Pin_15)
#define LED_ON GPIO_SetBits(GPIOD,GPIO_Pin_12|GPIO_Pin_13|GPIO_Pin_14|GPIO_Pin_15)

#define LED1_OFF GPIO_ResetBits(GPIOD,GPIO_Pin_12)
#define LED2_OFF GPIO_ResetBits(GPIOD,GPIO_Pin_13)
#define LED3_OFF GPIO_ResetBits(GPIOD,GPIO_Pin_14)
#define LED4_OFF GPIO_ResetBits(GPIOD,GPIO_Pin_15)
#define LED_OFF GPIO_ResetBits(GPIOD,GPIO_Pin_12|GPIO_Pin_13|GPIO_Pin_14|GPIO_Pin_15)

void LED_Init(void);//初始化		 				    
#endif
