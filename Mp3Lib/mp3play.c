/**
  ******************************************************************************
  * @file    Audio_playback_and_record/src/Mp3player.c
  * @author  MCD Application Team
  * @version V1.0.0
  * @date    28-October-2011
  * @brief   I2S audio program
  ******************************************************************************
  * @attention
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2011 STMicroelectronics</center></h2>
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "mp3dec.h"
#include "mp3common.h"
#include <string.h>
#include "lcd.h"
#include "delay.h"
#include "malloc.h"
/* Private macro -------------------------------------------------------------*/
#if 0
u8 mp3_id3v1_decode(u8* buf,__mp3ctrl *pctrl);
u8 mp3_id3v2_decode(u8* buf,u32 size,__mp3ctrl *pctrl);
u8 mp3_get_info(u8 *pname,__mp3ctrl* pctrl);
void mp3_get_curtime(FIL*fx,__mp3ctrl *mp3x);
#endif

/* Private variables ---------------------------------------------------------*/

//#define AUDIO_MIN(x,y)	((x)<(y)? (x):(y))
uint8_t buffer_switch = 1;
//__mp3ctrl * mp3ctrl;
char buff[12];

#define READBUF_SIZE 4000
#define BUFF_SIZE 2304

HMP3Decoder hMP3Decoder;
MP3FrameInfo mp3FrameInfo;
int bytesLeft;

u8  readBuf[READBUF_SIZE];
u8 *readPtr;
int offset;
short buffer1[BUFF_SIZE] = {0x00};
short buffer2[BUFF_SIZE] = {0x00};
short 	*Decbuf;
u8 init;
u8 music_mode = 1;				//音乐模式，1:全部循环2:单曲循环3:随机播放

//char *music_name = NULL;
u8 music_curindex = 0;

extern FATFS fatfs;
extern FIL file;
extern FIL fileR;
extern DIR dir;
extern FILINFO fno;
extern uint16_t *CurrentPos;
extern USB_OTG_CORE_HANDLE USB_OTG_Core;
extern uint8_t Mp3RecStatus;

__IO uint32_t XferCplt = 0;
__IO uint8_t volume = 50, AudioPlayStart = 0;
__IO uint32_t Mp3Counter;
uint8_t Buffer[6];
__IO uint32_t Mp3DataLength = 0;
extern __IO uint8_t Count;
extern __IO uint8_t RepeatState ;
extern __IO uint8_t LED_Toggle;
extern __IO uint8_t PauseResumeStatus ;
extern uint32_t AudioRemSize;
static __IO uint32_t TimingDelay;


/**
  * @brief  Pause or Resume a played Mp3
  * @param  state: if it is equal to 0 pause Playing else resume playing
  * @retval None
  */
void Mp3PlayerPauseResume(uint8_t state)
{
	EVAL_AUDIO_PauseResume(state);
}

/**
  * @brief  Configure the volune
  * @param  vol: volume value
  * @retval None
  */
uint8_t Mp3playerCtrlVolume(uint8_t vol)
{
	EVAL_AUDIO_VolumeCtl(vol);
	return 0;
}


/**
  * @brief  Stop playing Mp3
  * @param  None
  * @retval None
  */
void Mp3PlayerStop(void)
{
	EVAL_AUDIO_Stop(CODEC_PDWN_SW);
}

/**
* @brief  Initializes the Mp3 player
* @param  AudioFreq: Audio sampling frequency
* @retval None
*/
int Mp3PlayerInit(uint32_t AudioFreq)
{

	/* Initialize I2S interface */
	EVAL_AUDIO_SetAudioInterface(AUDIO_INTERFACE_I2S);

	/* Initialize the Audio codec and all related peripherals (I2S, I2C, IOExpander, IOs...) */
	EVAL_AUDIO_Init(OUTPUT_DEVICE_AUTO, volume, AudioFreq );

	return 0;
}



