#ifndef __ILI9341_LCD_H
#define	__ILI9341_LCD_H

#include "stm32f4xx.h"
#include "stdlib.h"
#include "string.h"

//////////////////////////////////////////////////////////////////////////////////	
//																				//
//					ILI9341 8位数据线		2.4寸LCD屏幕引脚配置					//
//																				//
//					数据线：	PE0--------->LCD_D0									//
//						 	PE1--------->LCD_D1									//
//							PE2--------->LCD_D2									//
//							PE3--------->LCD_D3									//
//							PE4--------->LCD_D4									//
//							PE5--------->LCD_D5									//
//							PE6--------->LCD_D6									//
//							PE7--------->LCD_D7									//
//																				//
//					控制线：	PB0 --------->LCD_CS								//
//							PB1 --------->LCD_RS								//
//							PB12--------->LCD_WR								//
//							PB13--------->LCD_WD								//
//							PB14--------->LCD_REST								//
//																				//
//////////////////////////////////////////////////////////////////////////////////	

//LCD重要参数集
typedef struct  
{										    
	u16 width;			//LCD 宽度
	u16 height;			//LCD 高度
	u16 id;				//LCD ID
	u8  dir;			//横屏还是竖屏控制：0，竖屏；1，横屏。	
	u16	 wramcmd;		//开始写gram指令
	u16  setxcmd;		//设置x坐标指令
	u16  setycmd;		//设置y坐标指令	 
}_lcd_dev; 	

//LCD参数
extern _lcd_dev lcddev;	//管理LCD重要参数

#define USE_HORIZONTAL  	1	//定义是否使用横屏 		0,不使用.1,使用.
  
//定义LCD的尺寸
#if USE_HORIZONTAL==1	//使用横屏
#define LCD_W 320
#define LCD_H 240
#else
#define LCD_W 240
#define LCD_H 320
#endif

//TFTLCD部分外要调用的函数		   
extern u16  POINT_COLOR;//默认红色    
extern u16  BACK_COLOR; //背景颜色.默认为白色

#define	LCD_CS_SET  GPIOB->BSRRL = 1<<0 		//片选端口
#define	LCD_RS_SET	GPIOB->BSRRL = 1<<1 		//数据/命令	   
#define	LCD_WR_SET	GPIOB->BSRRL = 1<<12		//写数据	
#define	LCD_RD_SET	GPIOB->BSRRL = 1<<13		//读数据	
#define LCD_RST_SET GPIOB->BSRRL = 1<<14		//REST   			 
  
#define	LCD_CS_CLR  GPIOB->BSRRH = 1<<0			//片选端口  	    
#define	LCD_RS_CLR	GPIOB->BSRRH = 1<<1 		//数据/命令         
#define	LCD_WR_CLR	GPIOB->BSRRH = 1<<12		//写数据	
#define	LCD_RD_CLR	GPIOB->BSRRH = 1<<13		//读数据
#define LCD_RST_CLR GPIOB->BSRRH = 1<<14		//REST	

#define DATAIN	GPIOE->IDR;
#define DATAOUT(x)	GPIOE->ODR = x;

//扫描方向定义
#define L2R_U2D  0 //从左到右,从上到下
#define L2R_D2U  1 //从左到右,从下到上
#define R2L_U2D  2 //从右到左,从上到下
#define R2L_D2U  3 //从右到左,从下到上

#define U2D_L2R  4 //从上到下,从左到右
#define U2D_R2L  5 //从上到下,从右到左
#define D2U_L2R  6 //从下到上,从左到右
#define D2U_R2L  7 //从下到上,从右到左

#define DFT_SCAN_DIR  L2R_U2D  //默认的扫描方向


