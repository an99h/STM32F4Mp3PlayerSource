#include "lcd.h"
#include "FONT.H"

void Lcd_Delay(__IO uint32_t nCount)
{
	for(; nCount != 0; nCount--);
}

//����LCD��Ҫ����
//Ĭ��Ϊ����
_lcd_dev lcddev;

//������ɫ,������ɫ
u16 POINT_COLOR = 0xFFE0,BACK_COLOR = 0x0000;
u16 DeviceCode;

//******************************************************************
//��������  LCD_WR_REG
//���ܣ�    ��Һ��������д��д16λָ��
//���������Reg:��д���ָ��ֵ
//����ֵ��  ��
//******************************************************************
void LCD_WR_REG(u8 data)
{
	LCD_RS_CLR;
	LCD_CS_CLR;
	DATAOUT(data);//DATAOUT(data<<8);
	LCD_WR_CLR;
	LCD_WR_SET;
	LCD_CS_SET;
}

//******************************************************************
//��������  LCD_WR_DATA
//���ܣ�    ��Һ��������д��д16λ����
//���������Data:��д�������
//����ֵ��  ��
//******************************************************************
void LCD_WR_DATA(u16 data)
{
	LCD_RS_SET;
	LCD_CS_CLR;
	DATAOUT(data);//DATAOUT(data<<8);
	LCD_WR_CLR;
	LCD_WR_SET;
	LCD_CS_SET;
}
//******************************************************************
//��������  LCD_DrawPoint_16Bit
//���ܣ�    8λ���������д��һ��16λ����
//���������(x,y):�������
//����ֵ��  ��
//******************************************************************
void LCD_DrawPoint_16Bit(u16 color)
{
	LCD_CS_CLR;
	LCD_RD_SET;
	LCD_RS_SET;//д��ַ
	DATAOUT(color>>8);//DATAOUT(color);
	LCD_WR_CLR;
	LCD_WR_SET;
	DATAOUT(color);//DATAOUT(color<<8);
	LCD_WR_CLR;
	LCD_WR_SET;
	LCD_CS_SET;
}

//******************************************************************
//��������  LCD_WriteReg
//���ܣ�    д�Ĵ�������
//���������LCD_Reg:�Ĵ�����ַ
//			LCD_RegValue:Ҫд�������
//����ֵ��  ��
//******************************************************************
void LCD_WriteReg(u8 LCD_Reg, u16 LCD_RegValue)
{
	LCD_WR_REG(LCD_Reg);
	LCD_WR_DATA(LCD_RegValue);
}

//******************************************************************
//��������  LCD_WriteRAM_Prepare
//���ܣ�    ��ʼдGRAM
//			�ڸ�Һ��������RGB����ǰ��Ӧ�÷���дGRAMָ��
//�����������
//����ֵ��  ��
//******************************************************************
void LCD_WriteRAM_Prepare(void)
{
	LCD_WR_REG(lcddev.wramcmd);
}

//******************************************************************
//��������  LCD_DrawPoint
//���ܣ�    ��ָ��λ��д��һ�����ص�����
//���������(x,y):�������
//����ֵ��  ��
//******************************************************************
void LCD_DrawPoint(u16 x,u16 y)
{
	LCD_SetCursor(x,y);	//���ù��λ��
	LCD_CS_CLR;
	LCD_RD_SET;
	LCD_RS_SET;			//д��ַ
	DATAOUT(POINT_COLOR>>8);
	LCD_WR_CLR;
	LCD_WR_SET;
	DATAOUT(POINT_COLOR);
	LCD_WR_CLR;
	LCD_WR_SET;
	LCD_CS_SET;
}

//******************************************************************
//��ILI93xx����������ΪGBR��ʽ��������д���ʱ��ΪRGB��ʽ��
//ͨ���ú���ת��
//c:GBR��ʽ����ɫֵ
//����ֵ��RGB��ʽ����ɫֵ
//******************************************************************
u16 LCD_BGR2RGB(u16 c)
{
	u16  r,g,b,rgb;
	b=(c>>0)&0x1f;
	g=(c>>5)&0x3f;
	r=(c>>11)&0x1f;
	rgb=(b<<11)+(g<<5)+(r<<0);
	return(rgb);
}