/*--------------------------------
Callbacks implementation:
the callbacks prototypes are defined in the stm324xg_eval_audio_codec.h file
and their implementation should be done in the user code if they are needed.
Below some examples of callback implementations.
--------------------------------------------------------*/
/**
* @brief  Calculates the remaining file size and new position of the pointer.
* @param  None
* @retval None
*/
void EVAL_AUDIO_TransferComplete_CallBack(uint32_t pBuffer, uint32_t Size)
{
	/* Calculate the remaining audio data in the file and the new size
	for the DMA transfer. If the Audio files size is less than the DMA max
	data transfer size, so there is no calculation to be done, just restart
	from the beginning of the file ... */
	/* Check if the end of file has been reached */

#ifdef AUDIO_MAL_MODE_NORMAL

	XferCplt = 1;

#else /* #ifdef AUDIO_MAL_MODE_CIRCULAR */


#endif /* AUDIO_MAL_MODE_CIRCULAR */
}


/**
* @brief  Get next data sample callback
* @param  None
* @retval Next data sample to be sent
*/
uint16_t EVAL_AUDIO_GetSampleCallBack(void)
{
	return 0;
}


#ifndef USE_DEFAULT_TIMEOUT_CALLBACK
/**
  * @brief  Basic management of the timeout situation.
  * @param  None.
  * @retval None.
  */
uint32_t Codec_TIMEOUT_UserCallback(void)
{
	return (0);
}
#endif /* USE_DEFAULT_TIMEOUT_CALLBACK */
/*----------------------------------------------------------------------------*/

/**
  * @brief  Inserts a delay time.
  * @param  nTime: specifies the delay time length, in 10 ms.
  * @retval None
  */
void Delay(__IO uint32_t nTime)
{
	TimingDelay = nTime;

	while(TimingDelay != 0);
}

/**
  * @brief  Decrements the TimingDelay variable.
  * @param  None
  * @retval None
  */
void TimingDelay_Decrement(void)
{
	if (TimingDelay != 0x00)
	{
		TimingDelay--;
	}
}

#if defined MEDIA_USB_KEY

/**
  * @brief  Start Mp3 player
  * @param  None
  * @retval None
  */
