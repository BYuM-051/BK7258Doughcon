/******************************************************************************
 Copyright (C) 2011      Advanced Digital Chips Inc. 
						http://www.adc.co.kr
 Author : Software Team.

******************************************************************************/

#include "adstar.h"
#include <time.h>
#include "Mydefine.h"

unsigned char gabDC1307_rbuf[64] = {0,};
unsigned char gabDC1307_wbuf[64] = {0,};

static uint32_t DS1307_fail_cnt;
time_t time_last = 1560157200; // 2019-06-10 AM 09:00


uint32_t DS1307_mode_write(void)
{
	int nret;
	unsigned char atmp[1];
	atmp[0] = 8;
	
	uint8_t buffer[20];
	memset(buffer,0,sizeof(buffer));
	
	buffer[0] = State_Data.Menu;
	buffer[1] = State_Data.Mode_Op;
	buffer[2] = State_Data.Start;
	buffer[3] = State_Data.Lamp;
	buffer[4] = State_Data.Working_Time_1h;
	buffer[5] = State_Data.Working_Time_1m;
	buffer[6] = State_Data.Operating_Remain_Time_1h;
	buffer[7] = State_Data.Operating_Remain_Time_1m;
	buffer[8] = (uint8_t)(State_Data.Current_Time>>24);
	buffer[9] = (uint8_t)(State_Data.Current_Time>>16);
	buffer[10] = (uint8_t)(State_Data.Current_Time>>8);
	buffer[11] = (uint8_t)(State_Data.Current_Time>>0);
	buffer[12] = 0;
	buffer[13] = 0;
	buffer[14] = (uint8_t)(State_Data.Total_Complete_Time>>24);
	buffer[15] = (uint8_t)(State_Data.Total_Complete_Time>>16);
	buffer[16] = (uint8_t)(State_Data.Total_Complete_Time>>8);
	buffer[17] = (uint8_t)(State_Data.Total_Complete_Time>>0);
	buffer[18] = 0;
	buffer[19] = 0;
	

	/*
	if(State_Data.Start == 1)
	{
		State_Data.Working_Time = State_Data.Current_Time - State_Data.Start_Time - State_Data.Correction_Time;
	
		gabDC1307_wbuf[10] = (State_Data.Working_Time&0xFF000000)>>24;
		gabDC1307_wbuf[11] = (State_Data.Working_Time&0x00FF0000)>>16;
		gabDC1307_wbuf[12] = (State_Data.Working_Time&0x0000FF00)>>8;
		gabDC1307_wbuf[13] = (State_Data.Working_Time&0x000000FF)>>0;
		
		debugprintf("Working Time is %d\r\n",State_Data.Working_Time);
		debugprintf("Correction Time is %d\r\n",State_Data.Correction_Time);
	}
	else
	{
		State_Data.Working_Time = 0;
		gabDC1307_wbuf[10] = 0;
		gabDC1307_wbuf[11] = 0;
		gabDC1307_wbuf[12] = 0;
		gabDC1307_wbuf[13] = 0;
	}
	*/
	
	
	
	//nret = twi_write( DS1307_DEVICE_ADDR, atmp, 1, &(buffer[0]), sizeof(buffer) );
	nret = twi_write_int( DS1307_DEVICE_ADDR, atmp, 1, &(buffer[0]), sizeof(buffer) );
	return nret;
}

uint32_t DS1307_Write(uint8_t addr, uint8_t* buffer, uint32_t size)
{
	int nret;
	unsigned char atmp[1];
	atmp[0] = addr;
	
	//nret = twi_write( DS1307_DEVICE_ADDR, atmp, 1, buffer, size );
	nret = twi_write_int( DS1307_DEVICE_ADDR, atmp, 1, buffer, size );
	return nret;
	/*
	do
	{
		nret = twi_iscompleted();
	}
	while( nret == 0);
	if(nret == -1 || nret == FALSE)
	{
		return 0;
	}
	else
	{
		if(nret == size)
		{
			return nret;
		}
		else
		{
			return 0;
		}
	}
	*/
}

uint32_t DS1307_Read(uint8_t addr, uint8_t* out_buffer, uint32_t size, function_pointer Function_Pointer)
{
	int nret;
	unsigned char atmp[1];
	atmp[0] = addr;
	
	//nret = twi_read( DS1307_DEVICE_ADDR, atmp, 1, buffer, size);
	//nret = twi_read_int( DS1307_DEVICE_ADDR, atmp, 1, buffer, size,DOSOMEHING);
	nret = twi_read_int( DS1307_DEVICE_ADDR, atmp, 1, out_buffer, size,Function_Pointer);
	return nret;
}

/*=========================================================================
	
=========================================================================*/

static void DS1307_init_callback()
{
	asm("nop");
	debugprintf("init post process\r\n");
}


