#ifndef __KEY_H
#define __KEY_H	 
#include "sys.h"  
	
/*���淽ʽ��ͨ��λ��������ʽ��ȡIO*/
 
#define MUSIC_MODE_KEY 		PAin(0)		//���ֲ���ģʽ
#define PAGE_DOWN_KEY		PDin(0)		//��һҳ
#define NEXT_SONG_KEY		PDin(1)		//��һ��
#define VOL_UP_KEY			PDin(2)		//����-
#define PAUSE_SONG_KEY		PDin(3)		//��ͣ
#define VOL_DOWN_KEY		PBin(3)		//����+
#define STOP_SONG_KEY		PDin(5)		//ֹͣ
#define PAGE_UP_KEY			PDin(6)		//��һҳ
#define LAST_SONG_KEY		PDin(7)		//��һ��


#if 1
typedef enum _KEY_PRESS
{
	NO_KEY_PRESS = 0,
	MUSIC_MODE,		//���ֲ���ģʽ
	PAGE_DOWN,		//��һҳ
	NEXT_SONG,		//��һ��
	VOL_UP,			//����+
	PAUSE_SONG,		//��ͣ
	VOL_DOWN,		//����-
	STOP_SONG,		//ֹͣ
	PAGE_UP,		//��һҳ
	LAST_SONG		//��һ��
}KEY_PRESS;
#endif

void KEY_Init(void);			//IO��ʼ��
KEY_PRESS KEY_Scan(u8 mode);	//����ɨ�躯��

#endif
