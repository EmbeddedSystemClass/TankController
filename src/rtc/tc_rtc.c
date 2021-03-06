/*
 * tc_rtc.c
 *
 *  Created on: Jan 3, 2016
 *      Author: daniel
 */

#include "tc_rtc.h"
#include <stdio.h>
#include <time.h>
#include <stdlib.h>


// Configures RTC
void RTC_Configuration(void)
{
	/* Enable PWR and BKP clocks */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);

	/* Allow access to BKP Domain */
	PWR_BackupAccessCmd(ENABLE);

	/* Reset Backup Domain */
	BKP_DeInit();

	/* Enable LSE */
	RCC_LSEConfig(RCC_LSE_ON);
	/* Wait till LSE is ready */
	while (RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET)
	{
	}

	/* Select LSE as RTC Clock Source */
	RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);

	/* Enable RTC Clock */
	RCC_RTCCLKCmd(ENABLE);

	/* Wait for RTC registers synchronization */
	RTC_WaitForSynchro();

	/* Wait until last write operation on RTC registers has finished */
	RTC_WaitForLastTask();

	/* Set RTC prescaler: set RTC period to 1sec */
	RTC_SetPrescaler(32767); /* RTC period = RTCCLK/RTC_PR = (32.768 KHz)/(32767+1) */

	/* Wait until last write operation on RTC registers has finished */
	RTC_WaitForLastTask();
}

void InitRTC(void)
{
	// 设置时区（固定为东八区，北京时间）
	setenv("TZ", "CST-8", 0);
	tzset();

	if (BKP_ReadBackupRegister(BKP_DR1) != 0xA5A5)
	{
		/* Backup data register value is not correct or not yet programmed (when
		 the first time the program is executed) */
		/* RTC Configuration */
		RTC_Configuration();

		RTC_WaitForLastTask();
		// 1970 1 1 00:00:00
		RTC_SetCounter(0);
		RTC_WaitForLastTask();

		BKP_WriteBackupRegister(BKP_DR1, 0xA5A5);
	}
	else
	{
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);
		PWR_BackupAccessCmd(ENABLE);

		RTC_WaitForSynchro();
	}

	/* Clear reset flags */
	RCC_ClearFlag();
}

void GetRTC(uint32_t* pYear, uint32_t* pMonth, uint32_t* pDate, uint32_t* pWday, uint32_t* pHour, uint32_t* pMinute, uint32_t* pSecond)
{
	uint32_t		now_sec;
	struct tm		now;

	now_sec = RTC_GetCounter();
	localtime_r((time_t*)&now_sec, &now);

	if (pYear)
	{
		*pYear = now.tm_year + 1900;
	}

	if (pMonth)
	{
		*pMonth = now.tm_mon + 1;
	}

	if (pDate)
	{
		*pDate = now.tm_mday;
	}

	if (pWday)
	{
		*pWday = now.tm_wday;
	}

	if (pHour)
	{
		*pHour = now.tm_hour;
	}

	if (pMinute)
	{
		*pMinute = now.tm_min;
	}

	if (pSecond)
	{
		*pSecond = now.tm_sec;
	}
}

// 记录系统启动后运行时间（单位：秒）
static uint32_t		s_Uptime = 0;

uint32_t GetUptime(uint32_t* day, uint32_t* hour, uint32_t* minute, uint32_t* second)
{
	uint32_t	LeftTime = s_Uptime;

	if (second)
	{
		*second = LeftTime % 60;
	}

	// 转换为分钟
	LeftTime = LeftTime / 60;

	if (minute)
	{
		*minute = LeftTime % 60;
	}

	// 转换为小时
	LeftTime = LeftTime / 60;

	if (hour)
	{
		*hour = LeftTime % 24;
	}

	// 转换为天
	LeftTime = LeftTime / 24;

	if (day)
	{
		*day = LeftTime;
	}

	return s_Uptime;
}

// 利用xTaskGetTickCount累加计数
void UpdateUptime()
{
	static TickType_t		tLastTick = 0;

	TickType_t				tNow;
	TickType_t				tDelta;

	// 获取当前的，然后和前一次比较
	tNow = xTaskGetTickCount();

	if ((tLastTick == 0) && (s_Uptime == 0))
	{
		tLastTick = tNow;
	}
	else
	{
		if (tNow >= tLastTick)
		{
			tDelta = tNow - tLastTick;
		}
		else
		{
			tDelta = portMAX_DELAY - tLastTick + tNow;
		}

		if (tDelta >= 1000)
		{
			// 将偏差的秒数加入到Uptime，然后更新tLastTick
			s_Uptime += tDelta / 1000;
			tLastTick = tNow;
		}
	}
}