//******************************************************************
//��ȡ��ĳ�����ɫֵ
//x:0~239
//y:0~319
//����ֵ:�˵����ɫ
//******************************************************************
u16 LCD_ReadPoint(u16 x,u16 y)
{
	u16 t;
	if(x>=LCD_W||y>=LCD_H)
		return 0;		//�����˷�Χ,ֱ�ӷ���
	LCD_SetCursor(x,y);
	LCD_WR_REG(lcddev.wramcmd);       	//ѡ��GRAM��ַ
	GPIOE->BSRRH=0X8888; 				//PC0-7  ��������
	GPIOE->ODR=0XFFFF;     				//ȫ�������
	LCD_RS_SET;
	LCD_CS_CLR;
	//��ȡ����(��GRAMʱ,��Ҫ��2��)
	LCD_RD_CLR;
	LCD_RD_SET;
	LCD_RD_CLR;
	LCD_RD_SET;
	t=DATAIN;
	LCD_CS_SET;
	GPIOE->BSRRH=0X3333;			 //PB0-7  �������
	GPIOE->ODR=0XFFFF;    			//ȫ�������
	return LCD_BGR2RGB(t);
}

//******************************************************************
//��������  LCD_Clear
//���ܣ�    LCDȫ�������������
//���������Color:Ҫ���������ɫ
//����ֵ��  ��
//******************************************************************
void LCD_Clear(u16 Color)
{
	u32 index=0;
	LCD_SetWindows(0,0,lcddev.width-1,lcddev.height-1);
	LCD_RS_SET;			//д����
	LCD_CS_CLR;
	for(index=0; index<lcddev.width*lcddev.height; index++)
	{
		DATAOUT(Color>>8);
		LCD_WR_CLR;
		LCD_WR_SET;

		DATAOUT(Color);
		LCD_WR_CLR;
		LCD_WR_SET;
	}
	LCD_CS_SET;
}

//******************************************************************
//��������  LCD_GPIOInit
//���ܣ�    GPIO��ʼ��
//�����������
//����ֵ��  ��
//PB0:Ƭѡ�˿�CS				PB1:����/����RS
//PB5:����							PB12��WR
//PB13:RD								PB14:RST
//******************************************************************
void LCD_GPIOInit(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB | RCC_AHB1Periph_GPIOE, ENABLE);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_5 | GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	GPIO_SetBits(GPIOB, GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_5 | GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7;
	//GPIO_InitStructure.GPIO_Pin = GPIO_Pin_All;
	GPIO_Init(GPIOE, &GPIO_InitStructure);
	GPIO_SetBits(GPIOE, GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7);
}

//******************************************************************
//��������  LCD_Reset
//���ܣ�    LCD��λ������Һ����ʼ��ǰҪ���ô˺���
//�����������
//����ֵ��  ��
//******************************************************************
void LCD_RESET(void)
{
	LCD_RST_CLR;
	Lcd_Delay(0xAFFF<<2);
	LCD_RST_SET;
	Lcd_Delay(0xAFFF<<2);
}

