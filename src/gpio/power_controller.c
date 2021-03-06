/*
 * power_controller.c
 *
 *	所有的继电器、电源控制GPIO
 *
 *  Created on: Oct 24, 2015
 *      Author: daniel
 */

#include <tc_gpio.h>
#include "logTask.h"

void InitPowerGPIO()
{
	GPIO_InitTypeDef		GPIO_InitStructure;

	// G 口（全部用于控制电源）
	// G0~G7（220v电源插座，G0~G3常开，G4~G7常闭）
	// G8~G15（直流电源继电器）

	/* GPIOG Periph clock enable */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOG, ENABLE);

	/* Configure all pin in output pushpull mode */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_All;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOG, &GPIO_InitStructure);

	// PE8~PE15（控制额外的电源，8路都有PWM输出能力）
	/* GPIOG Periph clock enable */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE, ENABLE);

	GPIO_InitStructure.GPIO_Pin = 0xFF00;
	GPIO_Init(GPIOE, &GPIO_InitStructure);

	// 关闭所有的设备（全部置0）
	GPIO_ResetBits(GPIOG, GPIO_Pin_All);
	GPIO_ResetBits(GPIOE, 0xFF00);

	// 特殊处理：蛋分器需要延迟启动，避免水位太高导致暴冲
	Switch_ProteinSkimmer(POWER_OFF);
}

#define SWITCH(on, device) 		on ? GPIO_SetBits(device##_GPIO, device##_PIN) : GPIO_ResetBits(device##_GPIO, device##_PIN)

// 24v/12v低压设备开关
// 主上水泵（有电流监测）
#define MAINPUMP_GPIO		GPIOG
#define MAINPUMP_PIN		GPIO_Pin_11
void Switch_MainPump(BaseType_t bOn)
{
	SWITCH(bOn, MAINPUMP);
	LogOutput(LOG_INFO, "Turn %s main pump.", bOn ? "on" : "off");
}

// 蛋白质分离器（有电流监测）
#define PROTEINSKIMMER_GPIO		GPIOG
#define PROTEINSKIMMER_PIN		GPIO_Pin_10
void Switch_ProteinSkimmer(BaseType_t bOn)
{
	SWITCH(bOn, PROTEINSKIMMER);
	LogOutput(LOG_INFO, "Turn %s protein skimmer.", bOn ? "on" : "off");
}

// 主缸造浪泵
#define WAVEMAKER_GPIO			GPIOG
#define WAVEMAKER_PIN			GPIO_Pin_9
void Switch_WaveMaker(BaseType_t bOn)
{
	SWITCH(bOn, WAVEMAKER);
	LogOutput(LOG_INFO, "Turn %s main wave maker.", bOn ? "on" : "off");
}

// 主缸造浪泵（夜间模式，备用模式）
#define WAVEMAKER_NIGHTMODE_GPIO	GPIOG
#define WAVEMAKER_NIGHTMODE_PIN		GPIO_Pin_8
void Switch_WaveMakerNightMode(BaseType_t bOn)
{
	SWITCH(bOn, WAVEMAKER_NIGHTMODE);
	// 关闭24V电源（或打开主电源）
	Switch_WaveMaker(!bOn);
}

// 底缸造浪泵（无电流监测）
#define SUB_WAVEMAKER_GPIO		GPIOG
#define SUB_WAVEMAKER_PIN		GPIO_Pin_12
void Switch_SubWaveMaker(BaseType_t bOn)
{
	SWITCH(bOn, SUB_WAVEMAKER);
	LogOutput(LOG_INFO, "Turn %s sub wave maker.", bOn ? "on" : "off");
}

// TODO:RO补水和外置补水泵还未确认，可能颠倒（和电磁阀同时开关）
// RO补水泵
#define RO_PUMP_GPIO		GPIOG
#define RO_PUMP_PIN			GPIO_Pin_13
void Switch_RoPump(BaseType_t bOn)
{
	SWITCH(bOn, RO_PUMP);
	LogOutput(LOG_INFO, "Turn %s RO reill pump.", bOn ? "on" : "off");

	// 同时开启/关闭电磁阀
}

// RO外置备份补水泵
#define BACKUP_RO_PUMP_GPIO		GPIOG
#define BACKUP_RO_PUMP_PIN		GPIO_Pin_14
void Switch_BackupRoPump(BaseType_t bOn)
{
	SWITCH(bOn, BACKUP_RO_PUMP);
	LogOutput(LOG_INFO, "Turn %s backup RO reill pump.", bOn ? "on" : "off");

	// 同时开启/关闭电磁阀

}

// 海水排水泵
#define SEA_PUMP_INOUT_GPIO		GPIOG
#define SEA_PUMP_INOUT_PIN		GPIO_Pin_15
void Switch_SeaPumpInOut(BaseType_t bOn)
{
	SWITCH(bOn, SEA_PUMP_INOUT);
}

// 打开24V升压电源
#define BACKUP_POWER_GPIO		GPIOE
#define BACKUP_POWER_PIN		GPIO_Pin_14
void Switch_BackupPower(BaseType_t bOn)
{
	SWITCH(bOn, BACKUP_POWER);
}

// 24V（开关电源）和 24V（备用电源）切换
#define TO_BACKUP_POWER_GPIO	GPIOE
#define TO_BACKUP_POWER_PIN		GPIO_Pin_15
void Switch_ToBackupPower(BaseType_t bOn)
{
	SWITCH(bOn, TO_BACKUP_POWER);
}

// Beep
#define BEEP_GPIO				GPIOE
#define BEEP_PIN				GPIO_Pin_8
void Switch_Beep(BaseType_t bOn)
{
	SWITCH(bOn, BEEP);
}

// ========================================================================
// 220V设备
// 常开
// T5HO灯管（两根一起控制）
#define T5HO_1_GPIO				GPIOG
#define T5HO_2_GPIO				GPIOG
#define T5HO_1_PIN				GPIO_Pin_0
#define T5HO_2_PIN				GPIO_Pin_3
void Switch_T5H0(BaseType_t bOn)
{
	// 两盏灯（4支T5H2灯管）
	SWITCH(bOn, T5HO_1);
	SWITCH(bOn, T5HO_2);
}

// 加热棒（一）
#define HEATER_1_GPIO			GPIOG
#define HEATER_2_GPIO			GPIOG
#define HEATER_1_PIN			GPIO_Pin_1
#define HEATER_2_PIN			GPIO_Pin_2
void Switch_Heater1(BaseType_t bOn)
{
	SWITCH(bOn, HEATER_1);
}

// 加热棒（二）
void Switch_Heater2(BaseType_t bOn)
{
	SWITCH(bOn, HEATER_2);
}

// 常闭
// 冷水机
#define CHILLER_GPIO				GPIOG
#define CHILLER_PIN				GPIO_Pin_4
void Switch_Chiller(BaseType_t bOn)
{
	SWITCH(!bOn, CHILLER);
}

// 主缸灯光
#define MAIN_LIGHT_GPIO			GPIOG
#define MAIN_LIGHT_PIN			GPIO_Pin_5
void Switch_MainLight(BaseType_t bOn)
{
	SWITCH(!bOn, MAIN_LIGHT);
}

// 底缸灯
#define SUB_LIGHT_GPIO			GPIOG
#define SUB_LIGHT_PIN			GPIO_Pin_6
void Switch_SubLight(BaseType_t bOn)
{
	SWITCH(!bOn, SUB_LIGHT);
}

// 备用插座（常开）

