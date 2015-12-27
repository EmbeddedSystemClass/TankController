/*
 * tc_serial.h
 *
 *  Created on: Oct 24, 2015
 *      Author: daniel
 */

#ifndef UART_TC_SERIAL_H_
#define UART_TC_SERIAL_H_

#include <FreeRTOS.h>
#include "FreeRTOS_CLI.h"

void InitShell( void );
void EnableBluetooth(BaseType_t bEnable);

void InitUltraSoundSensors( void );

// 超声波传感器对应CD4052通道
#define MAIN_US_PORT				0
#define SUB_US_PORT					1
#define RO_US_PORT					2

// 获取指定缸的水位（传感器距离水面距离）
BaseType_t GetDistance(uint8_t Port, uint16_t* pData);

#endif /* UART_TC_SERIAL_H_ */
