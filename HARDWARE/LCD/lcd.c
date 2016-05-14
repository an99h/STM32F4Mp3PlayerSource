#include "lcd.h"
#include "FONT.H"

void Lcd_Delay(__IO uint32_t nCount)
{
	for(; nCount != 0; nCount--);
}

//管理LCD重要参数
//默认为竖屏
_lcd_dev lcddev;

//画笔颜色,背景颜色
u16 POINT_COLOR = 0xFFE0,BACK_COLOR = 0x0000;
u16 DeviceCode;

//******************************************************************
//函数名：  LCD_WR_REG
//功能：    向液晶屏总线写入写16位指令
//输入参数：Reg:待写入的指令值
//返回值：  无
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
//函数名：  LCD_WR_DATA
//功能：    向液晶屏总线写入写16位数据
//输入参数：Data:待写入的数据
//返回值：  无
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
//函数名：  LCD_DrawPoint_16Bit
//功能：    8位总线下如何写入一个16位数据
//输入参数：(x,y):光标坐标
//返回值：  无
//******************************************************************
void LCD_DrawPoint_16Bit(u16 color)
{
	LCD_CS_CLR;
	LCD_RD_SET;
	LCD_RS_SET;//写地址
	DATAOUT(color>>8);//DATAOUT(color);
	LCD_WR_CLR;
	LCD_WR_SET;
	DATAOUT(color);//DATAOUT(color<<8);
	LCD_WR_CLR;
	LCD_WR_SET;
	LCD_CS_SET;
}

//******************************************************************
//函数名：  LCD_WriteReg
//功能：    写寄存器数据
//输入参数：LCD_Reg:寄存器地址
//			LCD_RegValue:要写入的数据
//返回值：  无
//******************************************************************
void LCD_WriteReg(u8 LCD_Reg, u16 LCD_RegValue)
{
	LCD_WR_REG(LCD_Reg);
	LCD_WR_DATA(LCD_RegValue);
}

//******************************************************************
//函数名：  LCD_WriteRAM_Prepare
//功能：    开始写GRAM
//			在给液晶屏传送RGB数据前，应该发送写GRAM指令
//输入参数：无
//返回值：  无
//******************************************************************
void LCD_WriteRAM_Prepare(void)
{
	LCD_WR_REG(lcddev.wramcmd);
}

//******************************************************************
//函数名：  LCD_DrawPoint
//功能：    在指定位置写入一个像素点数据
//输入参数：(x,y):光标坐标
//返回值：  无
//******************************************************************
void LCD_DrawPoint(u16 x,u16 y)
{
	LCD_SetCursor(x,y);	//设置光标位置
	LCD_CS_CLR;
	LCD_RD_SET;
	LCD_RS_SET;			//写地址
	DATAOUT(POINT_COLOR>>8);
	LCD_WR_CLR;
	LCD_WR_SET;
	DATAOUT(POINT_COLOR);
	LCD_WR_CLR;
	LCD_WR_SET;
	LCD_CS_SET;
}

//******************************************************************
//从ILI93xx读出的数据为GBR格式，而我们写入的时候为RGB格式。
//通过该函数转换
//c:GBR格式的颜色值
//返回值：RGB格式的颜色值
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
//读取个某点的颜色值
//x:0~239
//y:0~319
//返回值:此点的颜色
//******************************************************************
u16 LCD_ReadPoint(u16 x,u16 y)
{
	u16 t;
	if(x>=LCD_W||y>=LCD_H)
		return 0;		//超过了范围,直接返回
	LCD_SetCursor(x,y);
	LCD_WR_REG(lcddev.wramcmd);       	//选择GRAM地址
	GPIOE->BSRRH=0X8888; 				//PC0-7  上拉输入
	GPIOE->ODR=0XFFFF;     				//全部输出高
	LCD_RS_SET;
	LCD_CS_CLR;
	//读取数据(读GRAM时,需要读2次)
	LCD_RD_CLR;
	LCD_RD_SET;
	LCD_RD_CLR;
	LCD_RD_SET;
	t=DATAIN;
	LCD_CS_SET;
	GPIOE->BSRRH=0X3333;			 //PB0-7  上拉输出
	GPIOE->ODR=0XFFFF;    			//全部输出高
	return LCD_BGR2RGB(t);
}

