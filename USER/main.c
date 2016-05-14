/**
  ******************************************************************************
  * @file    mp3_play/main.c
  * @author  ANG
  * @version V0.0.1
  * @date    2015年11月1日
  * @brief   Main program body
  ******************************************************************************
  * @attention
  *
  *
  *
  */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "led.h"
#include "lcd.h"
#include "sd.h"
#include "delay.h"
#include "usart.h"
#include "key.h"
#include "includes.h"
#include "exfuns.h"
#include "icon.h"
#include <string.h>

//记录当前使用的是TF卡还是U盘 默认是ERROR
#define USE_ERROR		0
#define USE_USB_OTG		1
#define USE_TF_CARD		2
u8 mount_flage = USE_ERROR;

//判断音乐播放是否成功 默认ERROR
#define STATUS_OK		1
#define STATUS_ERROR	0
u8 music_status = STATUS_ERROR;

//当前播放时间
u8 cur_min = 0;
u8 cur_sec = 0;

//OS_EVENT * msg_key;			//按键邮箱事件块指针
u8 pause = 0;					//暂停/播放标志位
FATFS my_fs;
FIL my_file;
USB_OTG_CORE_HANDLE          USB_OTG_Core;
USBH_HOST                    USB_Host;

__IO uint8_t RepeatState = 0;
extern __IO uint8_t LED_Toggle;

//START 任务
//设置任务优先级
#define START_TASK_PRIO						10
//设置任务堆栈大小
#define START_STK_SIZE						64
//任务堆栈
OS_STK START_TASK_STK[START_STK_SIZE];
//任务函数
void start_task(void *pdata);

#if 1
//USB任务
#define USB_TASK_PRIO						9
#define USB_STK_SIZE						512
OS_STK USB_TASK_STK[USB_STK_SIZE];
void usb_task(void *pdata);
#endif

#if 1
//TF卡任务
#define TF_TASK_PRIO						8
#define TF_STK_SIZE							512
OS_STK TF_TASK_STK[TF_STK_SIZE];
void tf_task(void *pdata);
#endif


//UI任务
#define UI_TASK_PRIO						7
#define UI_STK_SIZE							512
OS_STK UI_TASK_STK[UI_STK_SIZE];
void ui_task(void *pdata);


#if 1
//LED任务
#define LED_TASK_PRIO						6
#define LED_STK_SIZE						64
OS_STK LED_TASK_STK[LED_STK_SIZE];
void led_task(void *pdata);
#endif

#if 1
//测试任务
#define TEST_TASK_PRIO						5
#define TEST_STK_SIZE						512
OS_STK TEST_TASK_STK[TEST_STK_SIZE];
void test_task(void *pdata);
#endif


//主函数，用于初始化硬件状态
int main(void)
{
	KEY_PRESS key;
	SysTick_Config(SystemCoreClock / 1000);
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);	//中断分组
	delay_init(168);
	uart_init(115200);
	KEY_Init();
	LED_Init();
	LCD_Init();
	LCD_Clear(BLACK);
	Show_Str(10,50,BLUE,BLACK,"安徽科技学院",32,0);
	Show_Str(60,90,BLUE,BLACK,"电子信息工程122班",24,0);
	Show_Str(130,125,BLUE,BLACK,"毕业设计答辩",24,0);
	LCD_ShowString(220,175,16,"2016-05-07",0);
	while(1)
	{
		delay_ms(20);
		key = KEY_Scan(0);
		if(key != NO_KEY_PRESS)
			break;
	}
	OSInit();
	OSTaskCreate(start_task,(void *)0,(OS_STK *)&START_TASK_STK[START_STK_SIZE-1],START_TASK_PRIO );
	OSStart();
}