//******************************************************************
//��������  LCD_Init
//���ܣ�    LCD��ʼ��
//�����������
//����ֵ��  ��
//******************************************************************
void LCD_Init(void)
{
	LCD_GPIOInit();
	LCD_RESET();

	//************* Start Initial Sequence **********//
	LCD_WR_REG(0xCF);
	LCD_WR_DATA(0x00);
	LCD_WR_DATA(0xC1);
	LCD_WR_DATA(0X30);
	LCD_WR_REG(0xED);
	LCD_WR_DATA(0x64);
	LCD_WR_DATA(0x03);
	LCD_WR_DATA(0X12);
	LCD_WR_DATA(0X81);
	LCD_WR_REG(0xE8);
	LCD_WR_DATA(0x85);
	LCD_WR_DATA(0x10);
	LCD_WR_DATA(0x7A);
	LCD_WR_REG(0xCB);
	LCD_WR_DATA(0x39);
	LCD_WR_DATA(0x2C);
	LCD_WR_DATA(0x00);
	LCD_WR_DATA(0x34);
	LCD_WR_DATA(0x02);
	LCD_WR_REG(0xF7);
	LCD_WR_DATA(0x20);
	LCD_WR_REG(0xEA);
	LCD_WR_DATA(0x00);
	LCD_WR_DATA(0x00);
	LCD_WR_REG(0xC0);    //Power control
	LCD_WR_DATA(0x1B);   //VRH[5:0]
	LCD_WR_REG(0xC1);    //Power control
	LCD_WR_DATA(0x01);   //SAP[2:0];BT[3:0]
	LCD_WR_REG(0xC5);    //VCM control
	LCD_WR_DATA(0x30); 	 //3F
	LCD_WR_DATA(0x30); 	 //3C
	LCD_WR_REG(0xC7);    //VCM control2
	LCD_WR_DATA(0XB7);
	LCD_WR_REG(0x36);    // Memory Access Control
	LCD_WR_DATA(0x48);
	LCD_WR_REG(0x3A);
	LCD_WR_DATA(0x55);
	LCD_WR_REG(0xB1);
	LCD_WR_DATA(0x00);
	LCD_WR_DATA(0x1A);
	LCD_WR_REG(0xB6);    // Display Function Control
	LCD_WR_DATA(0x0A);
	LCD_WR_DATA(0xA2);
	LCD_WR_REG(0xF2);    // 3Gamma Function Disable
	LCD_WR_DATA(0x00);
	LCD_WR_REG(0x26);    //Gamma curve selected
	LCD_WR_DATA(0x01);
	LCD_WR_REG(0xE0);    //Set Gamma
	LCD_WR_DATA(0x0F);
	LCD_WR_DATA(0x2A);
	LCD_WR_DATA(0x28);
	LCD_WR_DATA(0x08);
	LCD_WR_DATA(0x0E);
	LCD_WR_DATA(0x08);
	LCD_WR_DATA(0x54);
	LCD_WR_DATA(0XA9);
	LCD_WR_DATA(0x43);
	LCD_WR_DATA(0x0A);
	LCD_WR_DATA(0x0F);
	LCD_WR_DATA(0x00);
	LCD_WR_DATA(0x00);
	LCD_WR_DATA(0x00);
	LCD_WR_DATA(0x00);
	LCD_WR_REG(0XE1);    //Set Gamma
	LCD_WR_DATA(0x00);
	LCD_WR_DATA(0x15);
	LCD_WR_DATA(0x17);
	LCD_WR_DATA(0x07);
	LCD_WR_DATA(0x11);
	LCD_WR_DATA(0x06);
	LCD_WR_DATA(0x2B);
	LCD_WR_DATA(0x56);
	LCD_WR_DATA(0x3C);
	LCD_WR_DATA(0x05);
	LCD_WR_DATA(0x10);
	LCD_WR_DATA(0x0F);
	LCD_WR_DATA(0x3F);
	LCD_WR_DATA(0x3F);
	LCD_WR_DATA(0x0F);
	LCD_WR_REG(0x2B);
	LCD_WR_DATA(0x00);
	LCD_WR_DATA(0x00);
	LCD_WR_DATA(0x01);
	LCD_WR_DATA(0x3f);
	LCD_WR_REG(0x2A);
	LCD_WR_DATA(0x00);
	LCD_WR_DATA(0x00);
	LCD_WR_DATA(0x00);
	LCD_WR_DATA(0xef);
	LCD_WR_REG(0x11); //Exit Sleep
	Lcd_Delay(0xAFFf<<2);
	LCD_WR_REG(0x29); //display on
	LCD_SetParam();//����LCD����
	LCD_Clear(BLACK);
}

/*************************************************
��������LCD_SetWindows
���ܣ�����lcd��ʾ���ڣ��ڴ�����д�������Զ�����
��ڲ�����xy�����յ�
����ֵ����
*************************************************/
void LCD_SetWindows(u16 xStar, u16 yStar,u16 xEnd,u16 yEnd)
{
	LCD_WR_REG(lcddev.setxcmd);
	LCD_WR_DATA(xStar>>8);
	LCD_WR_DATA(0x00FF&xStar);
	LCD_WR_DATA(xEnd>>8);
	LCD_WR_DATA(0x00FF&xEnd);

	LCD_WR_REG(lcddev.setycmd);
	LCD_WR_DATA(yStar>>8);
	LCD_WR_DATA(0x00FF&yStar);
	LCD_WR_DATA(yEnd>>8);
	LCD_WR_DATA(0x00FF&yEnd);

	LCD_WriteRAM_Prepare();	//��ʼд��GRAM
}

/*************************************************
��������LCD_SetCursor
���ܣ����ù��λ��
��ڲ�����xy����
����ֵ����
*************************************************/
void LCD_SetCursor(u16 Xpos, u16 Ypos)
{
	LCD_WR_REG(lcddev.setxcmd);
	LCD_WR_DATA(Xpos>>8);
	LCD_WR_DATA(0x00FF&Xpos);

	LCD_WR_REG(lcddev.setycmd);
	LCD_WR_DATA(Ypos>>8);
	LCD_WR_DATA(0x00FF&Ypos);

	LCD_WriteRAM_Prepare();	//��ʼд��GRAM
}

//******************************************************************
//����LCD����
//������к�����ģʽ�л�
//******************************************************************
void LCD_SetParam(void)
{
	lcddev.wramcmd=0x2C;
#if USE_HORIZONTAL==1		//ʹ�ú���	  
	lcddev.dir=1;
	lcddev.width=320;
	lcddev.height=240;
	lcddev.setxcmd=0x2A;
	lcddev.setycmd=0x2B;
	LCD_WriteReg(0x36,0x6C);
#else						//����
	lcddev.dir=0;
	lcddev.width=240;
	lcddev.height=320;
	lcddev.setxcmd=0x2A;
	lcddev.setycmd=0x2B;
	LCD_WriteReg(0x36,0xC9);
#endif
}