void Mp3PlayerStart(u8 *path)
{
	u32 br;
	FRESULT res;
	u8 temp = 0;
	u8 mp3indextbl[40];
	char *fn = NULL;
	char fpath[30];					//带路径的歌曲名，最多支持30个字符
	u8 curindex = 0;
	u8 music_sum = 0;
	KEY_PRESS key = NO_KEY_PRESS;

	srand(6);
	hMP3Decoder = MP3InitDecoder();	//初始化MP3解码器
	if (f_opendir(&dir, (const XCHAR*)path) != FR_OK)
	{
		LCD_Clear(BLACK);
		LCD_ShowString(40,20,16,"f_opendir error",1);
		return;
	}
	else
	{
		fn = (char *)malloc(sizeof(fno.fname));
		while(1)
		{
			temp=dir.index;
			res=f_readdir(&dir,&fno);  
			if(res!=FR_OK||fno.fname[0]==0)break;
			fn = fno.fname;
			if(strstr(fn,".mp3") != NULL)
			{
				mp3indextbl[curindex]=temp;	
				curindex++;
				music_sum++;
			}
		}
		music_sum -= 1;									//不知道为什么索引文件是把第一个文件索引两次
		fn = NULL;
		curindex = 0;
		res = f_opendir(&dir,(const XCHAR*)path); 
		while(res == FR_OK)
		{
			LCD_Fill(1,88,190,120,BLACK);
			dir_seek(&dir,mp3indextbl[curindex]);		//改变当前目录索引
			res = f_readdir(&dir, &fno);
			if (res != FR_OK || fno.fname[0] == 0)break;
			fn = fno.fname;
			strcpy(fpath, (const char*)path);
			strcat(fpath,"/");
			strcat(fpath, fn);							//fpath格式	0:/MUSIC/xxx.mp3

			music_curindex = curindex;					//保存当前播放音乐索引位置用于UI任务调用

			//LCD_ShowString(20,98,16,(u8*)fpath,1);
			Show_Str(20,98,WHITE,BLACK,(u8*)fpath,16,0);
			
			LCD_Fill(144,123,194,135,BLACK);
			LCD_ShowNum(144,123,curindex+1,2,12);
			LCD_ShowString(160,123,12,"/",1);			//显示当前歌曲数/总歌曲数
			LCD_ShowNum(170,123,music_sum+1,2,12);
			
			reset_time();								//重置当前播放时间
#if 0			
			temp = mp3_get_info((u8*)fpath,mp3ctrl);
			if(temp == 0)
			{
				printf("     title:%s\r\n",mp3ctrl->title);
				printf("    artist:%s\r\n",mp3ctrl->artist);
				printf("   bitrate:%dbps\r\n",mp3ctrl->bitrate);
				printf("samplerate:%d\r\n", mp3ctrl->samplerate);
				printf("  totalsec:%d\r\n",mp3ctrl->totsec);
			}
			else
				printf("mp3_get_info error!!!\r\n");
#endif
			if(f_open(&fileR, fpath , FA_READ) != FR_OK)
				LCD_ShowString(40,70,16,"f_open music error",1);
			else
			{
				//复位计数器
				bytesLeft=0;
				readPtr=readBuf;
				res = f_read(&fileR,readBuf,READBUF_SIZE,&br);
				bytesLeft += br;
				if(res!=FR_OK||br==0) break;
				XferCplt=1;
				buffer_switch=0;
				init=0;
				Mp3PlayerInit((u32)44100);
				while(1)
				{
					offset=MP3FindSyncWord(readPtr, bytesLeft);//寻找下一帧头	assume EOF if no sync found
					if(offset<0)break;
					readPtr+=offset; //data start point
					bytesLeft-=offset; //in buffer
					if(bytesLeft<READBUF_SIZE)
					{
						memmove(readBuf,readPtr,bytesLeft);
						res=f_read(&fileR,readBuf+bytesLeft,READBUF_SIZE-bytesLeft,&br);
						if((res)||(br==0)) break;
						if(br<READBUF_SIZE-bytesLeft)
							memset(readBuf+bytesLeft+br,0,READBUF_SIZE-bytesLeft-br);
						bytesLeft=READBUF_SIZE;
						readPtr=readBuf;
					}
					MP3GetLastFrameInfo(hMP3Decoder, &mp3FrameInfo);

					while(XferCplt==0);
					XferCplt=0;
					
					//mp3_get_curtime(&fileR,mp3ctrl);
					
					if(buffer_switch == 0)
					{
						Audio_MAL_Play((u32)buffer2,BUFF_SIZE*2);
						MP3Decode(hMP3Decoder,&readPtr,&bytesLeft,buffer1,0);
						buffer_switch = 1;
					}
					else
					{
						Audio_MAL_Play((u32)buffer1,BUFF_SIZE*2);
						MP3Decode(hMP3Decoder,&readPtr,&bytesLeft,buffer2,0);
						buffer_switch = 0;
					}
					key = KEY_Scan(0);
					temp = music_ctrl(key,&curindex, music_sum);//音乐播放控制
					if(temp == 1)break;
				}//end while(1)
				if(music_mode == 1)
				{
					if(key == NO_KEY_PRESS)						//自动切换下一曲
					{
						if(curindex == music_sum)
							curindex = 0;
						else
							curindex++;
					}
				}
				else if(music_mode == 3)
					curindex = rand()%music_sum;				//音乐索引位置获得随机值（0~music_sum）
			}// end if(f_open(mp3) == FR_OK)
		}//end while(res == FR_OK)
	}
}

