/******************************************************************************
 Copyright (C) 2011      Advanced Digital Chips Inc. 
						http://www.adc.co.kr
 Author : Software Team.

******************************************************************************/
#include "adstar.h"
#include <string.h>
#include <stdio.h>
#include <typedef.h>
#include "Mydefine.h"
#include "Abstract.h"
#include "State.h"
#include "Window.h"

#define sbit(a,n) (a |= (1<<n))    //«ÿ¥Á«œ¥¬   bit¢¨¢¨ --> "1"
#define cbit(a,n) (a &= ~(1<<n))   //«ÿ¥Á«œ¥¬   bit¢¨¢¨ --> "0"
#define ibit(a,n) (a ^= (1<<n))    //«ÿ¥Á«œ¥¬  bit¢¨| invert
#define tbit(a,n) (a & (1<<n))   //«ÿ¥Á«œ¥¬  bit »Æ¿Œ

extern unsigned char gabDC1307_rbuf[RTC_RAM_SIZE];
extern unsigned char gabDC1307_wbuf[RTC_RAM_SIZE];
extern void TWIISR();

extern DSPM uint32_t Object_cnt; //Count object in a window
extern DSPM pObject ArrayofpObject[MAX_OBJECT];
extern DSPM Object ArrayofObject[MAX_OBJECT];

extern BOOL savebmp( char* savefname, SURFACE* src );
extern BOOL mysavebmp( char* savefname, SURFACE* src );

uint16_t test_mode_timer = 0;
uint8_t draw_cnt=0; //for blinking

void UpdateSetUpTime();
static void Buzzer_Ctrl(void);

uint32_t Buzzer_period;
uint32_t Buzzer_cnt;
uint32_t Buzzer_state;
//static uint32_t Buzzer_set;


void Set_Buzzer(uint32_t cnt, uint32_t period)
{
	if(Buzzer_state == 0 && cnt != 0)
	{
		BUZZER_TIMER_1ms = period;
		Buzzer_period = period;
		*(volatile U32*)0x80022c00 = 1<<7 | 0x03;
		*(volatile U32*)0x80022c10 |= 1<<8;    //pwm independent output 0 ch
		//*(volatile U32*)0x80022c08 = 2559;    //pwm period  // 16bit length
		*(volatile U32*)0x80022c08 = 1708;    //pwm period  // 16bit length
		*(volatile U32*)0x80022c2c = 1400;    //pwm duty 0
		Buzzer_state = 1;
		Buzzer_cnt = cnt;
		//debugprintf("set buzzer cnt %d period %d\r\n",cnt,period);
	}
	else if(cnt == 0 && period == 0)
	{
		Buzzer_cnt = 0;
		Buzzer_state = 0;
		*(volatile U32*)0x80022c00 = 1<<7;
		*(volatile U32*)0x80022c10 &= ~(1<<8);
	}
}

static void test()
{
	//uint8_t tmp[1024*(128+4)];
	uint8_t tmp[64][2048];
	memset(tmp,0xff,sizeof(tmp));
	uint32_t i,j,k;
	

	//for(i=0;i<1024;i++)
	/*
	for(i=0;i<1;i++)
	{
		debugprintf("block number is %d\r\n", i);
		nand_readblock(i, (uint8_t*)tmp);
		for(j=0;j<sizeof(tmp)/sizeof(tmp[j]);j++)
		{
			for(k=0;k<sizeof(tmp[j]);k++)
			{
				if( tmp[j][k] != 0xff)
				{
					debugprintf("block %d pagenumber is %d index %x value is %x\r\n",i,j,k,tmp[j][k]);
				}	
			}
		}
		debugprintf("\r\n done \r\n");
		
		loopdelayms(10240);
	}	
	*/
	
	uint8_t tmp2[2048];
	memset(tmp2,0xff,sizeof(tmp2));
	nand_writepage(0,tmp2);
	nand_readpage(0,tmp2);
	for(i=0;i<sizeof(tmp2);i++)
	{
		if(tmp2[i] != 0xff)	
		{
			debugprintf("%d value %x\r\n",i,tmp2[i]);
		}
	}
	nand_eraseblock(0);
}

/*

void Buzzer()
{
	if(Buzzer_cnt !=0)
	{
		if(Buzzer_state ==0)
		{
			*(volatile U32*)0x80022c00 = 1<<7 | 0x03;
			*(volatile U32*)0x80022c10 |= 1<<8;    //pwm independent output 0 ch
			*(volatile U32*)0x80022c08 = 2400;    //pwm period  // 16bit length
			*(volatile U32*)0x80022c2c = 1400;    //pwm duty 0
			Buzzer_state = 1;
			debugprintf("buzzer beep\r\n");
		}
		else if(Buzzer_state ==1)
		{
			*(volatile U32*)0x80022c00 = 1<<7;
			*(volatile U32*)0x80022c10 &= ~(1<<8);			
			Buzzer_state = 0;
			Buzzer_cnt -= 1;
		}
	}
	else 
	{
		pAlarm_Object[0]->IsPressed = 0;		
	}
}

void Set_Buzzer(uint32_t cnt, uint32_t set)
{
	if( ( cnt==0 ) && (set == 0))
	{
		Buzzer_cnt = 0;
		Buzzer_state = 0;
		Buzzer_set = 0;
		debugprintf("buzzer init\r\n");
	}
	else if( (cnt != 0) && (set == 1) && (Buzzer_set == 1)) //already set
	{
		
	}
	
	else if( (cnt != 0) && (set == 1) && (Buzzer_set == 0))
	{
		Buzzer_cnt = cnt;
		Buzzer_set = set;
		//debugprintf("buzzer set %d %d\r\n",cnt,set);
	}
	else if( (cnt != 0) && (set == 0) && (Buzzer_set == 0))
	{
		Buzzer_cnt = cnt;
		Buzzer_set = set;
		//debugprintf("buzzer set %d %d\r\n",cnt,set);
	}
	

	
	
	
	*(volatile U32*)0x80022c08 = 3023;
	loopdelayms(1000);
	*(volatile U32*)0x80022c08 = 2693;
	loopdelayms(1000);
	*(volatile U32*)0x80022c08 = 2400;
	loopdelayms(1000);
	*(volatile U32*)0x80022c08 = 2265;
	loopdelayms(1000);
	*(volatile U32*)0x80022c08 = 2018;
	loopdelayms(1000);
	*(volatile U32*)0x80022c08 = 1798;
	loopdelayms(1000);
	*(volatile U32*)0x80022c08 = 1601;
	loopdelayms(1000);
	*(volatile U32*)0x80022c08 = 1511;
	*(volatile U32*)0x80022c2c = 200;
	loopdelayms(1000);
	
}
*/
	