//******************************************************************
//函数名：  LCD_Clear
//功能：    LCD全屏填充清屏函数
//输入参数：Color:要清屏的填充色
//返回值：  无
//******************************************************************
void LCD_Clear(u16 Color)
{
	u32 index=0;
	LCD_SetWindows(0,0,lcddev.width-1,lcddev.height-1);
	LCD_RS_SET;			//写数据
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
//函数名：  LCD_GPIOInit
//功能：    GPIO初始化
//输入参数：无
//返回值：  无
//PB0:片选端口CS				PB1:数据/命令RS
//PB5:背光							PB12：WR
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
//函数名：  LCD_Reset
//功能：    LCD复位函数，液晶初始化前要调用此函数
//输入参数：无
//返回值：  无
//******************************************************************
void LCD_RESET(void)
{
	LCD_RST_CLR;
	Lcd_Delay(0xAFFF<<2);
	LCD_RST_SET;
	Lcd_Delay(0xAFFF<<2);
}

//******************************************************************
//函数名：  LCD_Init
//功能：    LCD初始化
//输入参数：无
//返回值：  无
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
	LCD_SetParam();//设置LCD参数
	LCD_Clear(BLACK);
}

/*************************************************
函数名：LCD_SetWindows
功能：设置lcd显示窗口，在此区域写点数据自动换行
入口参数：xy起点和终点
返回值：无
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

	LCD_WriteRAM_Prepare();	//开始写入GRAM
}

/*************************************************
函数名：LCD_SetCursor
功能：设置光标位置
入口参数：xy坐标
返回值：无
*************************************************/
void LCD_SetCursor(u16 Xpos, u16 Ypos)
{
	LCD_WR_REG(lcddev.setxcmd);
	LCD_WR_DATA(Xpos>>8);
	LCD_WR_DATA(0x00FF&Xpos);

	LCD_WR_REG(lcddev.setycmd);
	LCD_WR_DATA(Ypos>>8);
	LCD_WR_DATA(0x00FF&Ypos);

	LCD_WriteRAM_Prepare();	//开始写入GRAM
}

//******************************************************************
//设置LCD参数
//方便进行横竖屏模式切换
//******************************************************************
void LCD_SetParam(void)
{
	lcddev.wramcmd=0x2C;
#if USE_HORIZONTAL==1		//使用横屏	  
	lcddev.dir=1;
	lcddev.width=320;
	lcddev.height=240;
	lcddev.setxcmd=0x2A;
	lcddev.setycmd=0x2B;
	LCD_WriteReg(0x36,0x6C);
#else						//竖屏
	lcddev.dir=0;
	lcddev.width=240;
	lcddev.height=320;
	lcddev.setxcmd=0x2A;
	lcddev.setycmd=0x2B;
	LCD_WriteReg(0x36,0xC9);
#endif
}

//******************************************************************
//函数名：  GUI_DrawPoint
//功能：    GUI描绘一个点
//输入参数：x:光标位置x坐标
//        	y:光标位置y坐标
//			color:要填充的颜色
//返回值：  无
//******************************************************************
void GUI_DrawPoint(u16 x,u16 y,u16 color)
{
	LCD_SetCursor(x,y);//设置光标位置
	LCD_DrawPoint_16Bit(color);
}

//******************************************************************
//函数名：  GUI_DrawPoint
//功能：    GUI快速描绘一个点
//输入参数：x:光标位置x坐标
//        	y:光标位置y坐标
//			color:要填充的颜色
//返回值：  无
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
//函数名：  LCD_Fill
//功能：    在指定区域内填充颜色
//输入参数：sx:指定区域开始点x坐标
//        	sy:指定区域开始点y坐标
//			ex:指定区域结束点x坐标
//			ey:指定区域结束点y坐标
//        	color:要填充的颜色
//返回值：  无
//******************************************************************
void LCD_Fill(u16 sx,u16 sy,u16 ex,u16 ey,u16 color)
{
	u16 i,j;
	u16 width=ex-sx+1; 		//得到填充的宽度
	u16 height=ey-sy+1;		//高度
	LCD_SetWindows(sx,sy,ex-1,ey-1);//设置显示窗口

	LCD_RS_SET;//写数据
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
	LCD_SetWindows(0,0,lcddev.width-1,lcddev.height-1);//恢复窗口设置为全屏
}

