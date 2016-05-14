#include "key.h"
#include "delay.h" 
#include "usart.h"


//按键初始化函数
void KEY_Init(void)
{	
	GPIO_InitTypeDef  GPIO_InitStructure;
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA|RCC_AHB1Periph_GPIOD|RCC_AHB1Periph_GPIOB, ENABLE);//使能GPIOA时钟
 
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;    
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; 
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;  
	GPIO_Init(GPIOA, &GPIO_InitStructure); 				//初始化GPIOA Pin_0引脚
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_3|GPIO_Pin_5|GPIO_Pin_6|GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;    
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz; 
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(GPIOD, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;    
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz; 
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;  
	GPIO_Init(GPIOB, &GPIO_InitStructure); 
} 
//按键处理函数
//KEY_PRESS 返回按键值
//mode:0,不支持连续按;1,支持连续按;

#if 1
KEY_PRESS KEY_Scan(u8 mode)
{	 
	static u8 key_up = 1;//按键按松开标志
	if(mode)
		key_up = 1;  //支持连按		  
	if(key_up && (MUSIC_MODE_KEY == 1 || PAGE_DOWN_KEY == 0 || NEXT_SONG_KEY == 0 || VOL_DOWN_KEY == 0 || PAUSE_SONG_KEY == 0 || VOL_UP_KEY == 0 || LAST_SONG_KEY == 0 || PAGE_UP_KEY == 0 || STOP_SONG_KEY == 0))
	{
		delay_ms(10);//去抖动 
		key_up = 0;
		if(MUSIC_MODE_KEY == 1)
		{
			//printf("MUSIC_MODE\r\n");
			return MUSIC_MODE;
		}
		else if(PAGE_DOWN_KEY == 0)
		{
			//printf("PAGE_DOWN\r\n");
			return PAGE_DOWN;
		}
		else if(NEXT_SONG_KEY == 0)
		{
			//printf("NEXT_SONG\r\n");
			return NEXT_SONG;
		}
		else if(VOL_DOWN_KEY == 0)
		{
			//printf("VOL_DOWN\r\n");
			return VOL_DOWN;
		}
		else if(PAUSE_SONG_KEY == 0)
		{
			//printf("PAUSE_SONG\r\n");
			return PAUSE_SONG;
		}
		else if(VOL_UP_KEY == 0)
		{
			//printf("VOL_UP\r\n");
			return VOL_UP;
		}
		else if(LAST_SONG_KEY == 0)
		{
			//printf("LAST_SONG\r\n");
			return LAST_SONG;
		}
		else if(PAGE_UP_KEY == 0)
		{
			//printf("PAGE_UP\r\n");
			return PAGE_UP;
		}
		else if(STOP_SONG_KEY == 0)
		{
			//printf("STOP_SONG\r\n");
			return STOP_SONG;
		}
	}
	else if(MUSIC_MODE_KEY == 0 && PAGE_DOWN_KEY == 1 && NEXT_SONG_KEY == 1 && VOL_DOWN_KEY == 1 && PAUSE_SONG_KEY == 1 && VOL_UP_KEY == 1 && LAST_SONG_KEY == 1 && PAGE_UP_KEY == 1 && STOP_SONG_KEY == 1)
		key_up = 1;
 	return NO_KEY_PRESS;// 无按键按下
}
#endif




