//******************************************************************
//��������  GUI_DrawPoint
//���ܣ�    GUI���һ����
//���������x:���λ��x����
//        	y:���λ��y����
//			color:Ҫ������ɫ
//����ֵ��  ��
//******************************************************************
void GUI_DrawPoint(u16 x,u16 y,u16 color)
{
	LCD_SetCursor(x,y);//���ù��λ��
	LCD_DrawPoint_16Bit(color);
}

//******************************************************************
//��������  GUI_DrawPoint
//���ܣ�    GUI�������һ����
//���������x:���λ��x����
//        	y:���λ��y����
//			color:Ҫ������ɫ
//����ֵ��  ��
//******************************************************************
void GUI_Fast_DrawPoint(u16 x,u16 y,u16 color)
{
	LCD_WR_REG(lcddev.setxcmd);
	LCD_WR_DATA(x>>8);
	LCD_WR_DATA(x&0XFF);
	LCD_WR_REG(lcddev.setycmd);
	LCD_WR_DATA(y>>8);
	LCD_WR_DATA(y&0XFF);

	LCD_WriteRAM_Prepare();
	LCD_DrawPoint_16Bit(color);
}


//******************************************************************
//��������  LCD_Fill
//���ܣ�    ��ָ�������������ɫ
//���������sx:ָ������ʼ��x����
//        	sy:ָ������ʼ��y����
//			ex:ָ�����������x����
//			ey:ָ�����������y����
//        	color:Ҫ������ɫ
//����ֵ��  ��
//******************************************************************
void LCD_Fill(u16 sx,u16 sy,u16 ex,u16 ey,u16 color)
{
	u16 i,j;
	u16 width=ex-sx+1; 		//�õ����Ŀ��
	u16 height=ey-sy+1;		//�߶�
	LCD_SetWindows(sx,sy,ex-1,ey-1);//������ʾ����

	LCD_RS_SET;//д����
	LCD_CS_CLR;
	for(i=0; i<height; i++)
	{
		for(j=0; j<width; j++)
		{
			DATAOUT(color>>8);//DATAOUT(color);
			LCD_WR_CLR;
			LCD_WR_SET;

			DATAOUT(color);//DATAOUT(color<<8);
			LCD_WR_CLR;
			LCD_WR_SET;
		}
	}
	LCD_CS_SET;
	LCD_SetWindows(0,0,lcddev.width-1,lcddev.height-1);//�ָ���������Ϊȫ��
}

//******************************************************************
//��������  LCD_DrawLine
//���ܣ�    GUI����
//���������x1,y1:�������
//        	x2,y2:�յ�����
//����ֵ��  ��
//******************************************************************
void LCD_DrawLine(u16 x1, u16 y1, u16 x2, u16 y2)
{
	u16 t;
	int xerr=0,yerr=0,delta_x,delta_y,distance;
	int incx,incy,uRow,uCol;

	delta_x=x2-x1; //������������
	delta_y=y2-y1;
	uRow=x1;
	uCol=y1;
	if(delta_x>0)incx=1; //���õ�������
	else if(delta_x==0)incx=0;//��ֱ��
	else
	{
		incx=-1;
		delta_x=-delta_x;
	}
	if(delta_y>0)incy=1;
	else if(delta_y==0)incy=0;//ˮƽ��
	else
	{
		incy=-1;
		delta_y=-delta_y;
	}
	if( delta_x>delta_y)distance=delta_x; //ѡȡ��������������
	else distance=delta_y;
	for(t=0; t<=distance+1; t++ ) //�������
	{
		LCD_DrawPoint(uRow,uCol);//����
		xerr+=delta_x ;
		yerr+=delta_y ;
		if(xerr>distance)
		{
			xerr-=distance;
			uRow+=incx;
		}
		if(yerr>distance)
		{
			yerr-=distance;
			uCol+=incy;
		}
	}
}

//******************************************************************
//��������  LCD_DrawRectangle
//���ܣ�    GUI������(�����)
//���������(x1,y1),(x2,y2):���εĶԽ�����
//����ֵ��  ��
//******************************************************************
void LCD_DrawRectangle(u16 x1, u16 y1, u16 x2, u16 y2)
{
	LCD_DrawLine(x1,y1,x2,y1);
	LCD_DrawLine(x1,y1,x1,y2);
	LCD_DrawLine(x1,y2,x2,y2);
	LCD_DrawLine(x2,y1,x2,y2);
}

//******************************************************************
//��������  LCD_DrawFillRectangle
//���ܣ�    GUI������(���)
//���������(x1,y1),(x2,y2):���εĶԽ�����
//����ֵ��  ��
//******************************************************************
void LCD_DrawFillRectangle(u16 x1, u16 y1, u16 x2, u16 y2,u16 color)
{
	LCD_Fill(x1,y1,x2,y2,color);
}

