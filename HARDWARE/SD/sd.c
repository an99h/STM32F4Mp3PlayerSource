#include "sd.h"
//#include "usart.h"
/*
SPI1�ײ�����
*/
void SPI1_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	SPI_InitTypeDef  SPI_InitStructure;

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);

	GPIO_PinAFConfig(GPIOA,GPIO_PinSource5,GPIO_AF_SPI1);
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource6,GPIO_AF_SPI1);
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource7,GPIO_AF_SPI1);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	GPIO_SetBits(GPIOA,GPIO_Pin_5|GPIO_Pin_6|GPIO_Pin_7);

	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;	//����SPI�������˫�������ģʽ:SPI����Ϊ˫��˫��ȫ˫��
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;						//����SPI����ģʽ:����Ϊ��SPI
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;					//����SPI�����ݴ�С:SPI���ͽ���8λ֡�ṹ
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_High; 						//ѡ���˴���ʱ�ӵ���̬:ʱ�����ո�
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;						//���ݲ����ڵڶ���ʱ����
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;				//NSS�ź���Ӳ����NSS�ܽţ����������ʹ��SSIλ������:�ڲ�NSS�ź���SSIλ����
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_256;		//���岨����Ԥ��Ƶ��ֵ:������Ԥ��ƵֵΪ256
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;					//ָ�����ݴ����MSBλ����LSBλ��ʼ:���ݴ����MSBλ��ʼ
	SPI_InitStructure.SPI_CRCPolynomial = 7;							//CRCֵ����Ķ���ʽ
	SPI_Init(SPI1, &SPI_InitStructure);  								//����SPI_InitStruct��ָ���Ĳ�����ʼ������SPIx�Ĵ���
	SPI_Cmd(SPI1, ENABLE); //ʹ��SPI����
}

//��ʼ��SD��CS����
void CSPin_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	GPIO_SetBits(GPIOC, GPIO_Pin_5);
}
void SPI1_SetSpeed(u8 SpeedSet)
{
	SPI1->CR1&=0XFFC7;
	SPI1->CR1|=SpeedSet;
	SPI_Cmd(SPI1,ENABLE);
}
u8 SPI1_ReadWriteByte(u8 TxData)
{
	u8 retry=0;
	while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET) //���ָ����SPI��־λ�������:���ͻ���ձ�־λ
	{
		retry++;
		if(retry>200)return 0;
	}
	SPI_I2S_SendData(SPI1, TxData); //ͨ������SPIx����һ������
	retry=0;

	while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET) //���ָ����SPI��־λ�������:���ܻ���ǿձ�־λ
	{
		retry++;
		if(retry>200)return 0;
	}
	return SPI_I2S_ReceiveData(SPI1); //����ͨ��SPIx������յ�����
}
/*
SD����������
*/
u8  SD_Type=0;//SD��������
////////////////////////////////////��ֲ�޸���///////////////////////////////////
//��ֲʱ��Ľӿ�
//data:Ҫд�������
//����ֵ:����������
u8 SD_SPI_ReadWriteByte(u8 data)
{
	return SPI1_ReadWriteByte(data);
}
//SD����ʼ����ʱ��,��Ҫ����
void SD_SPI_SpeedLow(void)
{
	SPI1_SetSpeed(SPI_BaudRatePrescaler_256);//���õ�����ģʽ
}
//SD������������ʱ��,���Ը�����
void SD_SPI_SpeedHigh(void)
{
	SPI1_SetSpeed(SPI_BaudRatePrescaler_8);//���õ�����ģʽ
}
///////////////////////////////////////////////////////////////////////////////////
//ȡ��ѡ��,�ͷ�SPI����
void SD_DisSelect(void)
{
	GPIO_SetBits(GPIOC,GPIO_Pin_5);
	SD_SPI_ReadWriteByte(0xff);//�ṩ�����8��ʱ��
}
//ѡ��sd��,���ҵȴ���׼��OK
//����ֵ:0,�ɹ�;1,ʧ��;
u8 SD_Select(void)
{
	GPIO_ResetBits(GPIOC,GPIO_Pin_5);
	if(SD_WaitReady()==0)return 0;//�ȴ��ɹ�
	SD_DisSelect();
	return 1;//�ȴ�ʧ��
}
//�ȴ���׼����
//����ֵ:0,׼������;����,�������
u8 SD_WaitReady(void)
{
	u32 t=0;
	do
	{
		if(SD_SPI_ReadWriteByte(0XFF)==0XFF)return 0;//OK
		t++;
	}
	while(t<0XFFF); //�ȴ�
	return 1;
}
//�ȴ�SD����Ӧ
//Response:Ҫ�õ��Ļ�Ӧֵ
//����ֵ:0,�ɹ��õ��˸û�Ӧֵ
//    ����,�õ���Ӧֵʧ��
u8 SD_GetResponse(u8 Response)
{
	u16 Count=0xFFFF;//�ȴ�����
	while ((SD_SPI_ReadWriteByte(0XFF)!=Response)&&Count)Count--;//�ȴ��õ�׼ȷ�Ļ�Ӧ
	if (Count==0)return MSD_RESPONSE_FAILURE;//�õ���Ӧʧ��
	else return MSD_RESPONSE_NO_ERROR;//��ȷ��Ӧ
}
//��sd����ȡһ�����ݰ�������
//buf:���ݻ�����
//len:Ҫ��ȡ�����ݳ���.
//����ֵ:0,�ɹ�;����,ʧ��;
u8 SD_RecvData(u8*buf,u16 len)
{
	if(SD_GetResponse(0xFE))return 1;//�ȴ�SD������������ʼ����0xFE
	while(len--)//��ʼ��������
	{
		*buf=SPI1_ReadWriteByte(0xFF);
		buf++;
	}
	//������2��αCRC��dummy CRC��
	SD_SPI_ReadWriteByte(0xFF);
	SD_SPI_ReadWriteByte(0xFF);
	return 0;//��ȡ�ɹ�
}
//��sd��д��һ�����ݰ������� 512�ֽ�
//buf:���ݻ�����
//cmd:ָ��
//����ֵ:0,�ɹ�;����,ʧ��;
u8 SD_SendBlock(u8*buf,u8 cmd)
{
	u16 t;
	if(SD_WaitReady())return 1;//�ȴ�׼��ʧЧ
	SD_SPI_ReadWriteByte(cmd);
	if(cmd!=0XFD)//���ǽ���ָ��
	{
		for(t=0; t<512; t++)SPI1_ReadWriteByte(buf[t]); //����ٶ�,���ٺ�������ʱ��
		SD_SPI_ReadWriteByte(0xFF);//����crc
		SD_SPI_ReadWriteByte(0xFF);
		t=SD_SPI_ReadWriteByte(0xFF);//������Ӧ
		if((t&0x1F)!=0x05)return 2;//��Ӧ����
	}
	return 0;//д��ɹ�
}