uint32_t DS1307_init(void)
{	
	
	// UTIL.bits.PumpChangeInit = 1;	
		
	int i;
	uint32_t nret;
	
	nret = DS1307_Read(0,gabDC1307_rbuf,sizeof(gabDC1307_rbuf),DS1307_init_callback);
	do
	{
		nret = twi_isbusy();
	}
	while( nret == 0);
	
	nret = twi_iscompleted();
	
	if( nret == -1)
	{
		debugprintf("DS1307 Init failed\r\n");
		return -1;
	}		
		
	debugprintf("ds1307 value\t");
	for(i=0;i<64;i++)
	{
		debugprintf("%02x ",gabDC1307_rbuf[i]);
	}
	debugprintf("\r\n");

	
	return 0;
}
/*=========================================================================
	
=========================================================================*/
S32 DS1307_SetTime(struct tm* pDT)
{	
	debugprintf("RTC set time ");
	gabDC1307_wbuf[0] = (pDT->tm_sec/10)<<4;
	gabDC1307_wbuf[0] += (pDT->tm_sec%10);
	
	gabDC1307_wbuf[1] = (pDT->tm_min/10)<<4;
	gabDC1307_wbuf[1] += (pDT->tm_min%10);
	
	gabDC1307_wbuf[2] = (pDT->tm_hour/10)<<4;
	gabDC1307_wbuf[2] += (pDT->tm_hour%10);	
	
	//gabDC1307_wbuf[3] = (pDT->day/10)<<4;
	//gabDC1307_wbuf[3] += (pDT->day%10);
	
	gabDC1307_wbuf[4] = (pDT->tm_mday/10)<<4;
	gabDC1307_wbuf[4] += (pDT->tm_mday%10);
	
	gabDC1307_wbuf[5] = ((pDT->tm_mon+1)/10)<<4;
	gabDC1307_wbuf[5] += ((pDT->tm_mon+1)%10);
	
	gabDC1307_wbuf[6] = ((pDT->tm_year-100)/10)<<4;
	gabDC1307_wbuf[6] += (pDT->tm_year%10);	
	
	gabDC1307_wbuf[7] = 0x13;
	
	
	
	int i;
	for( i = 0; i< 8; i++)
	{
		printf("%x ", gabDC1307_wbuf[7-i] );
	}
	printf("\r\n");
	if(DS1307_Write(0, gabDC1307_wbuf, 8) == 0)
	{
		debugstring("Set fault DS1307 RTC time\r\n");
		return -1;
	}
	
//	debugstring("Set DS1307 RTC time\r\n");
	return 0;
}
/*=========================================================================	
=========================================================================*/

static struct tm* read_time_tm;