//画笔颜色
#define WHITE       	0xFFFF
#define BLACK      		0x0000	  
#define BLUE       		0x001F  
#define BRED        	0XF81F
#define GRED			0XFFE0		
#define GBLUE			0X07FF
#define RED         	0xF800
#define MAGENTA     	0xF81F		//品红
#define GREEN       	0x07E0
#define CYAN        	0x7FFF		//青色
#define YELLOW      	0xFFE0		
#define BROWN 			0XBC40		//棕色
#define BRRED 			0XFC07		//棕红色
#define GRAY  			0X8430		//灰色

//GUI颜色
#define DARKBLUE      	 0X01CF		//深蓝色
#define LIGHTBLUE      	 0X7D7C		//浅蓝色  
#define GRAYBLUE       	 0X5458		//灰蓝色
//以上三色为PANEL的颜色 
#define LIGHTGREEN     	0X841F 		//浅绿色
#define LIGHTGRAY     	0XEF5B 		//浅灰色(PANNEL)
#define LGRAY 			0XC618 		//浅灰色(PANNEL),窗体背景色
#define LGRAYBLUE      	0XA651 		//浅灰蓝色(中间层颜色)
#define LBBLUE          0X2B12 		//浅棕蓝色(选择条目的反色)
 
extern u16 BACK_COLOR, POINT_COLOR ;  

void LCD_Init(void);
void LCD_Clear(u16 Color);	 
void LCD_SetCursor(u16 Xpos, u16 Ypos);
void LCD_DrawPoint(u16 x,u16 y);//画点
u16  LCD_ReadPoint(u16 x,u16 y); //读点
void LCD_DrawLine(u16 x1, u16 y1, u16 x2, u16 y2);
//void LCD_DrawRectangle(u16 x1, u16 y1, u16 x2, u16 y2);		   
void LCD_SetWindows(u16 xStar, u16 yStar,u16 xEnd,u16 yEnd);
void LCD_DrawPoint_16Bit(u16 color);
//u16 LCD_RD_DATA(void);//读取LCD数据									    
void LCD_WriteReg(u8 LCD_Reg, u16 LCD_RegValue);
void LCD_WR_DATA(u16 data);
u16 LCD_ReadReg(u8 LCD_Reg);
void LCD_WriteRAM_Prepare(void);
void LCD_WriteRAM(u16 RGB_Code);
u16 LCD_ReadRAM(void);		   
u16 LCD_BGR2RGB(u16 c);
void LCD_SetParam(void);

//GUI
void DrawPage(u8 *str);
void GUI_DrawPoint(u16 x,u16 y,u16 color);
void LCD_Fill(u16 sx,u16 sy,u16 ex,u16 ey,u16 color);
void LCD_DrawRectangle(u16 x1, u16 y1, u16 x2, u16 y2);
void Draw_Circle(u16 x0,u16 y0,u16 fc,u8 r);
void LCD_ShowChar(u16 x,u16 y,u16 fc, u16 bc, u8 num,u8 size,u8 mode);
void LCD_ShowNum(u16 x,u16 y,u32 num,u8 len,u8 size);
void LCD_Show2Num(u16 x,u16 y,u16 num,u8 len,u8 size,u8 mode);
void LCD_ShowString(u16 x,u16 y,u8 size,u8 *p,u8 mode);
//void GUI_DrawFont16(u16 x, u16 y, u16 fc, u16 bc, u8 *s,u8 mode);
//void GUI_DrawFont24(u16 x, u16 y, u16 fc, u16 bc, u8 *s,u8 mode);
//void GUI_DrawFont32(u16 x, u16 y, u16 fc, u16 bc, u8 *s,u8 mode);
void Show_Str(u16 x, u16 y, u16 fc, u16 bc, u8 *str,u8 size,u8 mode);
void Gui_icon(u16 x,u16 y,const unsigned char *p); 
void gui_circle(int xc, int yc,u16 c,int r, int fill);
void Gui_StrCenter(u16 x, u16 y, u16 fc, u16 bc, u8 *str,u8 size,u8 mode);
void LCD_DrawFillRectangle(u16 x1, u16 y1, u16 x2, u16 y2,u16 color);
//void PutChinese(u16 x, u16 y, u8 *c);

#endif
