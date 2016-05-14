#include "ProductTest.h"
KEY_PRESS key;

void key_test(void);
void led_test(void);
void lcd_test(void);
void usart_test(void);

void ProductTest_main(void)
{
	while(1)
	{
		DrawPage("����ϵͳ");
		Show_Str(40,60,BLUE,BLACK,"KEY1:��������",16,0);
		Show_Str(40,80,BLUE,BLACK,"KEY2:LED ����",16,0);
		Show_Str(40,100,BLUE,BLACK,"KEY3:LCD ����",16,0);
		Show_Str(40,120,BLUE,BLACK,"KEY4:���ڲ���",16,0);
		while(1)
		{
			key = KEY_Scan(0);
			delay_ms(20);
			switch(key)
			{
			case PAGE_DOWN:
				key_test();
				break;
			case VOL_UP:
				led_test();
				break;
			case VOL_DOWN:
				lcd_test();
				break;
			case PAGE_UP:
				usart_test();
				break;
			default:
				break;
			}
		}
	}
}
void key_test(void)
{
	u8 key0 = 0;
	u8 key1 = 0;
	u8 key2 = 0;
	u8 key3 = 0;
	u8 key4 = 0;
	u8 key5 = 0;
	u8 key6 = 0;
	u8 key7 = 0;
	u8 key8 = 0;
	DrawPage("��������");
	gui_circle(52,70,WHITE,20,0);
	gui_circle(52,170,WHITE,20,0);
	gui_circle(104,70,WHITE,20,0);
	gui_circle(104,170,WHITE,20,0);
	gui_circle(156,70,WHITE,20,0);
	gui_circle(156,170,WHITE,20,0);
	gui_circle(208,70,WHITE,20,0);
	gui_circle(208,170,WHITE,20,0);
	gui_circle(260,120,WHITE,20,0);
	while(1)
	{
		key = KEY_Scan(0);
		delay_ms(20);
		switch(key)
		{
		case MUSIC_MODE:
			gui_circle(260,120,WHITE,20,1);
			key0 = 1;
			break;
		case PAGE_DOWN:
			gui_circle(208,70,WHITE,20,1);
			key1 = 1;
			break;
		case NEXT_SONG:
			gui_circle(208,170,WHITE,20,1);
			key2 = 1;
			break;
		case VOL_UP:
			gui_circle(156,70,WHITE,20,1);
			key3 = 1;
			break;
		case PAUSE_SONG:
			gui_circle(156,170,WHITE,20,1);
			key4 = 1;
			break;
		case VOL_DOWN:
			gui_circle(104,70,WHITE,20,1);
			key5 = 1;
			break;
		case STOP_SONG:
			gui_circle(104,170,WHITE,20,1);
			key6 = 1;
			break;
		case PAGE_UP:
			gui_circle(52,70,WHITE,20,1);
			key7 = 1;
			break;
		case LAST_SONG:
			gui_circle(52,170,WHITE,20,1);
			key8 = 1;
			break;
		case NO_KEY_PRESS:
			if(key0 == 1 && key1 == 1 && key2 == 1 && key3 == 1 && key4 == 1 && key5 == 1 && key6 == 1 && key7 == 1 && key8 == 1)
			{
				delay_ms(2000);
				ProductTest_main();
			}
			break;
		default:
			break;
		}
	}
}

void led_test(void)
{
	u8 x,y;
	u16 z;
	srand(6);
	DrawPage("LED ����");
	gui_circle(156,70,WHITE,20,0);
	gui_circle(156,170,WHITE,20,0);
	gui_circle(208,120,WHITE,20,0);
	gui_circle(104,120,WHITE,20,0);
	while(1)
	{
		key = KEY_Scan(0);
		delay_ms(20);
		switch(key)
		{
		case PAGE_DOWN:
			gui_circle(104,120,GREEN,20,1);
			LED1 = 1;
			break;
		case VOL_UP:
			gui_circle(156,70,BRRED,20,1);
			LED2 = 1;
			break;
		case VOL_DOWN:
			gui_circle(208,120,RED,20,1);
			LED3 = 1;
			break;
		case PAGE_UP:
			gui_circle(156,170,BLUE,20,1);
			LED4 = 1;
			break;
		case NEXT_SONG:
			LCD_Fill(1,21,319,219,BLACK);
			gui_circle(156,170,WHITE,20,0);//4
			gui_circle(156,70,WHITE,20,0);//2
			gui_circle(208,120,WHITE,20,0);//3
			gui_circle(104,120,WHITE,20,0);//1
			LED1 = 0;
			LED2 = 0;
			LED3 = 0;
			LED4 = 0;
			break;
		case PAUSE_SONG:
			while(1)
			{
				x = rand()%2;
				y = rand()%4;
				y = y + 1;
				z = rand()%500;
				key = KEY_Scan(0);
				switch(key)
				{
				case PAUSE_SONG:
					LED_OFF;
					led_test();
					break;
				default:
					if(y == 1)
					{
						LED1 = x;
						if(x == 0)
							gui_circle(104,120,WHITE,20,1);
						else
							gui_circle(104,120,GREEN,20,1);
						delay_ms(z);
					}
					else if(y == 2)
					{
						LED2 = x;
						if(x == 0)
							gui_circle(156,70,WHITE,20,1);
						else
							gui_circle(156,70,BRRED,20,1);
						delay_ms(z);
					}
					else if(y == 3)
					{
						LED3 = x;
						if(x == 0)
							gui_circle(208,120,WHITE,20,1);
						else
							gui_circle(208,120,RED,20,1);
						delay_ms(z);
					}
					else if(y == 4)
					{
						LED4 = x;
						if(x == 0)
							gui_circle(156,170,WHITE,20,1);
						else
							gui_circle(156,170,BLUE,20,1);
						delay_ms(z);
					}
					break;
				}
			}
		case STOP_SONG:
			LED_ON;
			gui_circle(104,120,GREEN,20,1);
			gui_circle(156,70,BRRED,20,1);
			gui_circle(208,120,RED,20,1);
			gui_circle(156,170,BLUE,20,1);
			break;
		case LAST_SONG:
			LED_OFF;
			ProductTest_main();
			break;
		case MUSIC_MODE:
			break;
		default:
			break;
		}
	}
}