//音乐播放控制
u8 music_ctrl(KEY_PRESS key,u8 *curindex,u8 music_sum)
{
	if(key == LAST_SONG)						//上一曲
	{
		if(*curindex > 0)
			(*curindex)--;
		else
			*curindex = music_sum;
		return 1;
	}
	else if(key == NEXT_SONG)					//下一曲
	{
		if(*curindex == music_sum)
			*curindex = 0;
		else
			(*curindex)++;
		return 1;
	}
	else if(key == VOL_UP)						//音量+
	{
		if(volume == 95)
			volume = 95;
		else
			volume += 5;
		Mp3playerCtrlVolume(volume);
		LCD_ShowNum(80+8*4,30,volume,2,16);
	}
	else if(key == VOL_DOWN)					//音量-
	{
		if(volume == 0)
			volume = 0;
		else
			volume -= 5;
		Mp3playerCtrlVolume(volume);
		LCD_ShowNum(80+8*4,30,volume,2,16);
	}
	else if(key == MUSIC_MODE)					//音乐播放模式
	{
		music_mode++;
		LCD_Fill(176,30,184,46,BLACK);
		if(music_mode == 2)
			LCD_ShowString(176,30,16,"1",1);	//单曲循环
		else if(music_mode == 3)
			LCD_ShowString(176,30,16,"X",1);	//随机播放
		else if(music_mode == 4)
		{
			music_mode = 1;
			LCD_ShowString(176,30,16,"O",1);	//循环播放
		}
	}
	return 0;
}

void AUDIO_TransferComplete(u32 pBuffer, u32 Size)
{
	XferCplt=1;
}

//获取当前播放音乐索引位置返回给UI任务
u8 get_file_curindex(void)
{
	return music_curindex;
}

//获取当前播放音量
u8 get_volume(void)
{
	return volume;
}

#if 0
//获取当前播放时间
char * get_music_time(void)
{

	LCD_Fill(10,123,100,135,BLACK);
	if(cur_sec < 10)
		sprintf(buff,"0%d:0%d/00:00",cur_min,cur_sec);
//	else if(tot_sec < 10)
//		sprintf(buff,"0%d:%d/0%d:0%d",cur_min,cur_sec,tot_min,tot_sec);
	else
		sprintf(buff,"0%d:%d/00:00",cur_min,cur_sec);
	cur_sec++;
	if(cur_sec == 60)
	{
		cur_sec = 0;
		cur_min++;
	}
	return buff;
}
#endif
/**
  * @brief  Reset the Mp3 player
  * @param  None
  * @retval None
  */
void Mp3Player_CallBack(void)
{
	/* Stops the codec */
	EVAL_AUDIO_Stop(CODEC_PDWN_HW);
	/* Reset the Mp3 player variables */
	RepeatState = 0;
	AudioPlayStart =0;
	LED_Toggle = 7;
	PauseResumeStatus = 1;
	Mp3DataLength =0;
	Count = 0;

	/* LED off */
	//STM_EVAL_LEDOff(LED3);
	//STM_EVAL_LEDOff(LED4);
	//STM_EVAL_LEDOff(LED6);

	/* TIM Interrupts disable */
	TIM_ITConfig(TIM4, TIM_IT_CC1, DISABLE);
	f_mount(0,NULL);
}
#endif

#ifdef  USE_FULL_ASSERT

/**
* @brief  Reports the name of the source file and the source line number
*   where the assert_param error has occurred.
* @param  file: pointer to the source file name
* @param  line: assert_param error line source number
* @retval None
*/
void assert_failed(uint8_t* file, uint32_t line)
{
	/* User can add his own implementation to report the file name and line number,
	ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

	/* Infinite loop */
	while (1)
	{
	}
}
#endif

