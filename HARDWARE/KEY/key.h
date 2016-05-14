#ifndef __KEY_H
#define __KEY_H	 
#include "sys.h"  
	
/*下面方式是通过位带操作方式读取IO*/
 
#define MUSIC_MODE_KEY 		PAin(0)		//音乐播放模式
#define PAGE_DOWN_KEY		PDin(0)		//下一页
#define NEXT_SONG_KEY		PDin(1)		//下一曲
#define VOL_UP_KEY			PDin(2)		//音量-
#define PAUSE_SONG_KEY		PDin(3)		//暂停
#define VOL_DOWN_KEY		PBin(3)		//音量+
#define STOP_SONG_KEY		PDin(5)		//停止
#define PAGE_UP_KEY			PDin(6)		//上一页
#define LAST_SONG_KEY		PDin(7)		//上一曲


#if 1
typedef enum _KEY_PRESS
{
	NO_KEY_PRESS = 0,
	MUSIC_MODE,		//音乐播放模式
	PAGE_DOWN,		//下一页
	NEXT_SONG,		//下一曲
	VOL_UP,			//音量+
	PAUSE_SONG,		//暂停
	VOL_DOWN,		//音量-
	STOP_SONG,		//停止
	PAGE_UP,		//上一页
	LAST_SONG		//上一曲
}KEY_PRESS;
#endif

void KEY_Init(void);			//IO初始化
KEY_PRESS KEY_Scan(u8 mode);	//按键扫描函数

#endif
