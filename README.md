# STM32F4Mp3PlayerSource
My graduation design
##STM32F07VGT6硬件连接图


###					ILI9341	8位数据线		2.4寸LCD屏幕引脚配置
					数据线：PE0 --------->LCD_D0							
						 	PE1 --------->LCD_D1	
							PE2 --------->LCD_D2	
							PE3 --------->LCD_D3	
							PE4 --------->LCD_D4
							PE5 --------->LCD_D5
							PE6 --------->LCD_D6
							PE7 --------->LCD_D7
					控制线：PB0 --------->LCD_CS
							PB1 --------->LCD_RS
							PB12--------->LCD_WR		
							PB13--------->LCD_WD		
							PB14--------->LCD_REST	


###					TF卡SPI1引脚配置									
							PA5--------->SD_CLK						
						 	PA6--------->SD_MISO						
							PA7--------->SD_MOSI							
							PC5--------->SD_CS								
																			
																			
																				
##					USB-OTG引脚配置											
							PA9 --------->OTG_FS_VBUS						
							PA10--------->OTG_FS_ID							
							PA11--------->OTG_FS_DM				
																			
																	
###					音频解码芯片CS43L22									
					I2C配置	PD4 --------->REST							
							PB6 --------->SCL								
							PB9 --------->SDA								
					I2S配置	PA4 --------->WS						
							PC10--------->SCK								
							PC12--------->SD							
							PC7 --------->MCK								
																		
																	
					USART3串口引脚配置							
							PB10--------->USART3_TX					
							PB11--------->USART3_RX						
																			
																			
##3					按键引脚配置										
							PA0--------->KEY0_PRESS					
							PD0--------->PAGE_DOWN				
							PD1--------->NEXT_SONG				
							PD2--------->VOL_UP					
							PD3--------->PAUSE_SONG					
							PB3--------->VOL_DOWN					
							PD5--------->STOP_SONG						
							PD6--------->PAGE_UP					
							PD7--------->LAST_SONG					
																		
								
