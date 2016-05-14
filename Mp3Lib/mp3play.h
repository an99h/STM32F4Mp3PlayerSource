/**
  ******************************************************************************
  * @file    Audio_playback_and_record/inc/Mp3player.h 
  * @author  MCD Application Team
  * @version V1.0.0
  * @date    28-October-2011
  * @brief   Header for Mp3player.c module
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
  
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MP3PLAY_H
#define __MP3PLAY_H

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "key.h"
/* Exported types ------------------------------------------------------------*/




typedef enum
{
  LittleEndian,
  BigEndian
}Endianness;

typedef struct
{
  uint32_t  RIFFchunksize;
  uint16_t  FormatTag;
  uint16_t  NumChannels;
  uint32_t  SampleRate;
  uint32_t  ByteRate;
  uint16_t  BlockAlign;
  uint16_t  BitsPerSample;
  uint32_t  DataSize;
}
Mp3_FormatTypeDef;

typedef enum
{
  Valid_Mp3_File = 0,
  Unvalid_RIFF_ID,
  Unvalid_Mp3_Format,
  Unvalid_FormatChunk_ID,
  Unsupporetd_FormatTag,
  Unsupporetd_Number_Of_Channel,
  Unsupporetd_Sample_Rate,
  Unsupporetd_Bits_Per_Sample,
  Unvalid_DataChunk_ID,
  Unsupporetd_ExtraFormatBytes,
  Unvalid_FactChunk_ID
} ErrorCode;
#if 0
#define MP3_TITSIZE_MAX		40		//歌曲名字最大长度
#define MP3_ARTSIZE_MAX		40		//歌曲名字最大长度
#define MP3_FILE_BUF_SZ    5*1024	//MP3解码时,文件buf大小
 
//ID3V1 标签 
typedef __packed struct 
{
    u8 id[3];		   	//ID,TAG三个字母
    u8 title[30];		//歌曲名字
    u8 artist[30];		//艺术家名字
	u8 year[4];			//年代
	u8 comment[30];		//备注
	u8 genre;			//流派 
}ID3V1_Tag;

//ID3V2 标签头 
typedef __packed struct 
{
    u8 id[3];		   	//ID
    u8 mversion;		//主版本号
    u8 sversion;		//子版本号
    u8 flags;			//标签头标志
    u8 size[4];			//标签信息大小(不包含标签头10字节).所以,标签大小=size+10.
}ID3V2_TagHead;

//ID3V2.3 版本帧头
typedef __packed struct 
{
    u8 id[4];		   	//帧ID
    u8 size[4];			//帧大小
    u16 flags;			//帧标志
}ID3V23_FrameHead;

//MP3 Xing帧信息(没有全部列出来,仅列出有用的部分)
typedef __packed struct 
{
    u8 id[4];		   	//帧ID,为Xing/Info
    u8 flags[4];		//存放标志
    u8 frames[4];		//总帧数
	u8 fsize[4];		//文件总大小(不包含ID3)
}MP3_FrameXing;
 
//MP3 VBRI帧信息(没有全部列出来,仅列出有用的部分)
typedef __packed struct 
{
    u8 id[4];		   	//帧ID,为Xing/Info
	u8 version[2];		//版本号
	u8 delay[2];		//延迟
	u8 quality[2];		//音频质量,0~100,越大质量越好
	u8 fsize[4];		//文件总大小
	u8 frames[4];		//文件总帧数 
}MP3_FrameVBRI;


//MP3控制结构体
typedef __packed struct 
{
    u8 title[MP3_TITSIZE_MAX];	//歌曲名字
    u8 artist[MP3_ARTSIZE_MAX];	//艺术家名字
    u32 totsec ;				//整首歌时长,单位:秒
    u32 cursec ;				//当前播放时长
	
    u32 bitrate;	   			//比特率
	u32 samplerate;				//采样率
	u16 outsamples;				//PCM输出数据量大小(以16位为单位),单声道MP3,则等于实际输出*2(方便DAC输出)
	
	u32 datastart;				//数据帧开始的位置(在文件里面的偏移)
}__mp3ctrl;

extern __mp3ctrl * mp3ctrl;
#endif

/* Exported constants --------------------------------------------------------*/
#define  CHUNK_ID                            0x52494646  /* correspond to the letters 'RIFF' */
#define  FILE_FORMAT                         0x57415645  /* correspond to the letters 'Mp3' */
#define  FORMAT_ID                           0x666D7420  /* correspond to the letters 'fmt ' */
#define  DATA_ID                             0x64617461  /* correspond to the letters 'data' */
#define  FACT_ID                             0x66616374  /* correspond to the letters 'fact' */
#define  Mp3_FORMAT_PCM                     0x01
#define  FORMAT_CHNUK_SIZE                   0x10
#define  CHANNEL_MONO                        0x01
#define  CHANNEL_STEREO                      0x02
#define  SAMPLE_RATE_8000                    8000
#define  SAMPLE_RATE_11025                   11025
#define  SAMPLE_RATE_22050                   22050
#define  SAMPLE_RATE_44100                   44100
#define  BITS_PER_SAMPLE_8                   8
#define  BITS_PER_SAMPLE_16                  16

/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
void TimingDelay_Decrement(void);
void Delay(__IO uint32_t nTime);
void Mp3PlayBack(uint32_t AudioFreq);
uint32_t AudioFlashPlay(uint16_t* pBuffer, uint32_t FullSize, uint32_t StartAdd);
int Mp3PlayerInit(uint32_t AudioFreq);
void Mp3PlayerStop(void);
void Mp3PlayerPauseResume(uint8_t state);
uint8_t Mp3playerCtrlVolume(uint8_t volume);
void Mp3PlayerStart(u8 *path);
void Mp3Player_CallBack(void);
uint32_t ReadUnit(uint8_t *buffer, uint8_t idx, uint8_t NbrOfBytes, Endianness BytesFormat);
u8 get_file_curindex(void);
u8 get_volume(void);
u8 music_ctrl(KEY_PRESS key,u8 *curindex,u8 music_sum);
char * get_music_time(void);

#endif /* __Mp3_PLAYER_H */

/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