//开始任务 用于创建其他任务
void start_task(void *pdata)
{
	OS_CPU_SR cpu_sr = 0;
	pdata = pdata;

//	msg_key = OSMboxCreate((void*)0);			//创建消息邮箱
	OSStatInit();								//初始化统计任务.这里会延时1秒钟左右
	OS_ENTER_CRITICAL();						//进入临界区(无法被中断打断)
	OSTaskCreate(led_task,(void *)0,(OS_STK*)&LED_TASK_STK[LED_STK_SIZE-1],LED_TASK_PRIO);
	OSTaskCreate(ui_task,(void *)0,(OS_STK*)&UI_TASK_STK[UI_STK_SIZE-1],UI_TASK_PRIO);
	OSTaskCreate(usb_task,(void *)0,(OS_STK*)&USB_TASK_STK[USB_STK_SIZE-1],USB_TASK_PRIO);
	OSTaskCreate(tf_task,(void *)0,(OS_STK*)&TF_TASK_STK[TF_STK_SIZE-1],TF_TASK_PRIO);
	OSTaskSuspend(START_TASK_PRIO);				//挂起起始任务.
	OS_EXIT_CRITICAL();							//退出临界区(可以被中断打断)
	OSTaskDel(OS_PRIO_SELF);					//起始任务使命完成 删除自身任务
}

//UI边框显示和文件列表显示
void ui_task(void *pdata)
{
	display_ui_start();						//开始UI

	delay_ms(1000);
	if(mount_flage == USE_TF_CARD)
	{
		//delay_ms(2000);
		display_files_list("1:/MUSIC");		//显示TF卡/MUSIC目录下的音乐列表
		OSTaskDel(TF_TASK_PRIO);
	}
	else if(mount_flage == USE_USB_OTG)
	{
		delay_ms(1000);
		display_files_list("0:/MUSIC");		//显示U盘/MUSIC目录下的音乐列表
		OSTaskDel(USB_TASK_PRIO);
	}

	if(music_status == STATUS_ERROR)		//显示未插入TF卡或U盘提示语
	{
		LCD_Fill(1,21,319,219,BLACK);
		OSTaskDel(LED_TASK_PRIO);
		Show_Str(20,100,RED,BLACK,"请插入TF卡或U盘然后按复位键重启机器",16,0);
		while(1);
	}

	OSTaskDel(LED_TASK_PRIO);
	display_ui_end();						//结束UI
	OSTaskCreate(test_task,(void *)0,(OS_STK*)&TEST_TASK_STK[TEST_STK_SIZE-1],TEST_TASK_PRIO);
	OSTaskDel(OS_PRIO_SELF);
}

//播放1:/MUSIC目录下的音乐文件
void tf_task(void *pdata)
{
	u8 temp = 0;
	while(SD_Initialize() != 0)
	{
		temp++;
		if(temp == 3)
		{
			mount_flage = USE_USB_OTG;
			OSTaskDel(OS_PRIO_SELF);	//TF卡没有初始化成功 删除自身任务
			return;
		}
		delay_ms(10);
	}

	if(f_mount(1,&my_fs) == FR_OK)
	{
		OSTaskDel(USB_TASK_PRIO);		//TF卡挂载成功 删除USB任务
		mount_flage = USE_TF_CARD;
		delay_ms(500);
		get_TF_free();
		Mp3PlayerStart("1:/MUSIC");
		OSTaskDel(UI_TASK_PRIO);
		OSTaskDel(LED_TASK_PRIO);
		OSTaskDel(OS_PRIO_SELF);
	}
	else
	{
		mount_flage = USE_USB_OTG;
		OSTaskDel(OS_PRIO_SELF);		//TF卡没有挂载成功 删除自身任务
	}
}

#if 1
//LED任务，目前只是为了显示cup使用率和当前音乐播放时间
void led_task(void *pdata)
{
//	u8 x,y;							//x:0/1		y:LEDy
//	u16 z;							//	z:延时ms
//	srand(6);
	delay_ms(2000);
	while(1)
	{
//		x = rand()%2;
//		y = rand()%4;
//		y = y + 1;
//		z = rand()%500;
//		switch(y)
//		{
//		case 1:
//			LED1 = x;
//			delay_ms(z);
//			break;
//		case 2:
//			LED2 = x;
//			delay_ms(z);
//			break;
//		case 3:
//			LED3 = x;
//			delay_ms(z);
//			break;
//		case 4:
//			LED4 = x;
//			delay_ms(z);
//			break;
//		default:
//			break;
//		}
		LCD_ShowNum(16+8*4,30,OSCPUUsage,2,16);
		delay_ms(1000);
		if(music_status == STATUS_OK)
		{
			if(!pause)
			{
				display_time();
			}
		}
	}
}
#endif