//��SD������һ������
//����: u8 cmd   ����
//      u32 arg  �������
//      u8 crc   crcУ��ֵ
//����ֵ:SD�����ص���Ӧ
u8 SD_SendCmd(u8 cmd, u32 arg, u8 crc)
{
	u8 r1;
	u8 Retry=0;
	SD_DisSelect();//ȡ���ϴ�Ƭѡ
	if(SD_Select())return 0XFF;//ƬѡʧЧ
	//����
	SD_SPI_ReadWriteByte(cmd | 0x40);//�ֱ�д������
	SD_SPI_ReadWriteByte(arg >> 24);
	SD_SPI_ReadWriteByte(arg >> 16);
	SD_SPI_ReadWriteByte(arg >> 8);
	SD_SPI_ReadWriteByte(arg);
	SD_SPI_ReadWriteByte(crc);
	if(cmd==CMD12)SD_SPI_ReadWriteByte(0xff);//Skip a stuff byte when stop reading
	//�ȴ���Ӧ����ʱ�˳�
	Retry=0X1F;
	do
	{
		r1=SD_SPI_ReadWriteByte(0xFF);
	}
	while((r1&0X80) && Retry--);
	//����״ֵ̬
	return r1;
}
//��ȡSD����CID��Ϣ��������������Ϣ
//����: u8 *cid_data(���CID���ڴ棬����16Byte��
//����ֵ:0��NO_ERR
//		 1������
u8 SD_GetCID(u8 *cid_data)
{
	u8 r1;
	//��CMD10�����CID
	r1=SD_SendCmd(CMD10,0,0x01);
	if(r1==0x00)
	{
		r1=SD_RecvData(cid_data,16);//����16���ֽڵ�����
	}
	SD_DisSelect();//ȡ��Ƭѡ
	if(r1)return 1;
	else return 0;
}
//��ȡSD����CSD��Ϣ�������������ٶ���Ϣ
//����:u8 *cid_data(���CID���ڴ棬����16Byte��
//����ֵ:0��NO_ERR
//		 1������
u8 SD_GetCSD(u8 *csd_data)
{
	u8 r1;
	r1=SD_SendCmd(CMD9,0,0x01);//��CMD9�����CSD
	if(r1==0)
	{
		r1=SD_RecvData(csd_data, 16);//����16���ֽڵ�����
	}
	SD_DisSelect();//ȡ��Ƭѡ
	if(r1)return 1;
	else return 0;
}
//��ȡSD����������������������
//����ֵ:0�� ȡ��������
//       ����:SD��������(������/512�ֽ�)
//ÿ�������ֽ�����Ϊ512����Ϊ�������512�����ʼ������ͨ��.
u32 SD_GetSectorCount(void)
{
	u8 csd[16];
	u32 Capacity;
	u8 n;
	u16 csize;
	//ȡCSD��Ϣ������ڼ��������0
	if(SD_GetCSD(csd)!=0) return 0;
	//���ΪSDHC�����������淽ʽ����
	if((csd[0]&0xC0)==0x40)	 //V2.00�Ŀ�
	{
		csize = csd[9] + ((u16)csd[8] << 8) + 1;
		Capacity = (u32)csize << 10;//�õ�������
	}
	else //V1.XX�Ŀ�
	{
		n = (csd[5] & 15) + ((csd[10] & 128) >> 7) + ((csd[9] & 3) << 1) + 2;
		csize = (csd[8] >> 6) + ((u16)csd[7] << 2) + ((u16)(csd[6] & 3) << 10) + 1;
		Capacity= (u32)csize << (n - 9);//�õ�������
	}
	return Capacity;
}
//��ʼ��SD��
u8 SD_Initialize(void)
{
	u8 r1;      // ���SD���ķ���ֵ
	u16 retry;  // �������г�ʱ����
	u8 buf[4];
	u16 i;

	CSPin_Init();
	SPI1_Init();		//��ʼ��IO
	SD_SPI_SpeedLow();	//���õ�����ģʽ

	for(i=0; i<10; i++)SD_SPI_ReadWriteByte(0XFF); //��������74������
	retry=20;
	do
	{
		r1=SD_SendCmd(CMD0,0,0x95);//����IDLE״̬
		//printf("r1 = %X retry = %X\n\r",r1,retry);
	}
	while((r1!=0X01) && retry--);
	SD_Type=0;//Ĭ���޿�
	if(r1==0X01)
	{
		if(SD_SendCmd(CMD8,0x1AA,0x87)==1)//SD V2.0
		{
			for(i=0; i<4; i++)buf[i]=SD_SPI_ReadWriteByte(0XFF);	//Get trailing return value of R7 resp
			if(buf[2]==0X01&&buf[3]==0XAA)//���Ƿ�֧��2.7~3.6V
			{
				retry=0XFFFE;
				do
				{
					SD_SendCmd(CMD55,0,0X01);	//����CMD55
					r1=SD_SendCmd(CMD41,0x40000000,0X01);//����CMD41

				}
				while(r1&&retry--);
				if(retry&&SD_SendCmd(CMD58,0,0X01)==0)//����SD2.0���汾��ʼ
				{
					for(i=0; i<4; i++)buf[i]=SD_SPI_ReadWriteByte(0XFF); //�õ�OCRֵ
					if(buf[0]&0x40)SD_Type=SD_TYPE_V2HC;    //���CCS
					else SD_Type=SD_TYPE_V2;
				}
			}
		}
		else //SD V1.x/ MMC	V3
		{
			SD_SendCmd(CMD55,0,0X01);		//����CMD55
			r1=SD_SendCmd(CMD41,0,0X01);	//����CMD41
			if(r1<=1)
			{
				SD_Type=SD_TYPE_V1;
				retry=0XFFFE;
				do //�ȴ��˳�IDLEģʽ
				{
					SD_SendCmd(CMD55,0,0X01);	//����CMD55
					r1=SD_SendCmd(CMD41,0,0X01);//����CMD41
				}
				while(r1&&retry--);
			}
			else
			{
				SD_Type=SD_TYPE_MMC;//MMC V3
				retry=0XFFFE;
				do //�ȴ��˳�IDLEģʽ
				{
					r1=SD_SendCmd(CMD1,0,0X01);//����CMD1
				}
				while(r1&&retry--);
			}
			if(retry==0||SD_SendCmd(CMD16,512,0X01)!=0)SD_Type=SD_TYPE_ERR;//����Ŀ�
		}
	}
	SD_DisSelect();//ȡ��Ƭѡ
	SD_SPI_SpeedHigh();//����
	if(SD_Type)return 0;
	else if(r1)return r1;
	return 0xaa;//��������
}
//��SD��
//buf:���ݻ�����
//sector:����
//cnt:������
//����ֵ:0,ok;����,ʧ��.
u8 SD_ReadDisk(u8*buf,u32 sector,u8 cnt)
{
	u8 r1;
	if(SD_Type!=SD_TYPE_V2HC)sector <<= 9;//ת��Ϊ�ֽڵ�ַ
	if(cnt==1)
	{
		r1=SD_SendCmd(CMD17,sector,0X01);//������
		if(r1==0)//ָ��ͳɹ�
		{
			r1=SD_RecvData(buf,512);//����512���ֽ�
		}
	}
	else
	{
		r1=SD_SendCmd(CMD18,sector,0X01);//����������
		do
		{
			r1=SD_RecvData(buf,512);//����512���ֽ�
			buf+=512;
		}
		while(--cnt && r1==0);
		SD_SendCmd(CMD12,0,0X01);	//����ֹͣ����
	}
	SD_DisSelect();//ȡ��Ƭѡ
	return r1;//
}
//дSD��
//buf:���ݻ�����
//sector:��ʼ����
//cnt:������
//����ֵ:0,ok;����,ʧ��.
u8 SD_WriteDisk(u8*buf,u32 sector,u8 cnt)
{
	u8 r1;
	if(SD_Type!=SD_TYPE_V2HC)sector *= 512;//ת��Ϊ�ֽڵ�ַ
	if(cnt==1)
	{
		r1=SD_SendCmd(CMD24,sector,0X01);//������
		if(r1==0)//ָ��ͳɹ�
		{
			r1=SD_SendBlock(buf,0xFE);//д512���ֽ�
		}
	}
	else
	{
		if(SD_Type!=SD_TYPE_MMC)
		{
			SD_SendCmd(CMD55,0,0X01);
			SD_SendCmd(CMD23,cnt,0X01);//����ָ��
		}
		r1=SD_SendCmd(CMD25,sector,0X01);//����������
		if(r1==0)
		{
			do
			{
				r1=SD_SendBlock(buf,0xFC);//����512���ֽ�
				buf+=512;
			}
			while(--cnt && r1==0);
			r1=SD_SendBlock(0,0xFD);//����512���ֽ�
		}
	}
	SD_DisSelect();//ȡ��Ƭѡ
	return r1;//
}