/*=========================================================================
#define LCD_BL_OFF		(*R_GPOLOW(3) = 0x20)		// P3.5
#define LCD_BL_ON			(*R_GPOHIGH(3) = 0x20)		// P3.5
=========================================================================*/
	
uint8_t islcdoff()
{
	uint8_t ret = 0;
	uint32_t duty = (*R_TM2DUT)&0xFFFF;
	if(duty == 8)
	{
		ret = 1;
	}
	return ret;
}
	
void lcdwakeup(uint8_t buzzer)
{
	if( LCD_TIMER_1ms == 60000) return;

	LCD_TIMER_1ms = 60000;
	
	//debugprintf("R_TM2DUT is %d\r\n", (*R_TM2DUT));
	
	if ( *R_TM2DUT == 2)
	{
		*R_TM2DUT = 7;
		if(buzzer != 0)
		{
			Set_Buzzer(1,100);
		}
	}
}


void lcdon(U8 step)
{	
	*R_PAF3 = (3<<14)|(3<<12)|(2<<10)|(3<<8)|(2<<6)|(2<<4)|(3<<2)|(1<<0);
	*R_TM2CTRL = 1 << 14 | 0 << 6 | 1 << 5 | 1 << 4 | 7 << 1 | 1 << 0;
	*R_TM2CNT =  8;
	
	if(step == 0x00)
	{
		//*R_TM2CTRL = 1 << 14 | 0 << 6 | 1 << 5 | 1 << 4 | 7 << 1 | 0 << 0;
		*R_TM2DUT = 8;
		
		//LCD_BL_OFF;
		//*R_GPOLOW(3) = 0x20;
	}
	else if(step == 0xFF)			
	{			
		*R_TM2DUT = 7;
		//LCD_BL_ON;	
		//*R_GPOHIGH(3) = 0x20;
	}
	else 
	{
		if(*R_TM2DUT != 8) // LCD_OFF CHECK
		{
			*R_TM2DUT = step;
		}
	}
}

static void Buzzer_Ctrl(void) 
{
	if(Buzzer_state == 1)
	{
		BUZZER_TIMER_1ms = Buzzer_period;
		Buzzer_state = 0;
		*(volatile U32*)0x80022c00 = 1<<7;
		*(volatile U32*)0x80022c10 &= ~(1<<8);
	}
	else
	{
		if(Buzzer_cnt <= 1)
		{
			Buzzer_cnt = 0;
		}
		else
		{
			BUZZER_TIMER_1ms = Buzzer_period;
			Buzzer_state = 1;
			Buzzer_cnt -= 1;
			*(volatile U32*)0x80022c00 = 1<<7 | 0x03;
			*(volatile U32*)0x80022c10 |= 1<<8;    //pwm independent output 0 ch
			//*(volatile U32*)0x80022c08 = 2559;    //pwm period  // 16bit length
			*(volatile U32*)0x80022c08 = 1708;    //pwm period  // 16bit length
			*(volatile U32*)0x80022c2c = 1400;    //pwm duty 0
		}
	}
}


 /*=========================================================================
// 1ms Timer
=========================================================================*/
static void TIMER0ISR(void)
{
	Timer_1S++;
	if(Timer_1S == 1000)
	{
		Timer_1S = 0;
		
		struct tm * tmp_time_tm;
		time_t tmp_time = mktime(&gstRTC_time_t);
		tmp_time++;
		tmp_time_tm = localtime(&tmp_time);
		memcpy(&gstRTC_time_t,tmp_time_tm,sizeof(struct tm));
		State_Data.Current_Time = tmp_time;
		
		if(time_last != 1560157200)
		{
			time_last++;
		}
	}
	systick++;
			
	if( RTC_TIMER_1ms != 0) 		RTC_TIMER_1ms--;
	if( RTC_DATA_1ms != 0) 		RTC_DATA_1ms--;
	
	if(BUZZER_TIMER_1ms != 0)
	{
		BUZZER_TIMER_1ms--;
	}
	else
	{
		Buzzer_Ctrl();
	}
	
	if(UART3_WAIT_TIMER_1ms != 0)
	{
		UART3_WAIT_TIMER_1ms--;
	}
	else
	{
		if(checktxringbuf(3) == 0)
		{
			if(UART_485_State.state == TX_ing)
			{
				uart_tx_int_enable(3,0);
				(*R_GPOHIGH(4) = 0x08); //rx mode
				uart_rx_int_enable(3,1);
				UART_485_State.state = Rx_waiting;
			}
		}
		//debugprintf("uart 485 rx mode on\r\n");
	}
}