//播放0:/MUSIC目录下的音乐文件
void usb_task(void *pdata)
{
#if 1
	while(1)
	{
		RepeatState = 0;
		//初始化USB
		USBH_Init(&USB_OTG_Core, USB_OTG_FS_CORE_ID, &USB_Host, &USBH_MSC_cb, &USR_Callbacks);
		while (1)
		{
			USBH_Process(&USB_OTG_Core, &USB_Host);
		}
	}
#endif
}

void test_task(void *pdata)
{
	KEY_PRESS key;
	u8 temp = 0;
	while(1)
	{
		key = KEY_Scan(0);
		delay_ms(20);
		switch(key)
		{
		case NEXT_SONG:
			temp++;
			if(temp == 2)
				ProductTest_main();
			break;
		case NO_KEY_PRESS:
			break;
		default:
			temp = 0;
		}
	}
}


//显示MP3文件列表
void display_files_list(char * path)
{
	u8 res = 0;
	u8 temp = 0;
	u8 flage_page = 0;
	u8 mp3indextbl[40];
	u8 curindex = 0;
	u8 music_sum = 0;
	u8 i = 1;
	u8 music_curindex = 0;
	char *fn = NULL;
	FILINFO fno;

	fn = (char *)malloc(sizeof(fno.fname));
	if(f_opendir(&dir,path) == FR_OK)
	{
		music_status = STATUS_OK;					//打开文件夹成功代表状态OK否则ERROR
		//记录所有文件索引值
		while(1)
		{
			temp = dir.index;
			res = f_readdir(&dir,&fno);
			if(res != FR_OK||fno.fname[0] == 0)break;
			fn = fno.fname;
			if(strstr(fn,".mp3") != NULL)
			{
				mp3indextbl[curindex++] = temp;
				music_sum++;
			}
		}

		//delay_ms(2000);
		fn = NULL;
		curindex = 0;
		temp = 30;
		res = f_opendir(&dir,(const XCHAR*)path);
		while(res == FR_OK)
		{
			flage_page = curindex;					//当前页面文件列表第一个索引位置
			while(1)
			{
				dir_seek(&dir,mp3indextbl[curindex]);//改变当前目录索引
				res = f_readdir(&dir, &fno);
				if (res != FR_OK || fno.fname[0] == 0)break;
				fn = fno.fname;
				if(curindex == music_sum)
				{
					//Mp3PlayerPauseResume(1);
					temp = 30;
					break;
				}
				music_curindex = get_file_curindex();		//获取当前正在播放的歌曲索引值位置
				if(music_curindex != curindex)
					Show_Str(216,temp,WHITE,BLACK,(u8*)fn,16,0);
				else
				{
					LCD_ShowString(200,temp,16,">>",1);
					Show_Str(216,temp,WHITE,BLACK,(u8*)fn,16,0);
				}
				temp += 16;
				curindex++;

				//显示上下页滚动条
				display_scrollbar((curindex-1)/10+1,(music_sum-1)/10+1);

				i++;
				if(i == 11)
				{
					temp = 30;
					break;
				}
			}//end while(1)
			//Mp3PlayerPauseResume(1);
			//文件列表和音乐控制
			res = list_ctrl(&curindex,&i,music_sum,flage_page);
			if(res == 1)return;

		}// end while(res == FR_OK)
	}//end if(f_opendir(&dir,path) == FR_OK)
	free(fn);
}
//文件列表和音乐控制
u8 list_ctrl(u8* curindex,u8* i,u8 music_sum,u8 flage_page)
{
	KEY_PRESS key;
	while(1)
	{
		delay_ms(20);
		key = KEY_Scan(0);
		if(key == PAGE_DOWN)					//下一页
		{
			//Mp3PlayerPauseResume(0);
			if(*curindex >= music_sum)
				*curindex = flage_page;
			*i = 1;
			LCD_Fill(200,21,310,219,BLACK);
			break;
		}
		else if(key == PAGE_UP)					//上一页
		{
			//Mp3PlayerPauseResume(0);
			if(*curindex == 10)
				*curindex = 0;
			else
				*curindex -= *i + 9;
			*i = 1;
			LCD_Fill(200,21,310,219,BLACK);
			break;
		}
		else if(key == PAUSE_SONG)
		{
			if(pause == 0)
			{
				Gui_icon(84,140,gImage_pause);
				Mp3PlayerPauseResume(pause);	//暂停播放
				pause = 1;
			}
			else
			{
				Gui_icon(84,140,gImage_play);
				Mp3PlayerPauseResume(pause);	//开始播放
				pause = 0;
			}
		}
		else if(key == STOP_SONG)				//停止
		{
			Mp3PlayerStop();
			return 1;
		}
	}//end while(1)
	return 0;
}
//显示文件列表上下滚动条
void display_scrollbar(u8 now_page, u8 sum_page)
{
	u8 height = 180/sum_page;			//滚动条长度
	LCD_Fill(312,30,319,210,GRAY);
	LCD_DrawFillRectangle(312,30+height*(now_page-1),319,30+height*now_page,BLUE);
}