//******************************************************************
//��������  _draw_circle_8
//���ܣ�    8�Գ��Ի�Բ�㷨(�ڲ�����)
//���������(xc,yc) :Բ��������
// 			(x,y):��������Բ�ĵ�����
//         	c:������ɫ
//����ֵ��  ��
//******************************************************************
void _draw_circle_8(int xc, int yc, int x, int y, u16 c)
{
	GUI_DrawPoint(xc + x, yc + y, c);
	GUI_DrawPoint(xc - x, yc + y, c);
	GUI_DrawPoint(xc + x, yc - y, c);
	GUI_DrawPoint(xc - x, yc - y, c);
	GUI_DrawPoint(xc + y, yc + x, c);
	GUI_DrawPoint(xc - y, yc + x, c);
	GUI_DrawPoint(xc + y, yc - x, c);
	GUI_DrawPoint(xc - y, yc - x, c);
}

//******************************************************************
//��������  gui_circle
//���ܣ�    ��ָ��λ�û�һ��ָ����С��Բ(���)
//���������(xc,yc) :Բ��������
//         	c:������ɫ
//		 	r:Բ�뾶
//		 	fill:����жϱ�־��1-��䣬0-�����
//����ֵ��  ��
//******************************************************************
void gui_circle(int xc, int yc,u16 c,int r, int fill)
{
	int x = 0, y = r, yi, d;
	d = 3 - 2 * r;
	if (fill)
	{
		// �����䣨��ʵ��Բ��
		while (x <= y)
		{
			for (yi = x; yi <= y; yi++)
				_draw_circle_8(xc, yc, x, yi, c);

			if (d < 0)
			{
				d = d + 4 * x + 6;
			}
			else
			{
				d = d + 4 * (x - y) + 10;
				y--;
			}
			x++;
		}
	}
	else
	{
		// �������䣨������Բ��
		while (x <= y)
		{
			_draw_circle_8(xc, yc, x, y, c);
			if (d < 0)
			{
				d = d + 4 * x + 6;
			}
			else
			{
				d = d + 4 * (x - y) + 10;
				y--;
			}
			x++;
		}
	}
}

//******************************************************************
//��������  LCD_ShowChar
//���ܣ�    ��ʾ����Ӣ���ַ�
//���������(x,y):�ַ���ʾλ����ʼ����
//        	fc:ǰ�û�����ɫ
//			bc:������ɫ
//			num:��ֵ��0-94��
//			size:�����С
//			mode:ģʽ  0,���ģʽ;1,����ģʽ
//����ֵ��  ��
//******************************************************************
void LCD_ShowChar(u16 x,u16 y,u16 fc, u16 bc, u8 num,u8 size,u8 mode)
{
	u8 temp;
	u8 pos,t;
	u16 colortemp=POINT_COLOR;

	num=num-' ';//�õ�ƫ�ƺ��ֵ
	LCD_SetWindows(x,y,x+size/2-1,y+size-1);//���õ���������ʾ����
	if(!mode) //�ǵ��ӷ�ʽ
	{

		for(pos=0; pos<size; pos++)
		{
			if(size==12)temp=asc2_1206[num][pos];//����1206����
			else temp=asc2_1608[num][pos];		 //����1608����
			for(t=0; t<size/2; t++)
			{
				if(temp&0x01)LCD_DrawPoint_16Bit(fc);
				else LCD_DrawPoint_16Bit(bc);
				temp>>=1;
			}
		}
	}
	else //���ӷ�ʽ
	{
		for(pos=0; pos<size; pos++)
		{
			if(size==12)temp=asc2_1206[num][pos];//����1206����
			else temp=asc2_1608[num][pos];		 //����1608����
			for(t=0; t<size/2; t++)
			{
				POINT_COLOR=fc;
				if(temp&0x01)LCD_DrawPoint(x+t,y+pos);//��һ����
				temp>>=1;
			}
		}
	}
	POINT_COLOR=colortemp;
	LCD_SetWindows(0,0,lcddev.width-1,lcddev.height-1);//�ָ�����Ϊȫ��
}
//******************************************************************
//��������  LCD_ShowString
//���ܣ�    ��ʾӢ���ַ���
//���������x,y :�������
//			size:�����С
//			*p:�ַ�����ʼ��ַ
//			mode:ģʽ	0,���ģʽ;1,����ģʽ
//����ֵ��  ��
//******************************************************************
void LCD_ShowString(u16 x,u16 y,u8 size,u8 *p,u8 mode)
{
	while((*p<='~')&&(*p>=' '))//�ж��ǲ��ǷǷ��ַ�!
	{
		if(x>(lcddev.width-1)||y>(lcddev.height-1))
			return;
		LCD_ShowChar(x,y,POINT_COLOR,BACK_COLOR,*p,size,mode);
		x+=size/2;
		p++;
	}
}