void lcd_test(void)
{
	u8 i=0;
	u16 ColorTab[5]= {BRED,YELLOW,RED,GREEN,BLUE};
	DrawPage("LCD ����");
	while(1)
	{
		key = KEY_Scan(0);
		delay_ms(20);
		switch(key)
		{
		case LAST_SONG:
			ProductTest_main();
		case NO_KEY_PRESS:
			break;
		case PAGE_DOWN:
			LCD_Fill(0,20,lcddev.width,lcddev.height-20,WHITE);
			delay_ms(1000);
			LCD_Fill(0,20,lcddev.width,lcddev.height-20,BLUE);
			delay_ms(1000);
			LCD_Fill(0,20,lcddev.width,lcddev.height-20,RED);
			delay_ms(1000);
			LCD_Fill(0,20,lcddev.width,lcddev.height-20,GREEN);
			delay_ms(1000);
			LCD_Fill(0,20,lcddev.width,lcddev.height-20,BLACK);
			delay_ms(1000);
			break;
		case VOL_UP:
			LCD_Fill(0,20,lcddev.width,lcddev.height-20,WHITE);
			for (i=0; i<6; i++)
			{
				LCD_DrawRectangle(lcddev.width/2-80+(i*15),lcddev.height/2-80+(i*15),lcddev.width/2-80+(i*15)+60,lcddev.height/2-80+(i*15)+60);
				POINT_COLOR=ColorTab[i];
				delay_ms(500);
			}
			delay_ms(1500);
			for (i=0; i<6; i++)
			{
				LCD_DrawFillRectangle(lcddev.width/2-80+(i*15),lcddev.height/2-80+(i*15),lcddev.width/2-80+(i*15)+60,lcddev.height/2-80+(i*15)+60,ColorTab[i]);
				POINT_COLOR=ColorTab[i];
				delay_ms(500);
			}
			break;
		case VOL_DOWN:
			LCD_Fill(0,20,lcddev.width,lcddev.height-20,WHITE);
			for (i=0; i<6; i++)
			{
				gui_circle(lcddev.width/2-80+(i*25),lcddev.height/2-50+(i*25),ColorTab[i],30,0);
				delay_ms(500);
			}
			delay_ms(1500);
			for (i=0; i<6; i++)
			{
				gui_circle(lcddev.width/2-80+(i*25),lcddev.height/2-50+(i*25),ColorTab[i],30,1);
				delay_ms(500);
			}

			break;
		default:
			break;
		}
	}
}

void usart_test(void)
{
	u8 tmp = 50;
	char temp[30];
	u16 i = 0;
	DrawPage("USART ����");
	LCD_ShowString(30,30,16,"please insert CH340 into PC",1);
	while(1)
	{
		key = KEY_Scan(0);
		delay_ms(20);
		switch(key)
		{
		case PAGE_DOWN:
			printf("�����ڰ�����K1Ŷ \n\r");
			break;
		case NEXT_SONG:
			printf("�����ڰ�����K2Ŷ \n\r");
			break;
		case VOL_UP:
			printf("�����ڰ�����K3Ŷ \n\r");
			break;
		case PAUSE_SONG:
			printf("�����ڰ�����K4Ŷ \n\r");
			break;
		case VOL_DOWN:
			printf("�����ڰ�����K5Ŷ \n\r");
			break;
		case STOP_SONG:
			printf("�����ڰ�����K6Ŷ \n\r");
			break;
		case PAGE_UP:
			printf("�����ڰ�����K7Ŷ \n\r");
			break;
		case LAST_SONG:
			printf("�����ڰ�����K8Ŷ \n\r");
			break;
		case NO_KEY_PRESS:
			break;
		case MUSIC_MODE:
			ProductTest_main();
			break;
		default:
			ProductTest_main();
			break;
		}
		if(strcmp((char*)USART_RX_TEMP,temp))
		{
			if(tmp > 200)
			{
				tmp = 50;
				LCD_Fill(0,50,319,219,BLACK);
			}
			
			LCD_ShowString(30,tmp,16,"receive data: ",1);
			LCD_ShowString(142,tmp,16,USART_RX_TEMP,1);

			strcpy(temp,(char*)USART_RX_TEMP);
			if(i != 0)
				tmp += 16;
			i++;
		}
	}
}


