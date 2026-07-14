/*
 * PID.h
 *
 *  Created on: 13 févr. 2026
 *      Author: valentin
 */

#ifndef PID_INC_PID_H_
#define PID_INC_PID_H_

#include "main.h"
#include "NTC_thermistor.h"
#include "OLED_096.h"
#include "interface.h"
//#include <stdlib.h>
//#include <string.h>
//#include <stdio.h>

typedef struct
{
	uint16_t Kp;
	uint16_t Ki;
	uint16_t Kd;
	int16_t errint;
}PIDSetting;

int PID_PWM_cmd(int SP, int PV, uint32_t delta_t, PIDSetting *PID);
void PID_Init(void);
PIDSetting PID_AutoTune(uint16_t target_temp, uint32_t timeout);

#endif /* PID_INC_PID_H_ */