//******************************************************************
//��������  mypow
//���ܣ�    ��m��n�η�(gui�ڲ�����)
//���������m:����
//	        n:��
//����ֵ��  m��n�η�
//******************************************************************
u32 mypow(u8 m,u8 n)
{
	u32 result=1;
	while(n--)result*=m;
	return result;
}

//******************************************************************
//��������  LCD_ShowNum
//���ܣ�    ��ʾ�������ֱ���ֵ
//���������x,y :�������
//			len :ָ����ʾ���ֵ�λ��
//			size:�����С(12,16)
//			color:��ɫ
//			num:��ֵ(0~4294967295)
//����ֵ��  ��
//******************************************************************
void LCD_ShowNum(u16 x,u16 y,u32 num,u8 len,u8 size)
{
	u8 t,temp;
	u8 enshow=0;

	for(t=0; t<len; t++)
	{
		temp=(num/mypow(10,len-t-1))%10;
		if(enshow==0&&t<(len-1))
		{
			if(temp==0)
			{
				LCD_ShowChar(x+(size/2)*t,y,POINT_COLOR,BACK_COLOR,' ',size,0);
				continue;
			}
			else enshow=1;

		}
		LCD_ShowChar(x+(size/2)*t,y,POINT_COLOR,BACK_COLOR,temp+'0',size,0);
	}
}

//******************************************************************
//��������  DrawTestPage
//���ܣ�    ���Ʋ��Խ���
//���������str :�ַ���ָ��
//����ֵ��  ��
//******************************************************************
void DrawPage(u8 *str)
{
	//���ƹ̶���up
	LCD_Fill(0,0,lcddev.width,20,BLUE);
	//���ƹ̶���down
	LCD_Fill(0,lcddev.height-20,lcddev.width,lcddev.height,BLUE);
	POINT_COLOR=WHITE;
	Gui_StrCenter(0,2,WHITE,BLUE,str,16,1);//������ʾ
	//���Ʋ�������
	LCD_Fill(0,20,lcddev.width,lcddev.height-20,BLACK);
}


#if 1
//******************************************************************
//��������  GUI_DrawFont16
//���ܣ�    ��ʾ����16X16��������
//���������x,y :�������
//			fc:ǰ�û�����ɫ
//			bc:������ɫ
//			s:�ַ�����ַ
//			mode:ģʽ	0,���ģʽ;1,����ģʽ
//����ֵ��  ��
//******************************************************************
void GUI_DrawFont16(u16 x, u16 y, u16 fc, u16 bc, u8 *s,u8 mode)
{
	u8 i,j;
	u16 k;
	u16 HZnum;
	u16 x0=x;
	HZnum=sizeof(tfont16)/sizeof(typFNT_GB16);	//�Զ�ͳ�ƺ�����Ŀ


	for (k=0; k<HZnum; k++)
	{
		if ((tfont16[k].Index[0]==*(s))&&(tfont16[k].Index[1]==*(s+1)))
		{
			LCD_SetWindows(x,y,x+16-1,y+16-1);
			for(i=0; i<16*2; i++)
			{
				for(j=0; j<8; j++)
				{
					if(!mode) //�ǵ��ӷ�ʽ
					{
						if(tfont16[k].Msk[i]&(0x80>>j))	LCD_DrawPoint_16Bit(fc);
						else LCD_DrawPoint_16Bit(bc);
					}
					else
					{
						POINT_COLOR=fc;
						if(tfont16[k].Msk[i]&(0x80>>j))	LCD_DrawPoint(x,y);//��һ����
						x++;
						if((x-x0)==16)
						{
							x=x0;
							y++;
							break;
						}
					}

				}

			}


		}
		continue;  //���ҵ���Ӧ�����ֿ������˳�����ֹ��������ظ�ȡģ����Ӱ��
	}

	LCD_SetWindows(0,0,lcddev.width-1,lcddev.height-1);//�ָ�����Ϊȫ��
}

