#include "sys.h"
#include "usart.h"
//////////////////////////////////////////////////////////////////////////////////
//���ʹ��ucos,����������ͷ�ļ�����.
#if SYSTEM_SUPPORT_UCOS
#include "includes.h"					//ucos ʹ��	  
#endif

//////////////////////////////////////////////////////////////////
//�������´���,֧��printf����,������Ҫѡ��use MicroLIB
void usart_ctrl_music(void);
#if 1
#pragma import(__use_no_semihosting)
//��׼����Ҫ��֧�ֺ���
struct __FILE
{
	int handle;
};

FILE __stdout;
//����_sys_exit()�Ա���ʹ�ð�����ģʽ
_sys_exit(int x)
{
	x = x;
}
//�ض���fputc����
int fputc(int ch, FILE *f)
{
	while((USART3->SR&0X40)==0);//ѭ������,ֱ���������
	USART3->DR = (u8) ch;
	return ch;
}
#endif

#if EN_USART3_RX   //���ʹ���˽���
//����3�жϷ������
//ע��,��ȡUSARTx->SR�ܱ���Ī������Ĵ���
u8 USART_RX_BUF[USART_REC_LEN];     //���ջ���,���USART_REC_LEN���ֽ�.
u8  USART_RX_TEMP[USART_REC_LEN];	//��ʱ�������ݣ����ⲿ����
//����״̬
//bit15��	������ɱ�־
//bit14��	���յ�0x0d
//bit13~0��	���յ�����Ч�ֽ���Ŀ
u16 USART_RX_STA=0;       //����״̬���

//��ʼ��IO ����3
//bound:������
void uart_init(u32 bound)
{
	//GPIO�˿�����
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB,ENABLE); //ʹ��GPIOBʱ��
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3,ENABLE);//ʹ��USART3ʱ��

	//����3��Ӧ���Ÿ���ӳ��
	GPIO_PinAFConfig(GPIOB,GPIO_PinSource10,GPIO_AF_USART3); //GPIOB10����ΪUSART3
	GPIO_PinAFConfig(GPIOB,GPIO_PinSource11,GPIO_AF_USART3); //GPIOB11����ΪUSART3

	//USART3�˿�����
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10 | GPIO_Pin_11; //GPIOB10��GPIOB11
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;			//���ù���
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		//�ٶ�50MHz
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; 			//���츴�����
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP; 			//����
	GPIO_Init(GPIOB,&GPIO_InitStructure); 					//��ʼ��PB10��PB11

	//USART3 ��ʼ������
	USART_InitStructure.USART_BaudRate = bound;					//����������
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//�ֳ�Ϊ8λ���ݸ�ʽ
	USART_InitStructure.USART_StopBits = USART_StopBits_1;		//һ��ֹͣλ
	USART_InitStructure.USART_Parity = USART_Parity_No;			//����żУ��λ
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//��Ӳ������������
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//�շ�ģʽ
	USART_Init(USART3, &USART_InitStructure); 					//��ʼ������3

	USART_Cmd(USART3, ENABLE);  							//ʹ�ܴ���3

	USART_ClearFlag(USART3, USART_FLAG_TC);

#if EN_USART3_RX
	USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);			//��������ж�

	//Usart1 NVIC ����
	NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;		//����3�ж�ͨ��
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3;//��ռ���ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority =3;		//�����ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQͨ��ʹ��
	NVIC_Init(&NVIC_InitStructure);							//����ָ���Ĳ�����ʼ��VIC�Ĵ�����

#endif

}

//����3�жϷ������
void USART3_IRQHandler(void)                	
{
	u8 Res;
#ifdef OS_TICKS_PER_SEC	 	//���ʱ�ӽ�����������,˵��Ҫʹ��ucosII��.
	OSIntEnter();
#endif
	if(USART_GetITStatus(USART3, USART_IT_RXNE) != RESET)  //�����ж�(���յ������ݱ�����0x0d 0x0a��β)
	{
		Res =USART_ReceiveData(USART3);//(USART3->DR);	//��ȡ���յ�������

		if((USART_RX_STA&0x8000)==0)//����δ���
		{
			if(USART_RX_STA&0x4000)//���յ���0x0d
			{
				if(Res!=0x0a)
					USART_RX_STA=0;//���մ���,���¿�ʼ
				else
				{
					USART_RX_STA|=0x8000;	//���������
					strcpy((char*)USART_RX_TEMP,(char*)USART_RX_BUF);//copy���յ������ݣ����ⲿ����
					strncpy((char*)USART_RX_BUF," ",30);//���USART_RX_BUF
					usart_ctrl_music();					//������������������ֲ���
					USART_RX_STA=0;
				}
			}
			else //��û�յ�0X0D
			{
				if(Res==0x0d)USART_RX_STA|=0x4000;
				else
				{
					USART_RX_BUF[USART_RX_STA&0X3FFF]=Res ;
					USART_RX_STA++;
					if(USART_RX_STA>(USART_REC_LEN-1))
						USART_RX_STA=0;//�������ݴ���,���¿�ʼ����
				}
			}
		}
	}
#ifdef OS_TICKS_PER_SEC	 	//���ʱ�ӽ�����������,˵��Ҫʹ��ucosII��.
	OSIntExit();
#endif
}

void usart_ctrl_music(void)
{
	if(!strncmp((char*)USART_RX_TEMP,"MUSIC_MODE",11))
	{
		MUSIC_MODE_KEY = 1;
		delay_ms(100);
		MUSIC_MODE_KEY = 0;
	}
	else if(!strncmp((char*)USART_RX_TEMP,"PAGE_DOWN",9))
	{
		PAGE_DOWN_KEY = 0;
		delay_ms(100);
		PAGE_DOWN_KEY = 1;
	}
	else if(!strncmp((char*)USART_RX_TEMP,"NEXT_SONG",9))
	{
		NEXT_SONG_KEY = 0;
		delay_ms(100);
		NEXT_SONG_KEY = 1;
	}
	else if(!strncmp((char*)USART_RX_TEMP,"VOL_UP",6))
	{
		VOL_UP_KEY = 0;
		delay_ms(100);
		VOL_UP_KEY = 1;
	}
	else if(!strncmp((char*)USART_RX_TEMP,"PAUSE_SONG",11))
	{
		PAUSE_SONG_KEY = 0;
		delay_ms(100);
		PAUSE_SONG_KEY = 1;
	}
	else if(!strncmp((char*)USART_RX_TEMP,"VOL_DOWN",8))
	{
		VOL_DOWN_KEY = 0;
		delay_ms(100);
		VOL_DOWN_KEY = 1;
	}
	else if(!strncmp((char*)USART_RX_TEMP,"STOP_SONG",9))
	{
		STOP_SONG_KEY = 0;
		delay_ms(100);
		STOP_SONG_KEY = 1;
	}
	else if(!strncmp((char*)USART_RX_TEMP,"PAGE_UP",7))
	{
		PAGE_UP_KEY = 0;
		delay_ms(100);
		PAGE_UP_KEY = 1;
	}
	else if(!strncmp((char*)USART_RX_TEMP,"LAST_SONG",9))
	{
		LAST_SONG_KEY = 0;
		delay_ms(100);
		LAST_SONG_KEY = 1;
	}
	else
		return;
}
#endif





