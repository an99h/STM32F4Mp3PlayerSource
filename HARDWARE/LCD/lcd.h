#ifndef __ILI9341_LCD_H
#define	__ILI9341_LCD_H

#include "stm32f4xx.h"
#include "stdlib.h"
#include "string.h"

//////////////////////////////////////////////////////////////////////////////////	
//																				//
//					ILI9341 8λ������		2.4��LCD��Ļ��������					//
//																				//
//					�����ߣ�	PE0--------->LCD_D0									//
//						 	PE1--------->LCD_D1									//
//							PE2--------->LCD_D2									//
//							PE3--------->LCD_D3									//
//							PE4--------->LCD_D4									//
//							PE5--------->LCD_D5									//
//							PE6--------->LCD_D6									//
//							PE7--------->LCD_D7									//
//																				//
//					�����ߣ�	PB0 --------->LCD_CS								//
//							PB1 --------->LCD_RS								//
//							PB12--------->LCD_WR								//
//							PB13--------->LCD_WD								//
//							PB14--------->LCD_REST								//
//																				//
//////////////////////////////////////////////////////////////////////////////////	

//LCD��Ҫ������
typedef struct  
{										    
	u16 width;			//LCD ���
	u16 height;			//LCD �߶�
	u16 id;				//LCD ID
	u8  dir;			//���������������ƣ�0��������1��������	
	u16	 wramcmd;		//��ʼдgramָ��
	u16  setxcmd;		//����x����ָ��
	u16  setycmd;		//����y����ָ��	 
}_lcd_dev; 	

//LCD����
extern _lcd_dev lcddev;	//����LCD��Ҫ����

#define USE_HORIZONTAL  	1	//�����Ƿ�ʹ�ú��� 		0,��ʹ��.1,ʹ��.
  
//����LCD�ĳߴ�
#if USE_HORIZONTAL==1	//ʹ�ú���
#define LCD_W 320
#define LCD_H 240
#else
#define LCD_W 240
#define LCD_H 320
#endif

//TFTLCD������Ҫ���õĺ���		   
extern u16  POINT_COLOR;//Ĭ�Ϻ�ɫ    
extern u16  BACK_COLOR; //������ɫ.Ĭ��Ϊ��ɫ

#define	LCD_CS_SET  GPIOB->BSRRL = 1<<0 		//Ƭѡ�˿�
#define	LCD_RS_SET	GPIOB->BSRRL = 1<<1 		//����/����	   
#define	LCD_WR_SET	GPIOB->BSRRL = 1<<12		//д����	
#define	LCD_RD_SET	GPIOB->BSRRL = 1<<13		//������	
#define LCD_RST_SET GPIOB->BSRRL = 1<<14		//REST   			 
  
#define	LCD_CS_CLR  GPIOB->BSRRH = 1<<0			//Ƭѡ�˿�  	    
#define	LCD_RS_CLR	GPIOB->BSRRH = 1<<1 		//����/����         
#define	LCD_WR_CLR	GPIOB->BSRRH = 1<<12		//д����	
#define	LCD_RD_CLR	GPIOB->BSRRH = 1<<13		//������
#define LCD_RST_CLR GPIOB->BSRRH = 1<<14		//REST	

#define DATAIN	GPIOE->IDR;
#define DATAOUT(x)	GPIOE->ODR = x;

//ɨ�跽����
#define L2R_U2D  0 //������,���ϵ���
#define L2R_D2U  1 //������,���µ���
#define R2L_U2D  2 //���ҵ���,���ϵ���
#define R2L_D2U  3 //���ҵ���,���µ���

#define U2D_L2R  4 //���ϵ���,������
#define U2D_R2L  5 //���ϵ���,���ҵ���
#define D2U_L2R  6 //���µ���,������
#define D2U_R2L  7 //���µ���,���ҵ���

#define DFT_SCAN_DIR  L2R_U2D  //Ĭ�ϵ�ɨ�跽��


//������ɫ
#define WHITE       	0xFFFF
#define BLACK      		0x0000	  
#define BLUE       		0x001F  
#define BRED        	0XF81F
#define GRED			0XFFE0		
#define GBLUE			0X07FF
#define RED         	0xF800
#define MAGENTA     	0xF81F		//Ʒ��
#define GREEN       	0x07E0
#define CYAN        	0x7FFF		//��ɫ
#define YELLOW      	0xFFE0		
#define BROWN 			0XBC40		//��ɫ
#define BRRED 			0XFC07		//�غ�ɫ
#define GRAY  			0X8430		//��ɫ

//GUI��ɫ
#define DARKBLUE      	 0X01CF		//����ɫ
#define LIGHTBLUE      	 0X7D7C		//ǳ��ɫ  
#define GRAYBLUE       	 0X5458		//����ɫ
//������ɫΪPANEL����ɫ 
#define LIGHTGREEN     	0X841F 		//ǳ��ɫ
#define LIGHTGRAY     	0XEF5B 		//ǳ��ɫ(PANNEL)
#define LGRAY 			0XC618 		//ǳ��ɫ(PANNEL),���屳��ɫ
#define LGRAYBLUE      	0XA651 		//ǳ����ɫ(�м����ɫ)
#define LBBLUE          0X2B12 		//ǳ����ɫ(ѡ����Ŀ�ķ�ɫ)
 
extern u16 BACK_COLOR, POINT_COLOR ;  

void LCD_Init(void);
void LCD_Clear(u16 Color);	 
void LCD_SetCursor(u16 Xpos, u16 Ypos);
void LCD_DrawPoint(u16 x,u16 y);//����
u16  LCD_ReadPoint(u16 x,u16 y); //����
void LCD_DrawLine(u16 x1, u16 y1, u16 x2, u16 y2);
//void LCD_DrawRectangle(u16 x1, u16 y1, u16 x2, u16 y2);		   
void LCD_SetWindows(u16 xStar, u16 yStar,u16 xEnd,u16 yEnd);
void LCD_DrawPoint_16Bit(u16 color);
//u16 LCD_RD_DATA(void);//��ȡLCD����									    
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