static void TIMER1ISR(void)
{
	if(UART_485_State.state == Rx_waiting) //RX
	{
		if(UART3_RX_TIMER_1ms != 0)
		{
			UART3_RX_TIMER_1ms --;
			
			uint32_t rx_ringbuf_cnt = get_rx_ringbuf_cnt(3);

			if( rx_ringbuf_cnt > 2)
			{
				uint32_t i;
				uint32_t stx_index = UART_BUF_SIZE;
				uint32_t length = UART_BUF_SIZE;
				uint8_t etx_check = 0;
				for(i=0;i<rx_ringbuf_cnt;i++)
				{
					if( getrxringbuf_ch(3,i) == 0xaa)
					{
						stx_index = i;
						break;
					}
				}
				
				if(stx_index < UART_BUF_SIZE)
				{
					length = getrxringbuf_ch(3,stx_index+1);
				}
				
				if( length < UART_BUF_SIZE)
				{
					if( rx_ringbuf_cnt >= stx_index+length+3)
					{
						etx_check = getrxringbuf_ch(3, stx_index+length+3);
					}
				}
				
				if(etx_check == 0x55)
				{
					UART3_RX_TIMER_1ms = 0;
				}
				/*
				else
				{ 
					if(ringbuf_check == 0)
					{
						uint16_t i;
						for(i=0;i<length+10;i++)
						{
							debugprintf("%x ",getrxringbuf_ch(3, i));
						}
						for(i=0;i<UART_BUF_SIZE;i++)
						{
							//debugprintf("%x ", *(get_rx_ringbuf(3)+i));
						}
						debugprintf("length is %d\r\n",length);
						debugprintf("etx not founded %x\r\n",etx_check);
						ringbuf_check = 1;
					}
				}
				*/
			}
		}
		else
		{
			//StartEstimate();
			UART_485_State.state = Rx_ing;
			UART3_TRANS_TIMER_1ms = 5;
		}
	}
	else if(UART_485_State.state == Rx_ing)
	{
		if(UART3_TRANS_TIMER_1ms != 0)
		{
			UART3_TRANS_TIMER_1ms--;
		}
		else
		{
			uint32_t check_ret = UART3_RECEIVE_check();
			if(check_ret == 0)
			{
				UART3_STATE_check();			
				UART_485_State.state = 0;
				UART_485_State.state = Tx_waiting;
				UART3_TX_TIMER_1ms = 100;
			}
			else
			{
				UART3_STATE_check();			
				UART_485_State.state = 0;
				UART_485_State.state = Tx_waiting;
				UART3_TX_TIMER_1ms = 100;
			}
			uart_rx_int_enable(3,0);
			(*R_GPOLOW(4) = 0x08); //tx mode
			uart_tx_int_enable(3,1);
		}
	}
	else if(UART_485_State.state == Tx_waiting)
	{
		if(UART3_TX_TIMER_1ms != 0)
		{
			UART3_TX_TIMER_1ms--;
		}
		else
		{
			//EndEstimate("Time ");
			UART3_RX_TIMER_1ms = UART3_SEND_dequeue();
			UART_485_State.state = TX_ing;
		}
	}
		
	if( KEY_TIMER_1ms != 0)
	{
		KEY_TIMER_1ms--;
	}
	else
	{
		Key_Check();
		KEY_TIMER_1ms = 5;
	}
	
	if(TWI_TIMER_1ms != 0)
	{
		TWI_TIMER_1ms--;
	}
	else
	{
		twi_process();
	}
	
	if(TOUCH_TIMER_1ms != 0)  
	{
		TOUCH_TIMER_1ms--;
	}
	else
	{
		if(Key.Power == 0)
		{
			if(GUI_inf.IsInterrupted == 1)
			{
				GUI_inf.IsInterrupted = 0;
				TOUCH_TIMER_1ms = 30;
				TOUCH_get_coordinate();
			}
			
		}
		
	}
		
	
	
	/*
	//get touch coordinate
	if(TOUCH_TIMER_1ms != 0)  
	{
		TOUCH_TIMER_1ms--;
	}
	else
	{
		if(Key.Power == 0)
		{
			if(GUI_inf.IsInterrupted == 1 && TWI_IsUsing == 0)
			//if(GUI_inf.IsInterrupted == 1 && UART_485_State.state == Tx_waiting)
			{
				enable_interrupt(INTNUM_EIRQ0,FALSE);
				TWI_IsUsing = 1;
				TOUCH_TIMER_1ms = 50;
				I2C_PROCESS();
				TWI_IsUsing = 0;
				enable_interrupt(INTNUM_EIRQ0,TRUE);
			}
		}
	}
	*/
	
	//process touch coordinate
	
	
	
	if(FLASH_TIMER_1ms	!= 0)	FLASH_TIMER_1ms--;
	if(LCD_TIMER_1ms	!= 0)	LCD_TIMER_1ms--;
	if(POPUP_ERR_TIMER_1ms != 0) POPUP_ERR_TIMER_1ms--;
	if(ERROR_CHECK_TIMER_1ms !=0)
	{
		ERROR_CHECK_TIMER_1ms --;
	}
}




/*=========================================================================

	
=========================================================================*/
void Drow_Logo_Page(void)
{
	set_draw_target(getfrontframe());		

	//ICON_TEMP[0] = loadbmp("image/logo.bmp");
	ICON_TEMP[0] = loadbmp("image/logo_dither.bmp");
	draw_surface(ICON_TEMP[0], 0, 0);	
	release_surface(ICON_TEMP[0]);
	ICON_TEMP[0] = 0;
		
	debugstring("SYSTEM LOGO ON \r\n");		
	
}

	