u32 total,free1;
void get_TF_free(void)
{
	while(exf_getfree("1:",&total,&free1))	//�õ�SD������������ʣ������
	{
		//	LCD_ShowString(10,60,16,"SD Card Fatfs Error!",1);
		delay_ms(200);
	}
	//LCD_ShowString(10,179,16,"TF ���ܿռ�  :    MB",1);
	//LCD_ShowString(10,195,16,"TF ��ʣ��ռ�:    MB",1);
	Show_Str(10,179,WHITE,BLACK,"TF ���ܿռ�  :    MB",16,0);
	Show_Str(10,195,WHITE,BLACK,"TF ��ʣ��ռ�:    MB",16,0);
	LCD_ShowNum(10+8*14,179,total>>10,4,16);				//��ʾSD�������� MB
	LCD_ShowNum(10+8*14,195,free1>>10,4,16);				//��ʾSD��ʣ������ MB
}

void get_USB_free(void)
{
	while(exf_getfree("0:",&total,&free1))	//�õ�SD������������ʣ������
	{
		//	LCD_ShowString(10,60,16,"USB Fatfs Error!",1);
		delay_ms(200);
	}
	//LCD_ShowString(10,179,16,"USB �ܿռ�    :     MB",1);
	//LCD_ShowString(10,195,16,"USB ʣ��ռ�  :     MB",1);
	Show_Str(10,179,WHITE,BLACK,"USB �ܿռ�    :     MB",16,0);
	Show_Str(10,195,WHITE,BLACK,"USB ʣ��ռ�  :     MB",16,0);
	LCD_ShowNum(10+8*15,179,total>>10,5,16);				//��ʾUSB������ MB
	LCD_ShowNum(10+8*15,195,free1>>10,5,16);				//��ʾUSBʣ������ MB
}