// -------------------------Command line----------------------------------
// 设置时间，获取时间（没有参数时）
static BaseType_t cmd_time( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString )

{
	uint32_t		now_sec;
	struct tm		now;
	const char *	pcParameter;
	BaseType_t 		xParameterStringLength;

	pcParameter = FreeRTOS_CLIGetParameter(pcCommandString, 1, &xParameterStringLength);

	if (!pcParameter)
	{
		// 没有第一个参数，获取时间
		now_sec = RTC_GetCounter();
		localtime_r((time_t*)&now_sec, &now);

		snprintf(pcWriteBuffer, xWriteBufferLen, "time %d-%02d-%02d %02d:%02d:%02d\r\n",
				now.tm_year + 1900,
				now.tm_mon + 1,
				now.tm_mday,
				now.tm_hour,
				now.tm_min,
				now.tm_sec);
	}
	else
	{
		// 设置时间（需要年、月、日、时、分，5个必须参数，秒可选）
		do
		{
			now.tm_year = atoi(pcParameter) - 1900;
			if (now.tm_year < 100)
			{
				snprintf(pcWriteBuffer, xWriteBufferLen, "Year wrong!\r\n");
				break;
			}

			pcParameter = FreeRTOS_CLIGetParameter(pcCommandString, 2, &xParameterStringLength);
			if (!pcParameter)
			{
				break;
			}
			now.tm_mon = atoi(pcParameter) - 1;
			if ( (now.tm_mon < 0) || (now.tm_mon > 12) )
			{
				snprintf(pcWriteBuffer, xWriteBufferLen, "Month wrong!\r\n");
				break;;
			}

			pcParameter = FreeRTOS_CLIGetParameter(pcCommandString, 3, &xParameterStringLength);
			if (!pcParameter)
			{
				break;
			}
			now.tm_mday = atoi(pcParameter);
			if (now.tm_mday > 31)
			{
				snprintf(pcWriteBuffer, xWriteBufferLen, "Day wrong!\r\n");
				break;;
			}

			pcParameter = FreeRTOS_CLIGetParameter(pcCommandString, 4, &xParameterStringLength);
			if (!pcParameter)
			{
				break;
			}
			now.tm_hour = atoi(pcParameter);
			if ( (now.tm_hour < 0) || (now.tm_hour > 23) )
			{
				snprintf(pcWriteBuffer, xWriteBufferLen, "Hour wrong!\r\n");
				break;
			}

			pcParameter = FreeRTOS_CLIGetParameter(pcCommandString, 5, &xParameterStringLength);
			if (!pcParameter)
			{
				break;
			}
			now.tm_min = atoi(pcParameter);
			if ( (now.tm_min < 0) || (now.tm_min > 59) )
			{
				snprintf(pcWriteBuffer, xWriteBufferLen, "Minute wrong!\r\n");
				break;
			}

			pcParameter = FreeRTOS_CLIGetParameter(pcCommandString, 6, &xParameterStringLength);
			if (pcParameter)
			{
				now.tm_sec = atoi(pcParameter);
				if ( (now.tm_sec < 0) || (now.tm_sec > 59) )
				{
					snprintf(pcWriteBuffer, xWriteBufferLen, "Second wrong!\r\n");
					break;
				}
			}
			else
			{
				now.tm_sec = 0;
			}

			/* Wait until last write operation on RTC registers has finished */
			RTC_WaitForLastTask();
			/* Change the current time */
			RTC_SetCounter(mktime(&now));
			/* Wait until last write operation on RTC registers has finished */
			RTC_WaitForLastTask();

			snprintf(pcWriteBuffer, xWriteBufferLen, "Set time successfully!\r\n");

			return pdFALSE;
		} while(0);

		// 参数不对
		snprintf(pcWriteBuffer, xWriteBufferLen, "Fail to set time.\r\n");
	}

	return pdFALSE;
}

const CLI_Command_Definition_t cmd_def_time =
{
	"time",
	"\r\ntime [yyyy mm dd hh mm [ss]] \r\n Get&Set RTC date&time.\r\n",
	cmd_time, /* The function to run. */
	-1
};






