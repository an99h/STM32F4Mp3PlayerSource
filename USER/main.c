/**
  ******************************************************************************
  * @file    mp3_play/main.c
  * @author  ANG
  * @version V0.0.1
  * @date    2015��11��1��
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

//��¼��ǰʹ�õ���TF������U�� Ĭ����ERROR
#define USE_ERROR		0
#define USE_USB_OTG		1
#define USE_TF_CARD		2
u8 mount_flage = USE_ERROR;

//�ж����ֲ����Ƿ�ɹ� Ĭ��ERROR
#define STATUS_OK		1
#define STATUS_ERROR	0
u8 music_status = STATUS_ERROR;

//��ǰ����ʱ��
u8 cur_min = 0;
u8 cur_sec = 0;

//OS_EVENT * msg_key;			//���������¼���ָ��
u8 pause = 0;					//��ͣ/���ű�־λ
FATFS my_fs;
FIL my_file;
USB_OTG_CORE_HANDLE          USB_OTG_Core;
USBH_HOST                    USB_Host;

__IO uint8_t RepeatState = 0;
extern __IO uint8_t LED_Toggle;

//START ����
//�����������ȼ�
#define START_TASK_PRIO						10
//���������ջ��С
#define START_STK_SIZE						64
//�����ջ
OS_STK START_TASK_STK[START_STK_SIZE];
//������
void start_task(void *pdata);

#if 1
//USB����
#define USB_TASK_PRIO						9
#define USB_STK_SIZE						512
OS_STK USB_TASK_STK[USB_STK_SIZE];
void usb_task(void *pdata);
#endif

#if 1
//TF������
#define TF_TASK_PRIO						8
#define TF_STK_SIZE							512
OS_STK TF_TASK_STK[TF_STK_SIZE];
void tf_task(void *pdata);
#endif


//UI����
#define UI_TASK_PRIO						7
#define UI_STK_SIZE							512
OS_STK UI_TASK_STK[UI_STK_SIZE];
void ui_task(void *pdata);


#if 1
//LED����
#define LED_TASK_PRIO						6
#define LED_STK_SIZE						64
OS_STK LED_TASK_STK[LED_STK_SIZE];
void led_task(void *pdata);
#endif

#if 1
//��������
#define TEST_TASK_PRIO						5
#define TEST_STK_SIZE						512
OS_STK TEST_TASK_STK[TEST_STK_SIZE];
void test_task(void *pdata);
#endif


//�����������ڳ�ʼ��Ӳ��״̬
int main(void)
{
	KEY_PRESS key;
	SysTick_Config(SystemCoreClock / 1000);
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);	//�жϷ���
	delay_init(168);
	uart_init(115200);
	KEY_Init();
	LED_Init();
	LCD_Init();
	LCD_Clear(BLACK);
	Show_Str(10,50,BLUE,BLACK,"���տƼ�ѧԺ",32,0);
	Show_Str(60,90,BLUE,BLACK,"������Ϣ����122��",24,0);
	Show_Str(130,125,BLUE,BLACK,"��ҵ��ƴ��",24,0);
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

//��ʼ���� ���ڴ�����������
void start_task(void *pdata)
{
	OS_CPU_SR cpu_sr = 0;
	pdata = pdata;

//	msg_key = OSMboxCreate((void*)0);			//������Ϣ����
	OSStatInit();								//��ʼ��ͳ������.�������ʱ1��������
	OS_ENTER_CRITICAL();						//�����ٽ���(�޷����жϴ��)
	OSTaskCreate(led_task,(void *)0,(OS_STK*)&LED_TASK_STK[LED_STK_SIZE-1],LED_TASK_PRIO);
	OSTaskCreate(ui_task,(void *)0,(OS_STK*)&UI_TASK_STK[UI_STK_SIZE-1],UI_TASK_PRIO);
	OSTaskCreate(usb_task,(void *)0,(OS_STK*)&USB_TASK_STK[USB_STK_SIZE-1],USB_TASK_PRIO);
	OSTaskCreate(tf_task,(void *)0,(OS_STK*)&TF_TASK_STK[TF_STK_SIZE-1],TF_TASK_PRIO);
	OSTaskSuspend(START_TASK_PRIO);				//������ʼ����.
	OS_EXIT_CRITICAL();							//�˳��ٽ���(���Ա��жϴ��)
	OSTaskDel(OS_PRIO_SELF);					//��ʼ����ʹ����� ɾ����������
}

//UI�߿���ʾ���ļ��б���ʾ
void ui_task(void *pdata)
{
	display_ui_start();						//��ʼUI

	delay_ms(1000);
	if(mount_flage == USE_TF_CARD)
	{
		//delay_ms(2000);
		display_files_list("1:/MUSIC");		//��ʾTF��/MUSICĿ¼�µ������б�
		OSTaskDel(TF_TASK_PRIO);
	}
	else if(mount_flage == USE_USB_OTG)
	{
		delay_ms(1000);
		display_files_list("0:/MUSIC");		//��ʾU��/MUSICĿ¼�µ������б�
		OSTaskDel(USB_TASK_PRIO);
	}

	if(music_status == STATUS_ERROR)		//��ʾδ����TF����U����ʾ��
	{
		LCD_Fill(1,21,319,219,BLACK);
		OSTaskDel(LED_TASK_PRIO);
		Show_Str(20,100,RED,BLACK,"�����TF����U��Ȼ�󰴸�λ����������",16,0);
		while(1);
	}

	OSTaskDel(LED_TASK_PRIO);
	display_ui_end();						//����UI
	OSTaskCreate(test_task,(void *)0,(OS_STK*)&TEST_TASK_STK[TEST_STK_SIZE-1],TEST_TASK_PRIO);
	OSTaskDel(OS_PRIO_SELF);
}

//����1:/MUSICĿ¼�µ������ļ�
void tf_task(void *pdata)
{
	u8 temp = 0;
	while(SD_Initialize() != 0)
	{
		temp++;
		if(temp == 3)
		{
			mount_flage = USE_USB_OTG;
			OSTaskDel(OS_PRIO_SELF);	//TF��û�г�ʼ���ɹ� ɾ����������
			return;
		}
		delay_ms(10);
	}

	if(f_mount(1,&my_fs) == FR_OK)
	{
		OSTaskDel(USB_TASK_PRIO);		//TF�����سɹ� ɾ��USB����
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
		OSTaskDel(OS_PRIO_SELF);		//TF��û�й��سɹ� ɾ����������
	}
}

#if 1
//LED����Ŀǰֻ��Ϊ����ʾcupʹ���ʺ͵�ǰ���ֲ���ʱ��
void led_task(void *pdata)
{
//	u8 x,y;							//x:0/1		y:LEDy
//	u16 z;							//	z:��ʱms
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

//����0:/MUSICĿ¼�µ������ļ�
void usb_task(void *pdata)
{
#if 1
	while(1)
	{
		RepeatState = 0;
		//��ʼ��USB
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


//��ʾMP3�ļ��б�
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
		music_status = STATUS_OK;					//���ļ��гɹ�����״̬OK����ERROR
		//��¼�����ļ�����ֵ
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
			flage_page = curindex;					//��ǰҳ���ļ��б��һ������λ��
			while(1)
			{
				dir_seek(&dir,mp3indextbl[curindex]);//�ı䵱ǰĿ¼����
				res = f_readdir(&dir, &fno);
				if (res != FR_OK || fno.fname[0] == 0)break;
				fn = fno.fname;
				if(curindex == music_sum)
				{
					//Mp3PlayerPauseResume(1);
					temp = 30;
					break;
				}
				music_curindex = get_file_curindex();		//��ȡ��ǰ���ڲ��ŵĸ�������ֵλ��
				if(music_curindex != curindex)
					Show_Str(216,temp,WHITE,BLACK,(u8*)fn,16,0);
				else
				{
					LCD_ShowString(200,temp,16,">>",1);
					Show_Str(216,temp,WHITE,BLACK,(u8*)fn,16,0);
				}
				temp += 16;
				curindex++;

				//��ʾ����ҳ������
				display_scrollbar((curindex-1)/10+1,(music_sum-1)/10+1);

				i++;
				if(i == 11)
				{
					temp = 30;
					break;
				}
			}//end while(1)
			//Mp3PlayerPauseResume(1);
			//�ļ��б�����ֿ���
			res = list_ctrl(&curindex,&i,music_sum,flage_page);
			if(res == 1)return;

		}// end while(res == FR_OK)
	}//end if(f_opendir(&dir,path) == FR_OK)
	free(fn);
}
//�ļ��б�����ֿ���
u8 list_ctrl(u8* curindex,u8* i,u8 music_sum,u8 flage_page)
{
	KEY_PRESS key;
	while(1)
	{
		delay_ms(20);
		key = KEY_Scan(0);
		if(key == PAGE_DOWN)					//��һҳ
		{
			//Mp3PlayerPauseResume(0);
			if(*curindex >= music_sum)
				*curindex = flage_page;
			*i = 1;
			LCD_Fill(200,21,310,219,BLACK);
			break;
		}
		else if(key == PAGE_UP)					//��һҳ
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
				Mp3PlayerPauseResume(pause);	//��ͣ����
				pause = 1;
			}
			else
			{
				Gui_icon(84,140,gImage_play);
				Mp3PlayerPauseResume(pause);	//��ʼ����
				pause = 0;
			}
		}
		else if(key == STOP_SONG)				//ֹͣ
		{
			Mp3PlayerStop();
			return 1;
		}
	}//end while(1)
	return 0;
}
//��ʾ�ļ��б����¹�����
void display_scrollbar(u8 now_page, u8 sum_page)
{
	u8 height = 180/sum_page;			//����������
	LCD_Fill(312,30,319,210,GRAY);
	LCD_DrawFillRectangle(312,30+height*(now_page-1),319,30+height*now_page,BLUE);
}

//��ʾ��ǰ����ʱ��
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

//��ʾ��ʼUI
void display_ui_start(void)
{
	DrawPage("���ֲ�����");
	POINT_COLOR = WHITE;

	LCD_DrawRectangle(0,20,319,219);
	LCD_DrawLine(195,20,195,219);
	LCD_DrawLine(0,54,195,54);				//�ָ���
	LCD_DrawLine(0,170,195,170);

	LCD_DrawRectangle(311,20,319,219);
	LCD_DrawLine(311,29,319,29);			//�������߿�
	LCD_DrawLine(311,210,319,210);
	LCD_ShowString(311,23,16,"^",1);
	LCD_ShowString(312,208,12,"v",1);

	LCD_ShowString(80,30,16,"VOL:50",1);
	LCD_ShowString(16,30,16,"CPU:  %",1);	//״̬��ʾ
	LCD_ShowString(136,30,16,"MODE:O",1);
	LCD_ShowString(15,123,12,"00:00",1);

	Gui_icon(35,140,gImage_last);
	Gui_icon(84,140,gImage_play);			//��һ������һ������ͣͼ��
	Gui_icon(133,140,gImage_next);
}
//��ʾ����UI
void display_ui_end(void)
{

	LCD_Clear(BLACK);
	DrawPage("�豸��Ϣ");
	LCD_DrawRectangle(0,20,319,219);
	Show_Str(35,60,WHITE,BLACK, "  ����оƬ   : STM32F407VGT6",16,0);
	Show_Str(35,80,WHITE,BLACK, "  �����     : ST-LINK/V2",16,0);
	Show_Str(35,100,WHITE,BLACK,"  DACоƬ    : CS43L22",16,0);
	Show_Str(35,120,WHITE,BLACK,"  LCD�ֱ���  : 320*240",16,0);
	Show_Str(35,140,WHITE,BLACK,"  ����汾�� : 2016.05.07",16,0);
	Show_Str(35,160,WHITE,BLACK,"  Designed by ANG",16,0);
}


//�л�����ʱ����ʱ��
void reset_time(void)
{
	LCD_Fill(21,123,26,135,BLACK);
	LCD_ShowNum(21,123,0,1,12);
	cur_min = 0;
	cur_sec = 0;
}
/********************************** 2016 ***** ANG ***** END OF FILE **********************************/