/**
* @}
*/
#if 0
//解析ID3V1 
//buf:输入数据缓存区(大小固定是128字节)
//pctrl:MP3控制器
//返回值:0,获取正常
//    其他,获取失败
u8 mp3_id3v1_decode(u8* buf,__mp3ctrl *pctrl)
{
	ID3V1_Tag *id3v1tag;
	id3v1tag=(ID3V1_Tag*)buf;
	if (strncmp("TAG",(char*)id3v1tag->id,3)==0)//是MP3 ID3V1 TAG
	{
		if(id3v1tag->title[0])strncpy((char*)pctrl->title,(char*)id3v1tag->title,30);
		if(id3v1tag->artist[0])strncpy((char*)pctrl->artist,(char*)id3v1tag->artist,30); 
	}else return 1;
	return 0;
}
//解析ID3V2 
//buf:输入数据缓存区
//size:数据大小
//pctrl:MP3控制器
//返回值:0,获取正常
//    其他,获取失败
u8 mp3_id3v2_decode(u8* buf,u32 size,__mp3ctrl *pctrl)
{
	ID3V2_TagHead *taghead;
	ID3V23_FrameHead *framehead; 
	u32 t;
	u32 tagsize;	//tag大小
	u32 frame_size;	//帧大小 
	taghead=(ID3V2_TagHead*)buf; 
	if(strncmp("ID3",(const char*)taghead->id,3)==0)//存在ID3?
	{
		tagsize=((u32)taghead->size[0]<<21)|((u32)taghead->size[1]<<14)|((u16)taghead->size[2]<<7)|taghead->size[3];//得到tag 大小
		pctrl->datastart=tagsize;		//得到mp3数据开始的偏移量
		if(tagsize>size)tagsize=size;	//tagsize大于输入bufsize的时候,只处理输入size大小的数据
		if(taghead->mversion<3)
		{
			printf("not supported mversion!\r\n");
			return 1;
		}
		t=10;
		while(t<tagsize)
		{
			framehead=(ID3V23_FrameHead*)(buf+t);
			frame_size=((u32)framehead->size[0]<<24)|((u32)framehead->size[1]<<16)|((u32)framehead->size[2]<<8)|framehead->size[3];//得到帧大小
 			if (strncmp("TT2",(char*)framehead->id,3)==0||strncmp("TIT2",(char*)framehead->id,4)==0)//找到歌曲标题帧,不支持unicode格式!!
			{
				strncpy((char*)pctrl->title,(char*)(buf+t+sizeof(ID3V23_FrameHead)+1),AUDIO_MIN(frame_size-1,MP3_TITSIZE_MAX-1));
			}
 			if (strncmp("TP1",(char*)framehead->id,3)==0||strncmp("TPE1",(char*)framehead->id,4)==0)//找到歌曲艺术家帧
			{
				strncpy((char*)pctrl->artist,(char*)(buf+t+sizeof(ID3V23_FrameHead)+1),AUDIO_MIN(frame_size-1,MP3_ARTSIZE_MAX-1));
			}
			t+=frame_size+sizeof(ID3V23_FrameHead);
		} 
	}else pctrl->datastart=0;//不存在ID3,mp3数据是从0开始
	return 0;
} 

//得到当前播放时间
//fx:文件指针
//mp3x:mp3播放控制器
void mp3_get_curtime(FIL*fx,__mp3ctrl *mp3x)
{
	u32 fpos=0;
	if(fx->fptr>mp3x->datastart)fpos=fx->fptr-mp3x->datastart;	//得到当前文件播放到的地方
	mp3x->cursec=fpos*mp3x->totsec/(fx->fsize-mp3x->datastart);	//当前播放到第多少秒
	printf("  cursec:%d\r\n",mp3ctrl->cursec);
}