//******************************************************************
//��������  GUI_DrawFont24
//���ܣ�    ��ʾ����24X24��������
//���������x,y :�������
//			fc:ǰ�û�����ɫ
//			bc:������ɫ
//			s:�ַ�����ַ
//			mode:ģʽ	0,���ģʽ;1,����ģʽ
//����ֵ��  ��
//******************************************************************
void GUI_DrawFont24(u16 x, u16 y, u16 fc, u16 bc, u8 *s,u8 mode)
{
	u8 i,j;
	u16 k;
	u16 HZnum;
	u16 x0=x;
	HZnum=sizeof(tfont24)/sizeof(typFNT_GB24);	//�Զ�ͳ�ƺ�����Ŀ

	for (k=0; k<HZnum; k++)
	{
		if ((tfont24[k].Index[0]==*(s))&&(tfont24[k].Index[1]==*(s+1)))
		{
			LCD_SetWindows(x,y,x+24-1,y+24-1);
			for(i=0; i<24*3; i++)
			{
				for(j=0; j<8; j++)
				{
					if(!mode) //�ǵ��ӷ�ʽ
					{
						if(tfont24[k].Msk[i]&(0x80>>j))	LCD_DrawPoint_16Bit(fc);
						else LCD_DrawPoint_16Bit(bc);
					}
					else
					{
						POINT_COLOR=fc;
						if(tfont24[k].Msk[i]&(0x80>>j))	LCD_DrawPoint(x,y);//��һ����
						x++;
						if((x-x0)==24)
						{
							x=x0;
							y++;
							break;
						}
					}
				}
			}


		}
		continue;  //���ҵ���Ӧ�����ֿ������˳�����ֹ��������ظ�ȡģ����Ӱ��
	}

	LCD_SetWindows(0,0,lcddev.width-1,lcddev.height-1);//�ָ�����Ϊȫ��
}

//******************************************************************
//��������  GUI_DrawFont32
//���ܣ�    ��ʾ����32X32��������
//���������x,y :�������
//			fc:ǰ�û�����ɫ
//			bc:������ɫ
//			s:�ַ�����ַ
//			mode:ģʽ	0,���ģʽ;1,����ģʽ
//����ֵ��  ��
//******************************************************************
void GUI_DrawFont32(u16 x, u16 y, u16 fc, u16 bc, u8 *s,u8 mode)
{
	u8 i,j;
	u16 k;
	u16 HZnum;
	u16 x0=x;
	HZnum=sizeof(tfont32)/sizeof(typFNT_GB32);	//�Զ�ͳ�ƺ�����Ŀ
	for (k=0; k<HZnum; k++)
	{
		if ((tfont32[k].Index[0]==*(s))&&(tfont32[k].Index[1]==*(s+1)))
		{
			LCD_SetWindows(x,y,x+32-1,y+32-1);
			for(i=0; i<32*4; i++)
			{
				for(j=0; j<8; j++)
				{
					if(!mode) //�ǵ��ӷ�ʽ
					{
						if(tfont32[k].Msk[i]&(0x80>>j))	LCD_DrawPoint_16Bit(fc);
						else LCD_DrawPoint_16Bit(bc);
					}
					else
					{
						POINT_COLOR=fc;
						if(tfont32[k].Msk[i]&(0x80>>j))	LCD_DrawPoint(x,y);//��һ����
						x++;
						if((x-x0)==32)
						{
							x=x0;
							y++;
							break;
						}
					}
				}
			}


		}
		continue;  //���ҵ���Ӧ�����ֿ������˳�����ֹ��������ظ�ȡģ����Ӱ��
	}

	LCD_SetWindows(0,0,lcddev.width-1,lcddev.height-1);//�ָ�����Ϊȫ��
}

#endif
#if 0
/*--------------������������ ʵ���ֿ�ĵ��ü���ʾ-------------------*/
FATFS fss;
FIL fsrc;      /* file objects  */
UINT brr;         // File R/W count
static uint8_t Find_Chinese(uint8_t* str, uint8_t* p)
{
	uint8_t High8bit,Low8bit;
	FRESULT res;
//	uint8_t error[]="Please put the sys folder which include the font file HZK16.bin into the SD  card  root directory. ";
	High8bit=*str;		 /*  ��8λ����  */
	Low8bit=*(str+1);		 /*  ��8λ����	*/

	f_mount(&fss,"0:/sys",1);

	/* Infinite loop */
	res = f_open(&fsrc, "/sys/FONT16.FON", FA_OPEN_EXISTING | FA_READ);	 /* ��SD��������HZK16.bin*/
	if(res != 0)
	{
		//LCD_ShowString(32,100,error,RED ,BLACK );
		return 1;
	}

	f_lseek(&fsrc,32*((High8bit-0xa0-1)*94+(Low8bit-0xa0-1)));
	f_read(&fsrc, p, 32, &brr);

	f_close(&fsrc);
	//f_mount(0, NULL);
	return 0;
}
/*********************************************************************************
* ��    �ƣ�void PutChinese(u16 x, u16 y, u8 *c, u32 PenColor, u32 BackColor)
* ��ڲ�����u16 x,u16 y �ַ�����λ��
						u8  *c      ���ֵ�ָ��
						u32 PenColor������ɫ
						u32 BackColor������ɫ
* ���ڲ�������
* ��    �ܣ���SPI_FLASH����Һ��ֻ���ַ��
						����32�ֽڵĵ��������͵�LCD��ʾ
* ˵    ����ֻ�ܱ�GUI_Put����
*********************************************************************************/
void PutChinese(u16 x, u16 y, u8 *c)
{
	u16 i=0;
	u16 j=0;

	u8 buffer[32];
	u16 tmp_char=0;

	if(!Find_Chinese(c,buffer))
	{
		LCD_SetWindows(x,y,x+16-1,y+16-1);	 	/*���ô���*/
		LCD_SetCursor(x,y);                       /*���ù��λ�� */
		LCD_WriteRAM_Prepare();

		/* 16*16���ֵ��󣬹���16�� ÿ��16���㣬�洢��ʱ�������ֽ�Ϊ��λ
			 ��ϲ�����*/
		for (i=0; i<16; i++)
		{
			tmp_char=buffer[i*2];
			tmp_char=(tmp_char<<8);
			tmp_char|=buffer[2*i+1];  /*����tmp_char�洢��һ�еĵ�������*/

			for (j=0; j<16; j++)
			{
				if((tmp_char<<j) & 0x8000)
					LCD_DrawPoint_16Bit(WHITE);
				else
					LCD_DrawPoint_16Bit(BLACK);
			}
		}
		LCD_SetWindows(0,0,lcddev.width-1,lcddev.height-1);  	/*�ָ������С*/
	}
}
#endif