//******************************************************************
//函数名：  LCD_DrawLine
//功能：    GUI画线
//输入参数：x1,y1:起点坐标
//        	x2,y2:终点坐标
//返回值：  无
//******************************************************************
void LCD_DrawLine(u16 x1, u16 y1, u16 x2, u16 y2)
{
	u16 t;
	int xerr=0,yerr=0,delta_x,delta_y,distance;
	int incx,incy,uRow,uCol;

	delta_x=x2-x1; //计算坐标增量
	delta_y=y2-y1;
	uRow=x1;
	uCol=y1;
	if(delta_x>0)incx=1; //设置单步方向
	else if(delta_x==0)incx=0;//垂直线
	else
	{
		incx=-1;
		delta_x=-delta_x;
	}
	if(delta_y>0)incy=1;
	else if(delta_y==0)incy=0;//水平线
	else
	{
		incy=-1;
		delta_y=-delta_y;
	}
	if( delta_x>delta_y)distance=delta_x; //选取基本增量坐标轴
	else distance=delta_y;
	for(t=0; t<=distance+1; t++ ) //画线输出
	{
		LCD_DrawPoint(uRow,uCol);//画点
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
//函数名：  LCD_DrawRectangle
//功能：    GUI画矩形(非填充)
//输入参数：(x1,y1),(x2,y2):矩形的对角坐标
//返回值：  无
//******************************************************************
void LCD_DrawRectangle(u16 x1, u16 y1, u16 x2, u16 y2)
{
	LCD_DrawLine(x1,y1,x2,y1);
	LCD_DrawLine(x1,y1,x1,y2);
	LCD_DrawLine(x1,y2,x2,y2);
	LCD_DrawLine(x2,y1,x2,y2);
}

//******************************************************************
//函数名：  LCD_DrawFillRectangle
//功能：    GUI画矩形(填充)
//输入参数：(x1,y1),(x2,y2):矩形的对角坐标
//返回值：  无
//******************************************************************
void LCD_DrawFillRectangle(u16 x1, u16 y1, u16 x2, u16 y2,u16 color)
{
	LCD_Fill(x1,y1,x2,y2,color);
}

//******************************************************************
//函数名：  _draw_circle_8
//功能：    8对称性画圆算法(内部调用)
//输入参数：(xc,yc) :圆中心坐标
// 			(x,y):光标相对于圆心的坐标
//         	c:填充的颜色
//返回值：  无
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
//函数名：  gui_circle
//功能：    在指定位置画一个指定大小的圆(填充)
//输入参数：(xc,yc) :圆中心坐标
//         	c:填充的颜色
//		 	r:圆半径
//		 	fill:填充判断标志，1-填充，0-不填充
//返回值：  无
//******************************************************************
void gui_circle(int xc, int yc,u16 c,int r, int fill)
{
	int x = 0, y = r, yi, d;
	d = 3 - 2 * r;
	if (fill)
	{
		// 如果填充（画实心圆）
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
		// 如果不填充（画空心圆）
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
//函数名：  LCD_ShowChar
//功能：    显示单个英文字符
//输入参数：(x,y):字符显示位置起始坐标
//        	fc:前置画笔颜色
//			bc:背景颜色
//			num:数值（0-94）
//			size:字体大小
//			mode:模式  0,填充模式;1,叠加模式
//返回值：  无
//******************************************************************
void LCD_ShowChar(u16 x,u16 y,u16 fc, u16 bc, u8 num,u8 size,u8 mode)
{
	u8 temp;
	u8 pos,t;
	u16 colortemp=POINT_COLOR;

	num=num-' ';//得到偏移后的值
	LCD_SetWindows(x,y,x+size/2-1,y+size-1);//设置单个文字显示窗口
	if(!mode) //非叠加方式
	{

		for(pos=0; pos<size; pos++)
		{
			if(size==12)temp=asc2_1206[num][pos];//调用1206字体
			else temp=asc2_1608[num][pos];		 //调用1608字体
			for(t=0; t<size/2; t++)
			{
				if(temp&0x01)LCD_DrawPoint_16Bit(fc);
				else LCD_DrawPoint_16Bit(bc);
				temp>>=1;
			}
		}
	}
	else //叠加方式
	{
		for(pos=0; pos<size; pos++)
		{
			if(size==12)temp=asc2_1206[num][pos];//调用1206字体
			else temp=asc2_1608[num][pos];		 //调用1608字体
			for(t=0; t<size/2; t++)
			{
				POINT_COLOR=fc;
				if(temp&0x01)LCD_DrawPoint(x+t,y+pos);//画一个点
				temp>>=1;
			}
		}
	}
	POINT_COLOR=colortemp;
	LCD_SetWindows(0,0,lcddev.width-1,lcddev.height-1);//恢复窗口为全屏
}
//******************************************************************
//函数名：  LCD_ShowString
//功能：    显示英文字符串
//输入参数：x,y :起点坐标
//			size:字体大小
//			*p:字符串起始地址
//			mode:模式	0,填充模式;1,叠加模式
//返回值：  无
//******************************************************************
void LCD_ShowString(u16 x,u16 y,u8 size,u8 *p,u8 mode)
{
	while((*p<='~')&&(*p>=' '))//判断是不是非法字符!
	{
		if(x>(lcddev.width-1)||y>(lcddev.height-1))
			return;
		LCD_ShowChar(x,y,POINT_COLOR,BACK_COLOR,*p,size,mode);
		x+=size/2;
		p++;
	}
}

//******************************************************************
//函数名：  mypow
//功能：    求m的n次方(gui内部调用)
//输入参数：m:乘数
//	        n:幂
//返回值：  m的n次方
//******************************************************************
u32 mypow(u8 m,u8 n)
{
	u32 result=1;
	while(n--)result*=m;
	return result;
}

//******************************************************************
//函数名：  LCD_ShowNum
//功能：    显示单个数字变量值
//输入参数：x,y :起点坐标
//			len :指定显示数字的位数
//			size:字体大小(12,16)
//			color:颜色
//			num:数值(0~4294967295)
//返回值：  无
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
//函数名：  DrawTestPage
//功能：    绘制测试界面
//输入参数：str :字符串指针
//返回值：  无
//******************************************************************
void DrawPage(u8 *str)
{
	//绘制固定栏up
	LCD_Fill(0,0,lcddev.width,20,BLUE);
	//绘制固定栏down
	LCD_Fill(0,lcddev.height-20,lcddev.width,lcddev.height,BLUE);
	POINT_COLOR=WHITE;
	Gui_StrCenter(0,2,WHITE,BLUE,str,16,1);//居中显示
	//绘制测试区域
	LCD_Fill(0,20,lcddev.width,lcddev.height-20,BLACK);
}


#if 1
//******************************************************************
//函数名：  GUI_DrawFont16
//功能：    显示单个16X16中文字体
//输入参数：x,y :起点坐标
//			fc:前置画笔颜色
//			bc:背景颜色
//			s:字符串地址
//			mode:模式	0,填充模式;1,叠加模式
//返回值：  无
//******************************************************************
void GUI_DrawFont16(u16 x, u16 y, u16 fc, u16 bc, u8 *s,u8 mode)
{
	u8 i,j;
	u16 k;
	u16 HZnum;
	u16 x0=x;
	HZnum=sizeof(tfont16)/sizeof(typFNT_GB16);	//自动统计汉字数目


	for (k=0; k<HZnum; k++)
	{
		if ((tfont16[k].Index[0]==*(s))&&(tfont16[k].Index[1]==*(s+1)))
		{
			LCD_SetWindows(x,y,x+16-1,y+16-1);
			for(i=0; i<16*2; i++)
			{
				for(j=0; j<8; j++)
				{
					if(!mode) //非叠加方式
					{
						if(tfont16[k].Msk[i]&(0x80>>j))	LCD_DrawPoint_16Bit(fc);
						else LCD_DrawPoint_16Bit(bc);
					}
					else
					{
						POINT_COLOR=fc;
						if(tfont16[k].Msk[i]&(0x80>>j))	LCD_DrawPoint(x,y);//画一个点
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
		continue;  //查找到对应点阵字库立即退出，防止多个汉字重复取模带来影响
	}

	LCD_SetWindows(0,0,lcddev.width-1,lcddev.height-1);//恢复窗口为全屏
}

//******************************************************************
//函数名：  GUI_DrawFont24
//功能：    显示单个24X24中文字体
//输入参数：x,y :起点坐标
//			fc:前置画笔颜色
//			bc:背景颜色
//			s:字符串地址
//			mode:模式	0,填充模式;1,叠加模式
//返回值：  无
//******************************************************************
void GUI_DrawFont24(u16 x, u16 y, u16 fc, u16 bc, u8 *s,u8 mode)
{
	u8 i,j;
	u16 k;
	u16 HZnum;
	u16 x0=x;
	HZnum=sizeof(tfont24)/sizeof(typFNT_GB24);	//自动统计汉字数目

	for (k=0; k<HZnum; k++)
	{
		if ((tfont24[k].Index[0]==*(s))&&(tfont24[k].Index[1]==*(s+1)))
		{
			LCD_SetWindows(x,y,x+24-1,y+24-1);
			for(i=0; i<24*3; i++)
			{
				for(j=0; j<8; j++)
				{
					if(!mode) //非叠加方式
					{
						if(tfont24[k].Msk[i]&(0x80>>j))	LCD_DrawPoint_16Bit(fc);
						else LCD_DrawPoint_16Bit(bc);
					}
					else
					{
						POINT_COLOR=fc;
						if(tfont24[k].Msk[i]&(0x80>>j))	LCD_DrawPoint(x,y);//画一个点
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
		continue;  //查找到对应点阵字库立即退出，防止多个汉字重复取模带来影响
	}

	LCD_SetWindows(0,0,lcddev.width-1,lcddev.height-1);//恢复窗口为全屏
}

//******************************************************************
//函数名：  GUI_DrawFont32
//功能：    显示单个32X32中文字体
//输入参数：x,y :起点坐标
//			fc:前置画笔颜色
//			bc:背景颜色
//			s:字符串地址
//			mode:模式	0,填充模式;1,叠加模式
//返回值：  无
//******************************************************************
void GUI_DrawFont32(u16 x, u16 y, u16 fc, u16 bc, u8 *s,u8 mode)
{
	u8 i,j;
	u16 k;
	u16 HZnum;
	u16 x0=x;
	HZnum=sizeof(tfont32)/sizeof(typFNT_GB32);	//自动统计汉字数目
	for (k=0; k<HZnum; k++)
	{
		if ((tfont32[k].Index[0]==*(s))&&(tfont32[k].Index[1]==*(s+1)))
		{
			LCD_SetWindows(x,y,x+32-1,y+32-1);
			for(i=0; i<32*4; i++)
			{
				for(j=0; j<8; j++)
				{
					if(!mode) //非叠加方式
					{
						if(tfont32[k].Msk[i]&(0x80>>j))	LCD_DrawPoint_16Bit(fc);
						else LCD_DrawPoint_16Bit(bc);
					}
					else
					{
						POINT_COLOR=fc;
						if(tfont32[k].Msk[i]&(0x80>>j))	LCD_DrawPoint(x,y);//画一个点
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
		continue;  //查找到对应点阵字库立即退出，防止多个汉字重复取模带来影响
	}

	LCD_SetWindows(0,0,lcddev.width-1,lcddev.height-1);//恢复窗口为全屏
}

#endif
#if 0
/*--------------下面三个函数 实现字库的调用及显示-------------------*/
FATFS fss;
FIL fsrc;      /* file objects  */
UINT brr;         // File R/W count
static uint8_t Find_Chinese(uint8_t* str, uint8_t* p)
{
	uint8_t High8bit,Low8bit;
	FRESULT res;
//	uint8_t error[]="Please put the sys folder which include the font file HZK16.bin into the SD  card  root directory. ";
	High8bit=*str;		 /*  高8位数据  */
	Low8bit=*(str+1);		 /*  低8位数据	*/

	f_mount(&fss,"0:/sys",1);

	/* Infinite loop */
	res = f_open(&fsrc, "/sys/FONT16.FON", FA_OPEN_EXISTING | FA_READ);	 /* 在SD卡里搜索HZK16.bin*/
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
* 名    称：void PutChinese(u16 x, u16 y, u8 *c, u32 PenColor, u32 BackColor)
* 入口参数：u16 x,u16 y 字符坐标位置
						u8  *c      汉字的指针
						u32 PenColor画笔颜色
						u32 BackColor背景颜色
* 出口参数：无
* 功    能：从SPI_FLASH里查找汉字基地址，
						并将32字节的点阵数据送到LCD显示
* 说    明：只能被GUI_Put调用
*********************************************************************************/
void PutChinese(u16 x, u16 y, u8 *c)
{
	u16 i=0;
	u16 j=0;

	u8 buffer[32];
	u16 tmp_char=0;

	if(!Find_Chinese(c,buffer))
	{
		LCD_SetWindows(x,y,x+16-1,y+16-1);	 	/*设置窗口*/
		LCD_SetCursor(x,y);                       /*设置光标位置 */
		LCD_WriteRAM_Prepare();

		/* 16*16汉字点阵，共有16行 每行16个点，存储的时候是以字节为单位
			 需合并成字*/
		for (i=0; i<16; i++)
		{
			tmp_char=buffer[i*2];
			tmp_char=(tmp_char<<8);
			tmp_char|=buffer[2*i+1];  /*现在tmp_char存储着一行的点阵数据*/

			for (j=0; j<16; j++)
			{
				if((tmp_char<<j) & 0x8000)
					LCD_DrawPoint_16Bit(WHITE);
				else
					LCD_DrawPoint_16Bit(BLACK);
			}
		}
		LCD_SetWindows(0,0,lcddev.width-1,lcddev.height-1);  	/*恢复窗体大小*/
	}
}
#endif

//******************************************************************
//函数名：  Show_Str
//功能：    显示一个字符串,包含中英文显示
//输入参数：x,y :起点坐标
// 			fc:前置画笔颜色
//			bc:背景颜色
//			str :字符串
//			size:字体大小
//			mode:模式	0,填充模式;1,叠加模式
//返回值：  无
//******************************************************************
#if 1
void Show_Str(u16 x, u16 y, u16 fc, u16 bc, u8 *str,u8 size,u8 mode)
{
	u16 x0=x;
	u8 bHz=0;     //字符或者中文
	while(*str!=0)//数据未结束
	{
		if(!bHz)
		{
			if(x>(lcddev.width-size/2)||y>(lcddev.height-size))
				return;
			if(*str>0x80)
				bHz=1;//中文
			else              //字符
			{
				if(*str==0x0D)//换行符号
				{
					y+=size;
					x=x0;
					str++;
				}
				else
				{
					if(size>16)//字库中没有集成12X24 16X32的英文字体,用8X16代替
					{
						LCD_ShowChar(x,y,fc,bc,*str,16,mode);
						x+=8; //字符,为全字的一半
					}
					else
					{
						LCD_ShowChar(x,y,fc,bc,*str,size,mode);
						x+=size/2; //字符,为全字的一半
					}
				}
				str++;

			}
		}
		else //中文
        {   
			if(x>(lcddev.width-size)||y>(lcddev.height-size)) 
			return;  
            bHz=0;//有汉字库    
			if(size==32)
			GUI_DrawFont32(x,y,fc,bc,str,mode);	 	
			else if(size==24)
			GUI_DrawFont24(x,y,fc,bc,str,mode);	
			else
			GUI_DrawFont16(x,y,fc,bc,str,mode);
				
	        str+=2; 
	        x+=size;//下一个汉字偏移	    
		}
	}
}

#endif
//******************************************************************
//函数名：  Gui_StrCenter
//功能：    居中显示一个字符串,包含中英文显示
//输入参数：x,y :起点坐标
// 			fc:前置画笔颜色
//			bc:背景颜色
//			str :字符串
//			size:字体大小
//			mode:模式	0,填充模式;1,叠加模式
//返回值：  无
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
//函数名：  Gui_Drawbmp16
//功能：    显示一副16位BMP图像
//输入参数：x,y :起点坐标
// 			*p :图像数组起始地址
//返回值：  无
//******************************************************************
void Gui_icon(u16 x,u16 y,const unsigned char *p) //显示24*24图片
{
	int i;
	unsigned char picH,picL;
	LCD_SetWindows(x,y,x+24-1,y+24-1);//窗口设置
	for(i=0; i<24*24; i++)
	{
		picL=*(p+i*2);	//数据低位在前
		picH=*(p+i*2+1);
		LCD_DrawPoint_16Bit(picH<<8|picL);
	}
	LCD_SetWindows(0,0,lcddev.width-1,lcddev.height-1);//恢复显示窗口为全屏

}