//获取MP3基本信息
//pname:MP3文件路径
//pctrl:MP3控制信息结构体 
//返回值:0,成功
//    其他,失败
u8 mp3_get_info(u8 *pname,__mp3ctrl* pctrl)
{
    HMP3Decoder decoder;
    MP3FrameInfo frame_info;
	MP3_FrameXing* fxing;
	MP3_FrameVBRI* fvbri;
	FIL*fmp3;
	u8 *buf;
	u32 br;
	u8 res;
	int offset=0;
	u32 p;
	short samples_per_frame;	//一帧的采样个数
	u32 totframes;				//总帧数
	
	fmp3 = malloc(sizeof(FIL)); 
	buf = malloc(5*1024);		//申请5K内存 
	if(fmp3&&buf)//内存申请成功
	{ 		
		f_open(fmp3,(const XCHAR*)pname,FA_READ);//打开文件
		res=f_read(fmp3,(char*)buf,5*1024,&br);
		if(res==0)//读取文件成功,开始解析ID3V2/ID3V1以及获取MP3信息
		{  
			mp3_id3v2_decode(buf,br,pctrl);	//解析ID3V2数据
			f_lseek(fmp3,fmp3->fsize-128);	//偏移到倒数128的位置
			f_read(fmp3,(char*)buf,128,&br);//读取128字节
			mp3_id3v1_decode(buf,pctrl);	//解析ID3V1数据  
			decoder=MP3InitDecoder(); 		//MP3解码申请内存
			f_lseek(fmp3,pctrl->datastart);	//偏移到数据开始的地方
			f_read(fmp3,(char*)buf,5*1024,&br);	//读取5K字节mp3数据
 			offset=MP3FindSyncWord(buf,br);	//查找帧同步信息
			if(offset>=0&&MP3GetNextFrameInfo(decoder,&frame_info,&buf[offset])==0)//找到帧同步信息了,且下一阵信息获取正常	
			{ 
				p=offset+4+32;
				fvbri=(MP3_FrameVBRI*)(buf+p);
				if(strncmp("VBRI",(char*)fvbri->id,4)==0)//存在VBRI帧(VBR格式)
				{
					if (frame_info.version==MPEG1)samples_per_frame=1152;//MPEG1,layer3每帧采样数等于1152
					else samples_per_frame=576;//MPEG2/MPEG2.5,layer3每帧采样数等于576 
 					totframes=((u32)fvbri->frames[0]<<24)|((u32)fvbri->frames[1]<<16)|((u16)fvbri->frames[2]<<8)|fvbri->frames[3];//得到总帧数
					pctrl->totsec=totframes*samples_per_frame/frame_info.samprate;//得到文件总长度
				}else	//不是VBRI帧,尝试是不是Xing帧(VBR格式)
				{  
					if (frame_info.version==MPEG1)	//MPEG1 
					{
						p=frame_info.nChans==2?32:17;
						samples_per_frame = 1152;	//MPEG1,layer3每帧采样数等于1152
					}else
					{
						p=frame_info.nChans==2?17:9;
						samples_per_frame=576;		//MPEG2/MPEG2.5,layer3每帧采样数等于576
					}
					p+=offset+4;
					fxing=(MP3_FrameXing*)(buf+p);
					if(strncmp("Xing",(char*)fxing->id,4)==0||strncmp("Info",(char*)fxing->id,4)==0)//是Xng帧
					{
						if(fxing->flags[3]&0X01)//存在总frame字段
						{
							totframes=((u32)fxing->frames[0]<<24)|((u32)fxing->frames[1]<<16)|((u16)fxing->frames[2]<<8)|fxing->frames[3];//得到总帧数
							pctrl->totsec=totframes*samples_per_frame/frame_info.samprate;//得到文件总长度
						}else	//不存在总frames字段
						{
							pctrl->totsec=fmp3->fsize/(frame_info.bitrate/8);
						} 
					}else 		//CBR格式,直接计算总播放时间
					{
						pctrl->totsec=fmp3->fsize/(frame_info.bitrate/8);
					}
				} 
				pctrl->bitrate=frame_info.bitrate;			//得到当前帧的码率
				mp3ctrl->samplerate=frame_info.samprate; 	//得到采样率. 
				if(frame_info.nChans==2)mp3ctrl->outsamples=frame_info.outputSamps; //输出PCM数据量大小 
				else mp3ctrl->outsamples=frame_info.outputSamps*2; //输出PCM数据量大小,对于单声道MP3,直接*2,补齐为双声道输出
			}else res=0XFE;//未找到同步帧	
			MP3FreeDecoder(decoder);//释放内存		
		} 
		f_close(fmp3);
	}else res=0XFF;
	free(fmp3);
	free(buf);	
	return res;	
}
#endif
/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