//******************************************************************
//��������  Show_Str
//���ܣ�    ��ʾһ���ַ���,������Ӣ����ʾ
//���������x,y :�������
// 			fc:ǰ�û�����ɫ
//			bc:������ɫ
//			str :�ַ���
//			size:�����С
//			mode:ģʽ	0,���ģʽ;1,����ģʽ
//����ֵ��  ��
//******************************************************************
#if 1
void Show_Str(u16 x, u16 y, u16 fc, u16 bc, u8 *str,u8 size,u8 mode)
{
	u16 x0=x;
	u8 bHz=0;     //�ַ���������
	while(*str!=0)//����δ����
	{
		if(!bHz)
		{
			if(x>(lcddev.width-size/2)||y>(lcddev.height-size))
				return;
			if(*str>0x80)
				bHz=1;//����
			else              //�ַ�
			{
				if(*str==0x0D)//���з���
				{
					y+=size;
					x=x0;
					str++;
				}
				else
				{
					if(size>16)//�ֿ���û�м���12X24 16X32��Ӣ������,��8X16����
					{
						LCD_ShowChar(x,y,fc,bc,*str,16,mode);
						x+=8; //�ַ�,Ϊȫ�ֵ�һ��
					}
					else
					{
						LCD_ShowChar(x,y,fc,bc,*str,size,mode);
						x+=size/2; //�ַ�,Ϊȫ�ֵ�һ��
					}
				}
				str++;

			}
		}
		else //����
        {   
			if(x>(lcddev.width-size)||y>(lcddev.height-size)) 
			return;  
            bHz=0;//�к��ֿ�    
			if(size==32)
			GUI_DrawFont32(x,y,fc,bc,str,mode);	 	
			else if(size==24)
			GUI_DrawFont24(x,y,fc,bc,str,mode);	
			else
			GUI_DrawFont16(x,y,fc,bc,str,mode);
				
	        str+=2; 
	        x+=size;//��һ������ƫ��	    
		}
	}
}

#endif
//******************************************************************
//��������  Gui_StrCenter
//���ܣ�    ������ʾһ���ַ���,������Ӣ����ʾ
//���������x,y :�������
// 			fc:ǰ�û�����ɫ
//			bc:������ɫ
//			str :�ַ���
//			size:�����С
//			mode:ģʽ	0,���ģʽ;1,����ģʽ
//����ֵ��  ��
//******************************************************************
#if 1
void Gui_StrCenter(u16 x, u16 y, u16 fc, u16 bc, u8 *str,u8 size,u8 mode)
{
	u16 len=strlen((const char *)str);
	u16 x1=(lcddev.width-len*8)/2;
	Show_Str(x+x1,y,fc,bc,str,size,mode);
}
#endif
//******************************************************************
//��������  Gui_Drawbmp16
//���ܣ�    ��ʾһ��16λBMPͼ��
//���������x,y :�������
// 			*p :ͼ��������ʼ��ַ
//����ֵ��  ��
//******************************************************************
void Gui_icon(u16 x,u16 y,const unsigned char *p) //��ʾ24*24ͼƬ
{
	int i;
	unsigned char picH,picL;
	LCD_SetWindows(x,y,x+24-1,y+24-1);//��������
	for(i=0; i<24*24; i++)
	{
		picL=*(p+i*2);	//���ݵ�λ��ǰ
		picH=*(p+i*2+1);
		LCD_DrawPoint_16Bit(picH<<8|picL);
	}
	LCD_SetWindows(0,0,lcddev.width-1,lcddev.height-1);//�ָ���ʾ����Ϊȫ��

}