void DS1307_GetTime_callback()
{
	asm("nop");
	//debugprintf("gettime post process\r\n");
	
	
	uint8_t i;
	uint8_t j=0;
	uint32_t nret = 0;
	struct tm* pDT = read_time_tm;
	
	debugprintf("RTC get time\t");
	for(i=0;i<8;i++)
	{
		debugprintf("%x ", gabDC1307_rbuf[i]);
	}
	debugprintf("\r\n");
	
	
	/*
	printf("%04d-%02d-%02d %02d:%02d:%02d\r\n", ( ( gabDC1307_rbuf[6] & 0xf ) %10 ) + ( ( gabDC1307_rbuf[6] >> 4 ) * 10 ) + 2000
											  , ( ( gabDC1307_rbuf[5] & 0xf ) %10 ) + ( ( gabDC1307_rbuf[5] >> 4 ) * 10 )
											  , ( ( gabDC1307_rbuf[4] & 0xf ) %10 ) + ( ( gabDC1307_rbuf[4] >> 4 ) * 10 )
											  , ( ( gabDC1307_rbuf[2] & 0xf ) %10 ) + ( ( gabDC1307_rbuf[2] >> 4 ) * 10 )
											  , ( ( gabDC1307_rbuf[1] & 0xf ) %10 ) + ( ( gabDC1307_rbuf[1] >> 4 ) * 10 )
											  , ( ( gabDC1307_rbuf[0] & 0xf ) %10 ) + ( ( gabDC1307_rbuf[0] >> 4 ) * 10 )
											   );
	  */
	for(i=0;i<8;i++)
	{
		if(gabDC1307_rbuf[i] == 0)
		{
			j++;
		}
	}
	if(j == 8)
	{
		nret = -1;
	}
	if( (gabDC1307_rbuf[0]&0x0f) > 0x09)
	{
		DS1307_fail_cnt ++;
		debugprintf("Seconds failed\r\n");
		nret = -1;
	}
	if( (gabDC1307_rbuf[0]&0xf0) > 0x50)
	{
		DS1307_fail_cnt ++;
		debugprintf("10 Seconds failed\r\n");
		nret = -1;
	}
	if( (gabDC1307_rbuf[1]&0x0f) > 0x09)
	{
		DS1307_fail_cnt ++;
		debugprintf("Minutes failed\r\n");
		nret = -1;
	}
	if( (gabDC1307_rbuf[1]&0xf0) > 0x50)
	{
		DS1307_fail_cnt ++;
		debugprintf("10 Minutes failed\r\n");
		nret = -1;
	}
	if( (gabDC1307_rbuf[2]&0x0f) > 0x09)
	{
		DS1307_fail_cnt ++;
		debugprintf("Hours failed\r\n");
		nret = -1;
	}
	if( (gabDC1307_rbuf[2]&0xf0) > 0x30)
	{
		DS1307_fail_cnt ++;
		debugprintf("10 Hours failed\r\n");
		nret = -1;
	}
	if( (gabDC1307_rbuf[4]&0x0f) > 0x09)
	{
		DS1307_fail_cnt ++;
		debugprintf("Date failed\r\n");
		nret = -1;
	}
	if( (gabDC1307_rbuf[4]&0xf0) > 0x30)
	{
		DS1307_fail_cnt ++;
		debugprintf("10 Date failed\r\n");
		nret = -1;
	}
	if( (gabDC1307_rbuf[4] ) == 0x00 )
	{
		DS1307_fail_cnt += 11;
		debugprintf("Date failed\r\n");
		nret = -1;
	}
	if( (gabDC1307_rbuf[5]&0x0f) > 0x09)
	{
		DS1307_fail_cnt ++;
		debugprintf("Month failed\r\n");
		nret = -1;
	}
	if( (gabDC1307_rbuf[5]&0xf0) > 0x10)
	{
		DS1307_fail_cnt ++;
		debugprintf("Month failed\r\n");
		nret = -1;
	}
	if( (gabDC1307_rbuf[5]) == 0x00)
	{
		DS1307_fail_cnt += 10;
		debugprintf("Month failed\r\n");
		nret = -1;
	}
	if( (gabDC1307_rbuf[6]&0x0f) > 0x09)
	{
		DS1307_fail_cnt ++;
		debugprintf("Year failed\r\n");
		nret = -1;
	}
	if( (gabDC1307_rbuf[6]&0xf0) > 0x90)
	{
		DS1307_fail_cnt ++;
		debugprintf("10 Year failed\r\n");
		nret = -1;
	}
	if( (gabDC1307_rbuf[7]&0x20) != 0)
	{
		DS1307_fail_cnt ++;
		debugprintf("Control bit failed\r\n");
		nret = -1;
	}
	
	if( nret == 0)
	{
		pDT->tm_year 	=  ( ( gabDC1307_rbuf[6] & 0xf ) %10 ) + ( ( gabDC1307_rbuf[6] >> 4 ) * 10 )+100;
		pDT->tm_mon = ( ( gabDC1307_rbuf[5] & 0xf ) %10 ) + ( ( gabDC1307_rbuf[5] >> 4 ) * 10 )-1;
		pDT->tm_mday 	= ( ( gabDC1307_rbuf[4] & 0xf ) %10 ) + ( ( gabDC1307_rbuf[4] >> 4 ) * 10 );
		//pDT->day = ( ( gabDC1307_rbuf[3] & 0xf ) %10 ) + ( ( gabDC1307_rbuf[3] >> 4 ) * 10 );
		pDT->tm_hour 	= ( ( gabDC1307_rbuf[2] & 0xf ) %10 ) + ( ( gabDC1307_rbuf[2] >> 4 ) * 10 );
		pDT->tm_min 	= ( ( gabDC1307_rbuf[1] & 0xf ) %10 ) + ( ( gabDC1307_rbuf[1] >> 4 ) * 10 );
		pDT->tm_sec 	= ( ( gabDC1307_rbuf[0] & 0xf ) %10 ) + ( ( gabDC1307_rbuf[0] >> 4 ) * 10 );
	
		time_last=mktime(pDT);
		
		DS1307_fail_cnt = 0;
		
	}
	else
	{
		if(DS1307_fail_cnt > 10)
		{
			debugprintf("try to recover time\r\n");
			struct tm* new_time;
			new_time = localtime(&time_last);
			DS1307_SetTime(new_time);
		}
		
	}
	
	
	

	//int i;
	// for( i = 0; i< 8; i++ )
	// {
	// 	printf("%04d ", gabDC1307_rbuf[i] );
	// }
	// printf("\r\n");
	//return nret;
}

S32 DS1307_GetTime(struct tm* pDT)
{
			
	read_time_tm = pDT;
	if(DS1307_Read(0,gabDC1307_rbuf,8, DS1307_GetTime_callback) == 0)
	{
		debugprintf("DS1307 RTC time get fail\r\n");
		return -1;
	}
	else
	{
		return 0;
	}
}