/*=========================================================================

	
=========================================================================*/
int main()
{	
	//memset( &gstRTC_time, 0, sizeof( DS1307_Time ) );

	PortInit();	
	lcdon(0x00); // need change to pwm control

	uart_config(0,115200,DATABITS_8,STOPBITS_1,UART_PARNONE);		

	
	debugstring("\r\n================================================\r\n");	
	debugstring("ADSTAR Boot  Ver 0.1 test>>>>\r\n");
	debugstring("================================================\r\n");	

	crtc_clock_init();		
		
		
	PRINTVAR(get_pll(0));
	PRINTVAR(get_pll(1));
	
	StartEstimate();

	debugprintf("First init, size of free memory is %d\r\n", get_cur_sp() - get_cur_heapend());
	/* Main fream ÏÉùÏÑ± */
	frame = createframe(800,480,16);
	frame2 = createframe(800,480,16);	
	
	//frame = createframe(800,480,32);
	//frame2 = createframe(800,480,32);	
	
	setscreen(SCREEN_KD50G21,SCREENMODE_RGB565);

	

	/*
	Horizontal total: *R_CRTHT = fornt porch+sync+back porch+active

	Horizontal sync start: *R_CRTHS[26:16]=front porch

	Horizontal sync end: *R_CRTHS[10:0]=front porch+sync

	Horizontal active start: *R_CRTHA[26:16]=front porch+sync+back porch

	Horizontal active end: *R_CRTHA[10:0]=front porch+sync+back porch+active
	*/
	
	//ht hs ha vt vs va
	//setscreenex(800,480,SCREENMODE_RGB888, 1096, (210<<16) | 250 , (296<< 16) | 1096, 545, (22<<16) | 42, (65<<16) | 545); //fourth
	//setscreenex(800,480,SCREENMODE_RGB888, 1000, (134<<16) | 154 , (200<< 16) | 1000, 540, (27<<16) | 37, (60<<16) | 540); //third
	//setscreenex(800,480,SCREENMODE_RGB888, 1000, (154<<16) | 180 , (200<< 16) | 1000, 540, (14<<16) | 41, (60<<16) | 540); //second
	//setscreenex(800,480,SCREENMODE_RGB888, 880, (16<<16) | 62 , (80<< 16) | 880, 530, (27<<16) | 30, (50<<16) | 530); //first
	
	setdoubleframebuffer(frame,frame2);
	set_draw_target(frame);
	debugprintf("Screent init, size of free memory is %d\r\n", get_cur_sp() - get_cur_heapend());
	/* NAND Flash mount ÏÑ§Ï†ï */
	f_mount(DRIVE_NAND,&fs);		
	


	/* ÏòÅÎ¨∏ FONT ÏÑ§Ï†ï */
	//f_chdir("font");
	//g_pFont = create_bmpfont("nanum_8bit_20px_han.fnt");
	g_pFont = create_bmpfont("image/Font/My_font.fnt");
	bmpfont_setautokerning(g_pFont, true);
	bmpfont_settextencoding(g_pFont, NONE);
	g_pFont_remain = create_bmpfont("image/Font/My_font_remain.fnt");
	bmpfont_setautokerning(g_pFont_remain, true);
	bmpfont_settextencoding(g_pFont_remain, NONE);
	//egl_font_set_color(g_pFont, MAKE_COLORREF(0, 0, 255));		
	//f_chdir("..");	
	debugprintf("Font init, size of free memory is %d\r\n", get_cur_sp() - get_cur_heapend());
			
	/* Load Icon image */
	Drow_Logo_Page();	
	loopdelayms(20);
	lcdon(0xFF);
	LCD_TIMER_1ms = 60000;
	//egl_font_set_color(g_pFont, MAKE_COLORREF(0, 0, 255));
	debugprintf("After logo, size of free memory is %d\r\n", get_cur_sp() - get_cur_heapend());
	
	//nand flash test
	debugprintf("nand page size is %d\r\n", nand_get_pagesize());
	debugprintf("nand block size is %d\r\n", nand_get_blocksize());
	debugprintf("nand page per block is %d\r\n", nand_get_pageperblock());
	debugprintf("nand block count is %d\r\n", nand_get_blockcnt());
	debugprintf("nand sector count is %d\r\n", nand_get_sectorcount());
	debugprintf("nand sector size is %d\r\n", nand_get_sectorsize());
	//test();
	
	WindowInit();
	Init_State();
	/* Load Parameter */
	Load_Parameter();
	Error_Log_Read();
	Working_Log_Read();

	Reload_Auto_Images();

	(*R_GPOLOW(4) = 0x08); //tx mode
	//uart_config(3,9600,DATABITS_8,STOPBITS_1,UART_PARNONE);	
	uart_config(3,9620,DATABITS_8,STOPBITS_1,UART_PARNONE);	

	
	/* I2C Initialization */
	loopdelayms(5);
	twi_set_freq (400000);	
	while(twi_wait_busy() != TRUE);
	loopdelayms(5);
	
	/*buzzer off*/
	Set_Buzzer(0,0);
	
	*R_PAF4 = (3<<14)|(3<<12)|(1<<10)|(1<<8)|(3<<6)|(3<<4)|(3<<2)|(2<<0); // EIRQ0 ENABLE
	*R_GPIDIR(4) = 0x47;	
	*R_EINTMOD = F_EINTMOD_0MOD_REDGE;
	set_interrupt(INTNUM_EIRQ0,EIRQ0_CHECK);
	

	/* timer set */
	systick = 0;
	set_interrupt( INTNUM_TIMER0, TIMER0ISR ); //Timer 0 
	set_interrupt( INTNUM_TIMER1, TIMER1ISR);
	//Timer 2 uses for LCD PWM
	//Timer 3 uses for estimate time
	set_timer( 0, 1);//1ms	
	set_timer( 1, 1);
	enable_interrupt(INTNUM_TIMER0,TRUE);
	enable_interrupt(INTNUM_TIMER1,FALSE);
	
	*R_TWICTRL = F_TWICTRL_TWIEN;// TWI Disable
	*R_TWISTAT &= ~(0x20);// TWI Busy Clear
	*R_TWISTAT = F_TWISTAT_TWILOST; // TWI Lost Clear
	loopdelayms(100);
	
	set_interrupt(INTNUM_TWI,TWIISR);
	enable_interrupt(INTNUM_TWI,TRUE);
	
	enable_interrupt(INTNUM_TIMER1,TRUE);
	/*
	//I2C test
	uint8_t twi_tmp[64];
	
	uint8_t address_temp[2];	
	uint8_t i;
	for(i=0;i<8;i++)
	{
		twi_tmp[i]=i;
	}
	address_temp[0] = 0;
	twi_write_int(0xD0,address_temp,1,twi_tmp,8);
	
	loopdelayms(1000);
	
	
	qwer :
	memset(twi_tmp,0, 64);
	
	
	
	
	address_temp[0] = 0x00;
	twi_read_int(0xD0,address_temp,1,twi_tmp,64);
	
	//address_temp[0] = 0x81;
	//address_temp[1] = 0x40;
	//twi_read_int(0x28,address_temp,2,twi_tmp,11);
	
		
	while(1)
	{
		loopdelayms(30);
		uint32_t i;
		for(i=0;i<64;i++)
		{
			debugprintf("%x ",twi_tmp[i]);
		}
		debugprintf("\r\n");
		while(1);
		goto qwer;
	}
	// I2C test end
	*/
	
	
	/* Factory Reset */
	
	if((*R_GPILEV(0)&0x01) == 0 && (*R_GPILEV(0)&0x02) != 0 )			//SW4,SW5
	{
		debugprintf("Factory Reset\r\n");
		memset(gabDC1307_wbuf, 0 , RTC_RAM_SIZE);
		gabDC1307_wbuf[0] = 0;
		gabDC1307_wbuf[1] = 0;
		gabDC1307_wbuf[2] = 0x09;
		gabDC1307_wbuf[3] = 0;
		gabDC1307_wbuf[4] = 0x10;
		gabDC1307_wbuf[5] = 0x06;
		gabDC1307_wbuf[6] = 0x19;
		
		uint32_t i;
		
		for(i=7;i<RTC_SET_SIZE;i++)
		{
			gabDC1307_wbuf[i] = 0x0;
		}
		
		i = 8; //except time
		DS1307_Write(i,gabDC1307_wbuf,RTC_RAM_SIZE-i);
		Set_Buzzer(1,1000);
		twi_wait();
		
		
		DS1307_mode_write();
		twi_wait();
		
		Init_State();
		Log_Reset();
		UART3_SEND_enqueue( SEND_CMD_0x30, 5000);
		UART3_SEND_enqueue( SEND_CMD_0x30, 2000);
		UART3_SEND_enqueue( SEND_CMD_0x31, 2000);
		UART3_SEND_enqueue( SEND_CMD_0x32, 2000);
		UTIL.bits.ParameterWrite_bit = 1;
		Unload_Auto_Images();
		Reload_Auto_Images();
	}
	
	
	/* RTC Function */
	debugprintf("RTC set start\r\n");
	asdf :
	twi_wait();
	memset(gabDC1307_rbuf,0xff,sizeof(gabDC1307_rbuf));
	while(DS1307_init()!=0);
	twi_wait();
	if( (gabDC1307_rbuf[0]&0x80) || (gabDC1307_rbuf[7]&0x20) )
	{
		uint32_t i;
		//memcpy(gabDC1307_wbuf,gabDC1307_rbuf,8);
		//gabDC1307_wbuf[0] -= 0x80;
		//gabDC1307_wbuf[7] = 0;
		
		debugprintf("Halt bit found\r\n");
		
		memset(gabDC1307_wbuf, 0 , RTC_RAM_SIZE);
		gabDC1307_wbuf[0] = 0;
		gabDC1307_wbuf[1] = 0;
		gabDC1307_wbuf[2] = 0x09;
		gabDC1307_wbuf[3] = 0;
		gabDC1307_wbuf[4] = 0x10;
		gabDC1307_wbuf[5] = 0x06;
		gabDC1307_wbuf[6] = 0x19;
		
		for(i=7;i<RTC_SET_SIZE;i++)
		{
			gabDC1307_wbuf[i] = 0x0;
		}
		
		DS1307_Write(0,gabDC1307_wbuf,RTC_SET_SIZE);

		/*
		for(i=0; i<8;i++)
		{
			debugprintf("%x ",gabDC1307_wbuf[i]);
		}
		*/
		goto asdf;
	}
	
	
	debugprintf("get time start\r\n");
	twi_wait();
	if( DS1307_GetTime(&gstRTC_time_t) == -1)
	{
		debugprintf("get time failed\r\n");
		memset(gabDC1307_wbuf, 0 , RTC_RAM_SIZE);
		gabDC1307_wbuf[0] = 0;
		gabDC1307_wbuf[1] = 0;
		gabDC1307_wbuf[2] = 0x09;
		gabDC1307_wbuf[3] = 0;
		gabDC1307_wbuf[4] = 0x10;
		gabDC1307_wbuf[5] = 0x06;
		gabDC1307_wbuf[6] = 0x19;
		
		gabDC1307_wbuf[7] = 0x0;
		gabDC1307_wbuf[8] = 0x0;
		gabDC1307_wbuf[9] = 0x0;
		gabDC1307_wbuf[10] = 0x0;
		gabDC1307_wbuf[11] = 0x0;
		gabDC1307_wbuf[12] = 0x0;
		gabDC1307_wbuf[13] = 0x0;
		gabDC1307_wbuf[14] = 0x0;
		gabDC1307_wbuf[15] = 0x0;
		gabDC1307_wbuf[16] = 0x0;
		gabDC1307_wbuf[17] = 0x0;
		gabDC1307_wbuf[18] = 0x0;
		gabDC1307_wbuf[19] = 0x0;
		DS1307_Write(0,gabDC1307_wbuf,RTC_SET_SIZE);
		loopdelayms(RTC_SET_SIZE*5);
		DS1307_GetTime(&gstRTC_time_t) ;
		goto asdf;
	}
	twi_wait();
	if( gstRTC_time_t.tm_sec > 29 )
	{
		RTC_TIMER_1ms = (90-gstRTC_time_t.tm_sec)*1000;
	}
	else
	{
		RTC_TIMER_1ms = (30 - gstRTC_time_t.tm_sec) * 1000;
	}
	
	State_Data.Current.Year = gstRTC_time_t.tm_year-100;
	State_Data.Current.Month = gstRTC_time_t.tm_mon+1;
	State_Data.Current.Date = gstRTC_time_t.tm_mday;
	State_Data.Current.Hour = gstRTC_time_t.tm_hour;
	State_Data.Current.Min = gstRTC_time_t.tm_min;
	State_Data.Current.Sec = gstRTC_time_t.tm_sec;
	
	
	
	State_Data.Menu = gabDC1307_rbuf[RTC_SET_SIZE];
	State_Data.Mode_Op = gabDC1307_rbuf[RTC_SET_SIZE+1];
	State_Data.Start = gabDC1307_rbuf[RTC_SET_SIZE+2];
	State_Data.Lamp = gabDC1307_rbuf[RTC_SET_SIZE+3];
	State_Data.Working_Time_1h = gabDC1307_rbuf[RTC_SET_SIZE+4];
	State_Data.Working_Time_1m = gabDC1307_rbuf[RTC_SET_SIZE+5];
	State_Data.Operating_Remain_Time_1h = gabDC1307_rbuf[RTC_SET_SIZE+6];
	State_Data.Operating_Remain_Time_1m = gabDC1307_rbuf[RTC_SET_SIZE+7];
	State_Data.Operating_Remain_Time = State_Data.Operating_Remain_Time_1h*3600 + State_Data.Operating_Remain_Time_1m * 60;
	
	State_Data.Current_Time = gabDC1307_rbuf[RTC_SET_SIZE+8]<<24 | gabDC1307_rbuf[RTC_SET_SIZE+9]<<16 | gabDC1307_rbuf[RTC_SET_SIZE+10]<<8 | gabDC1307_rbuf[RTC_SET_SIZE+11]<<0;
	State_Data.Total_Complete_Time = gabDC1307_rbuf[RTC_SET_SIZE+14]<<24 | gabDC1307_rbuf[RTC_SET_SIZE+15]<<16 | gabDC1307_rbuf[RTC_SET_SIZE+16]<<8 | gabDC1307_rbuf[RTC_SET_SIZE+17]<<0;
	Set_Complete_Time(State_Data.Total_Complete_Time);
	
	//State_Data.Working_Time = (gabDC1307_rbuf[10]<<24) | (gabDC1307_rbuf[11]<<16) | (gabDC1307_rbuf[12]<<8) | (gabDC1307_rbuf[13]<<0);
	if(State_Data.Lamp == 1)
	{
		Key.Lamp = 1;
	}
	
		
	//DS1307_GetTime(&gstRTC_time);	
	/*
	debugprintf("RTC Get time--->> %04d-%02d-%02d %02d:%02d:%02d \r\n", gstRTC_time.year + 2000
											  , gstRTC_time.month
											  , gstRTC_time.date
											  , gstRTC_time.hour
											  , gstRTC_time.min
											  , gstRTC_time.sec
											   );	
										   
	sprintf( TimeSettingData.szSettingYear , "%02d", gstRTC_time.year);
	sprintf( TimeSettingData.szSettingMouth , "%02d", gstRTC_time.month);
	sprintf( TimeSettingData.szSettingDay , "%02d", gstRTC_time.date);
	sprintf( TimeSettingData.szSettingHour , "%02d", gstRTC_time.hour);
	sprintf( TimeSettingData.szSettingMin , "%02d", gstRTC_time.min);
   */
	loopdelayms(100);  

	while(1)
	{
		if( (State_Data.Start == 1) )
		{
			debugprintf("Not finished yet\r\n");
			uint8_t i;
			Event_Msg tmp_msg;
			uint8_t tmp_menu;
			uint8_t tmp_mode;
			tmp_msg.Msg_type = TOUCH;
			
			if( State_Data.Menu == 1) //¥Á¿œ
			{
				tmp_msg.Msg_ID = &(pMenu_Object[2]);
			}
			else if( State_Data.Menu == 2) //≥ª¿œ
			{
				tmp_msg.Msg_ID = &(pMenu_Object[1]);
			}
			
			else if( State_Data.Menu >= 3) //»ﬁ¿œ
			{
				tmp_msg.Msg_ID = &(pMenu_Object[3]);
			}
			
			else if( State_Data.Menu == 0)
			{
				if( (State_Data.Mode_Op&0xf0) == 0x10)
				{
					tmp_msg.Msg_ID = &(pMenu_Object[4]);
				}
				else if( (State_Data.Mode_Op&0xf0) == 0x20)
				{
					tmp_msg.Msg_ID = &(pMenu_Object[5]);
				}
				else if( (State_Data.Mode_Op&0xf0) == 0x40)
				{
					tmp_msg.Msg_ID = &(pMenu_Object[6]);
				}
			}
			
			State_Data.Start = 0; //to pass ontouchmenu
			tmp_menu = State_Data.Menu;
			tmp_mode = State_Data.Mode_Op;
			OnMenuTouch(tmp_msg);
			//Draw();
			Set_D_pad_Off();
			Key.Start = 1;
			State_Data.Start = 1;
			State_Data.IsEnd = 0;
			OnStartPressed();
			Graph_Start();
			//CMD_0x33_Retry_cnt = 1;
			
			State_Data.Menu = tmp_menu;
			State_Data.Mode_Op = tmp_mode;
			
			set_draw_target(pRunning_Object[6]->image);
			if(tmp_mode == 0x21)
			{
				for(i=0;i<=Graph_Unit[0].cnt;i++)
				{
					Draw_Surface_Unit(Graph_Unit[0].image,Graph_Unit[0].x+Graph_Unit[0].inc*i,Graph_Unit[0].y);
				}
			}
			else if(tmp_mode == 0x31)
			{
				for(i=0;i<=Graph_Unit[0].cnt;i++)
				{
					Draw_Surface_Unit(Graph_Unit[0].image,Graph_Unit[0].x+Graph_Unit[0].inc*i,Graph_Unit[0].y);
				}
				for(i=0;i<=Graph_Unit[1].cnt;i++)
				{
					Draw_Surface_Unit(Graph_Unit[1].image,Graph_Unit[1].x+Graph_Unit[1].inc*i,Graph_Unit[1].y);
				}
			}
			else if(tmp_mode == 0x41 || tmp_mode == 0x42 || tmp_mode == 0x43 || tmp_mode == 0x44)
			{
				for(i=0;i<=Graph_Unit[0].cnt;i++)
				{
					Draw_Surface_Unit(Graph_Unit[0].image,Graph_Unit[0].x+Graph_Unit[0].inc*i,Graph_Unit[0].y);
				}
				for(i=0;i<=Graph_Unit[1].cnt;i++)
				{
					Draw_Surface_Unit(Graph_Unit[1].image,Graph_Unit[1].x+Graph_Unit[1].inc*i,Graph_Unit[1].y);
				}
				for(i=0;i<=Graph_Unit[2].cnt;i++)
				{
					Draw_Surface_Unit(Graph_Unit[2].image,Graph_Unit[2].x+Graph_Unit[2].inc*i,Graph_Unit[2].y);
				}
			}
				
			//get Correction Time
			//debugprintf("Working Time is %d\r\n",State_Data.Working_Time);
			//debugprintf("Correction Time is %d\r\n",State_Data.Correction_Time);
			//State_Data.Correction_Time = State_Data.Current_Time - State_Data.Start_Time - State_Data.Working_Time;
			
			//Working Time bit set
			State_Data.Working_Time_CMD = 0x11;
			
			UART3_SEND_clear_que();
			UART3_SEND_enqueue( SEND_CMD_0x10, 2000);
			UART3_SEND_enqueue( SEND_CMD_0x11, 2000);
			UART3_SEND_enqueue( SEND_CMD_0x12, 2000);
			UART3_SEND_enqueue( SEND_CMD_0x33, 2000);
			
			
			break;
		}
		else
		{
			State_Data.Start = 0;
			State_Data.Working_Time_CMD = 0x00;
			//State_Data.Working_Time = 0;
			//State_Data.Correction_Time = 0;
			UART3_SEND_clear_que();
			UART3_SEND_enqueue( SEND_CMD_0x10, 2000);
			UART3_SEND_enqueue( SEND_CMD_0x11, 2000);
			UART3_SEND_enqueue( SEND_CMD_0x12, 2000);
			UART3_SEND_enqueue( SEND_CMD_0x33, 2000);
			break;
		}
	}
	State_Data.Current_Time = mktime(&gstRTC_time_t);
	
	
	
	
	
	//GT911_Init();
	GT911_Port_Init();
	loopdelayms(1000);
	/*
	while(sound_isplay(wav_systemon) == 1)
	{
		loopdelayms(10);
	}
	*/
	
		
	//Test mode check
	
	
		
	enable_interrupt(INTNUM_EIRQ0,TRUE);
		
	*R_WDTCNT = 0xFFFFFFFF;
	*R_WDTCTRL = F_WDTCTRL_WDTMOD_RST | F_WDTCTRL_WDTEN_ENABLE;	
	
	
		
	
		
	debugstring("********** Main Start ********** \r\n");
	char capture_name[255];
	char capture_cnt;
	memset(capture_name,0,255);
	capture_cnt = 0;
	//savebmp("asdf.bmp",getfrontframe());

	EndEstimate("Time ");
	
	while(1)
	{
		//StartEstimate();
		*R_WDTCNT = 0x0FFFFFFF;
		draw_cnt++;
		
		
		if(Log_Check() == 0)
		//if(GUI_inf.IsQueued == 1 && Log_Check() == 0)
		{
			Event_Msg ret_msg;
			while( TOUCH_queue_check() != 0)
			{
				ret_msg = TOUCH_dequeue();
				if( ret_msg.Msg_ID != 0)
				{
					if( (*(ret_msg.Msg_ID))->Callback_Function !=0)
					{
						//debugprintf("dequeued\r\n");
						(*(ret_msg.Msg_ID))->Callback_Function(ret_msg);
						*R_WDTCNT = 0x0FFFFFFF;
					}
				}
			}
			GUI_inf.IsQueued = 0 ;
		}
		
		
		if(UART_485_State.state == Tx_waiting && UTIL.bits.ParameterWrite_bit != 0 && Log_Check() == 0)
		{
			//enable_interrupt(INTNUM_TIMER1,FALSE);
			uint32_t ret = CheckFlashWrite();
			
			if(ret == 1)
			{
				if(UTIL.bits.ParameterWrite_bit > 0x2f)
				{
					switch( UTIL.bits.ParameterWrite_bit )
					{
						case 0x30 :
							UART3_SEND_enqueue( SEND_CMD_0x30, 2000);	
							break;
						case 0x31 :
							UART3_SEND_enqueue( SEND_CMD_0x31, 2000);
							break;
						case 0x32 :
							UART3_SEND_enqueue( SEND_CMD_0x32, 2000);
							break;
						default :
							break;
					}
				}
			}
			
			
			
			
			/*
			fret = f_stat("0:setup.txt",&file_info);
			debugprintf("%d\r\n", fret);
			if( fret == FR_NO_FILE )
			{
				fret = f_open(&file, "0:setup.txt", FA_CREATE_NEW|FA_WRITE);
				if(fret == FR_OK)
				{
					f_write(&file, (uint8_t*)&Setup_Data, sizeof(Setup_Data), &bw);
					f_close(&file);
					UTIL.bits.ParameterWrite_bit = 0;
				}
			}
			else if(fret == FR_OK)
			{
				
			}
			*/
			
			//enable_interrupt(INTNUM_TIMER1,TRUE);
			UTIL.bits.ParameterWrite_bit = 0;
		}
		
		// ªÛ≈¬ »Æ¿Œ ø‰√ª

			
		if(RTC_TIMER_1ms  == 0 && twi_isbusy() != 0)
		{
			RTC_TIMER_1ms = 60000;
			//RTC_TIMER_1ms = 1000;
			
			//RTC_TIMER_1ms = 10000;
			//enable_interrupt(INTNUM_EIRQ0,FALSE);
			//GUI_inf.IsInterrupted = 0;
			//TWI_IsUsing = 1;
			//debugprintf("RTC read\r\n");
			if(DS1307_GetTime(&gstRTC_time_t) == -1)
			{
				/*
				gstRTC_time_t.tm_year = 119;
				gstRTC_time_t.tm_mon = 1;
				gstRTC_time_t.tm_mday = 1;
				gstRTC_time_t.tm_hour = 0;
				gstRTC_time_t.tm_min = 0;
				gstRTC_time_t.tm_sec = 1;
				DS1307_SetTime(&gstRTC_time_t);
				loopdelayms(5);
				DS1307_GetTime(&gstRTC_time_t);
				*/
			}
			
			
			if( gstRTC_time_t.tm_sec > 29 )
			{
				RTC_TIMER_1ms = (90-gstRTC_time_t.tm_sec)*1000;
			}
			else
			{
				RTC_TIMER_1ms = (30 - gstRTC_time_t.tm_sec) * 1000;
			}
			
			
			
			//TWI_IsUsing = 0;
			
			//debugprintf("RTC read\r\n");
			if(Key.Power == 0) //only interrupt enable when power on state
			{
				//enable_interrupt(INTNUM_EIRQ0,TRUE);
			}
			State_Data.Current_Time = mktime(&gstRTC_time_t);
		}
		else
		{
			struct tm *tmp_time_t;
			tmp_time_t = localtime(&(State_Data.Current_Time));
			memcpy(&gstRTC_time_t,tmp_time_t,sizeof(struct tm));
		}
		State_Data.Current.Year = gstRTC_time_t.tm_year-100;
		State_Data.Current.Month = gstRTC_time_t.tm_mon+1;
		State_Data.Current.Date = gstRTC_time_t.tm_mday;
		State_Data.Current.Hour = gstRTC_time_t.tm_hour;
		State_Data.Current.Min = gstRTC_time_t.tm_min;
		State_Data.Current.Sec = gstRTC_time_t.tm_sec;
		//uint64_t tmp = (State_Data.Operating_Remain_Time/3600);
		//debugprintf("%d hour %d min %d sec remaining\r\n", tmp,(State_Data.Operating_Remain_Time-(3600*tmp)-(State_Data.Operating_Remain_Time%3600)),(State_Data.Operating_Remain_Time%3600));
		sprintf(String_Current_Time_1,"%04d-%02d-%02d", State_Data.Current.Year+2000, State_Data.Current.Month, State_Data.Current.Date);
		sprintf(String_Current_Time_2,"%02d  :  %02d", State_Data.Current.Hour, State_Data.Current.Min);
			
		if(RTC_DATA_1ms == 0 && twi_isbusy() != 0 )
		{
			//TWI_IsUsing = 1;
			if( DS1307_mode_write() == 0)
			{
				debugprintf("mode_write failed\r\n");
			}
			RTC_DATA_1ms = 10000;
			//TWI_IsUsing = 0;
		}
		
		if(FLASH_TIMER_1ms == 0 )
		{
			Working_Log_t param;
			uint8_t cmp_param[sizeof(Working_Log_t)];
			memset(cmp_param, 0xff, sizeof(cmp_param));
			
			param = Working_Log_Dequeue();
			if( memcmp( (uint8_t*)&param, cmp_param, sizeof(cmp_param)) != 0)
			{
				Working_Log_Write(param);
			}
			param = Error_Log_Dequeue();
			if( memcmp( (uint8_t*)&param, cmp_param, sizeof(cmp_param)) != 0)
			{
				uint32_t i;
				for(i=0;i<sizeof(Working_Log_t);i++)
				{
					debugprintf("%d ", *(((uint8_t*)&param)+i)   );
				}
				debugprintf("\r\n");
				
				
				
				debugprintf("error log dequeued\r\n");
				Error_Log_Write(param);
			}
			FLASH_TIMER_1ms = 10;
		}
		
		
		
		
		if(draw_cnt == 16) 
		{
			draw_cnt = 0;
		}
		
		
		
		
		if( LCD_TIMER_1ms == 0) //lcd backlight dimming
		{
			lcdon(2);
		}
		
		if(POPUP_ERR_TIMER_1ms == 0)
		{
			Popup_Time_Err_Close();
			
		}

		if( IsTest == 1)
		{
			Update_Test_String();
			
			
			if (UART_485_State.retry_cnt > 0xFB)
			{
				Test_Data.Error |= 0x80;
			}
			else
			{
				Test_Data.Error &= ~(0x80);
			}
			
			test_mode_timer--;
			//test_mode exit
			
			if(test_mode_timer == 0)
			{
				Event_Msg msg;
				pObject* msg_id;
				msg_id = find_object("Icon_Return");
				msg.Msg_type = TOUCH;
				msg.Msg_ID = msg_id;
				msg.Point_X = 0;
				msg.Point_Y = 0;
				TOUCH_enqueue(msg);
			}
			
		}

		/*
		// capture
		if((*R_GPILEV(4)&0x40) == 0)
		{
			sprintf(capture_name,"%d.bmp",capture_cnt);
			savebmp(capture_name,getfrontframe());
			capture_cnt++;
			loopdelayms(1000);
		}
		*/
			
		if(popup_close_cnt == 0)
		{
			if(Current_Popup != 0)
			{
				if(Current_Menu != 0)
				{
					Set_Mode_Images(&Current_Mode);
					//Set_Current_Temperature_Humidity_On();
					Set_Default_GUI_Images();
				}
			}
			else if(Current_Popup == 0 && Current_String !=0 )
			{
				Discard_Time_Blinking_Data();
			}
			//popup_close_cnt--;
		}
		else if(popup_close_cnt > 0)
		{
			//debugprintf("%d\r\n",popup_close_cnt);
			popup_close_cnt--;
		}
		
		// not used
		/* 
		if(IsStart == 1)
		{
			OnStartPressed();
			IsStart = 0;
		}
		*/
			
		
		if(IsRedraw == 1)
		{
			Draw();
			
		}
		else
		{
			if(IsRedraw_cnt == 0)
			{
				Draw();
				IsRedraw_cnt ++;
			}
			else if(IsRedraw_cnt > 10)
			{
				IsRedraw_cnt = 0;
			}
		}
		
		
		while( *R_WDTCNT > (0x0FFFFFFF-(uint32_t)(150*1000*50*1.08)) ); //√÷º“¡÷±‚ 150ms
		//EndEstimate("Time ");
	}
	return 0;

}