//显示当前播放时间
void display_time(void)
{
	cur_sec++;
	if(cur_sec == 60)
	{
		cur_sec = 0;
		cur_min++;
	}

	if(cur_sec < 10)
	{
		LCD_Fill(33,123,38,134,BLACK);
		LCD_ShowNum(21,123,cur_min,1,12);
		LCD_ShowString(33,123,12,"0",1);
		LCD_ShowNum(39,123,cur_sec,1,12);
	}
	else
	{
		LCD_ShowNum(21,123,cur_min,1,12);
		LCD_ShowNum(33,123,cur_sec,2,12);
	}
}

//显示开始UI
void display_ui_start(void)
{
	DrawPage("音乐播放器");
	POINT_COLOR = WHITE;

	LCD_DrawRectangle(0,20,319,219);
	LCD_DrawLine(195,20,195,219);
	LCD_DrawLine(0,54,195,54);				//分割线
	LCD_DrawLine(0,170,195,170);

	LCD_DrawRectangle(311,20,319,219);
	LCD_DrawLine(311,29,319,29);			//滚动条边框
	LCD_DrawLine(311,210,319,210);
	LCD_ShowString(311,23,16,"^",1);
	LCD_ShowString(312,208,12,"v",1);

	LCD_ShowString(80,30,16,"VOL:50",1);
	LCD_ShowString(16,30,16,"CPU:  %",1);	//状态提示
	LCD_ShowString(136,30,16,"MODE:O",1);
	LCD_ShowString(15,123,12,"00:00",1);

	Gui_icon(35,140,gImage_last);
	Gui_icon(84,140,gImage_play);			//上一曲，下一曲，暂停图标
	Gui_icon(133,140,gImage_next);
}
//显示结束UI
void display_ui_end(void)
{

	LCD_Clear(BLACK);
	DrawPage("设备信息");
	LCD_DrawRectangle(0,20,319,219);
	Show_Str(35,60,WHITE,BLACK, "  主控芯片   : STM32F407VGT6",16,0);
	Show_Str(35,80,WHITE,BLACK, "  编程器     : ST-LINK/V2",16,0);
	Show_Str(35,100,WHITE,BLACK,"  DAC芯片    : CS43L22",16,0);
	Show_Str(35,120,WHITE,BLACK,"  LCD分辨率  : 320*240",16,0);
	Show_Str(35,140,WHITE,BLACK,"  编译版本号 : 2016.05.07",16,0);
	Show_Str(35,160,WHITE,BLACK,"  Designed by ANG",16,0);
}


//切换歌曲时重置时间
void reset_time(void)
{
	LCD_Fill(21,123,26,135,BLACK);
	LCD_ShowNum(21,123,0,1,12);
	cur_min = 0;
	cur_sec = 0;
}
/********************************** 2016 ***** ANG ***** END OF FILE **********************************/
