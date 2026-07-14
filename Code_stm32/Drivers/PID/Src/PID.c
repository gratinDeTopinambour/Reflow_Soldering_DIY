/*
 * PID.c
 *
 *  Created on: 13 févr. 2026
 *      Author: valentin
 */

#include "PID.h"

#define MAX_PWM 999

PIDSetting PID_set[6];

extern ADC_HandleTypeDef hadc1;
extern uint32_t PWM_data[1];
extern TIM_HandleTypeDef htim1;

int err_int; //integer error
int prev_err; //previous error

volatile int PWM_cycle = 0;

//SP set point
//PV process variable
//delta_t in ms
//return value between 0 and 100 (0% to 100% PWM)
int PID_PWM_cmd(int SP, int PV, uint32_t delta_t, PIDSetting *PID)
{
	int err = SP - PV;
	int cmd;

	//switch on oven until it reach 20°C error
	if(err > 20){
		PID->errint = 0;
		return 999;
	}

	//switch off oven if temp is 10°C higher than set point
	if(err < -10){
		return 0;
	}

	int err_drv = 1000*(err - prev_err)/delta_t;
	PID->errint += err*delta_t/1000;
	if(PID->errint  > 100) PID->errint  = 100;
	if(PID->errint  < -100) PID->errint  = -100;

	cmd = PID->Kp*err + PID->Ki*PID->errint  + PID->Kd*err_drv;

	prev_err = err;

	if(cmd > 999) cmd = 999;
	if(cmd < 0) cmd = 0;

	return cmd;
}

void PID_Init(void)
{
	for(uint8_t i = 0; i<6; i++)
	{
		PID_set[i].Kp = 999;
		PID_set[i].Ki = 0;
		PID_set[i].Kd = 0;
		PID_set[i].errint = 0;
	}
}

PIDSetting PID_AutoTune(uint16_t target_temp, uint32_t timeout)
{
	PIDSetting PID_auto;

	uint16_t current_temp = 0;
	uint32_t start_time = HAL_GetTick();
	uint32_t current_time = HAL_GetTick();

	uint8_t current_cycle = 0;
	uint8_t cycle_to_run = 3;
	uint16_t peak_max = 0;
	uint16_t peak_min = 1000;

	uint32_t t_start_cycle = 0;
	uint32_t t_period = 0;

	char temp[11];
	strcpy(temp, "T:");
	char max_temp[11];
	strcpy(max_temp, "max");
	char min_temp[11];
	strcpy(min_temp, "min");
	char cur_time[11];
	strcpy(cur_time, "time: ");
	char cycle[11];
	strcpy(cycle, "cycle: ");
	mini_sprintf(current_temp, temp + 2, 3);
	OLED_DrawString(temp, WHITE, 0, 15);
	mini_sprintf(peak_max, max_temp + 3, 3);
	OLED_DrawString(max_temp, WHITE, 42, 15);
	mini_sprintf(peak_min, min_temp + 3, 3);
	OLED_DrawString(min_temp, WHITE, 90, 15);

	mini_sprintf((current_time - start_time)/1000, cur_time + 6, 4);
	OLED_DrawString(cur_time, WHITE, 64-strlen(cur_time)*3, 30);
	mini_sprintf(current_cycle, cycle + 7, 1);
	OLED_DrawString(cycle, WHITE, 66, 45);
	OLED_UpdateScreen();

	PWM_data[0] = MAX_PWM;
	HAL_TIM_PWM_Start_DMA(&htim1, TIM_CHANNEL_1, PWM_data,1);

	while(current_temp <= target_temp && current_time < timeout + start_time)
	{
		mini_sprintf((current_time - start_time)/1000, cur_time + 6, 4);
		OLED_DrawString(cur_time, WHITE, 64-strlen(cur_time)*3, 30);
		mini_sprintf(current_temp, temp + 2, 3);
		OLED_DrawString(temp, WHITE, 0, 15);
		OLED_UpdateScreen();

		current_time = HAL_GetTick();
		current_temp = (NTC_Read_Value(&hadc1) + NTC_Read_Value(&hadc1) + NTC_Read_Value(&hadc1))/3;
		HAL_Delay(100);
	}
	mini_sprintf(current_temp, temp + 2, 3);
	OLED_DrawString(temp, WHITE, 0, 15);
	mini_sprintf(peak_max, max_temp + 3, 3);
	OLED_DrawString(max_temp, WHITE, 42, 15);
	mini_sprintf(peak_min, min_temp + 3, 3);
	OLED_DrawString(min_temp, WHITE, 90, 15);

	mini_sprintf((current_time - start_time)/1000, cur_time + 6, 4);
	OLED_DrawString(cur_time, WHITE, 64-strlen(cur_time)*3, 30);
	mini_sprintf(current_cycle, cycle + 7, 1);
	OLED_DrawString(cycle, WHITE, 66, 45);
	OLED_UpdateScreen();

	PWM_data[0] = 0;
	HAL_TIM_PWM_Start_DMA(&htim1, TIM_CHANNEL_1, PWM_data,1);

	while(current_temp >= target_temp && current_time < timeout + start_time)
	{
		mini_sprintf((current_time - start_time)/1000, cur_time + 6, 4);
		OLED_DrawString(cur_time, WHITE, 64-strlen(cur_time)*3, 30);
		mini_sprintf(current_temp, temp + 2, 3);
		OLED_DrawString(temp, WHITE, 0, 15);
		OLED_UpdateScreen();

		current_time = HAL_GetTick();
		current_temp = (NTC_Read_Value(&hadc1) + NTC_Read_Value(&hadc1) + NTC_Read_Value(&hadc1))/3;

		if (current_temp > peak_max)
		{
			peak_max = current_temp;
			mini_sprintf(peak_max, max_temp + 3, 3);
			OLED_DrawString(max_temp, WHITE, 42, 15);
		}
		if (current_temp < peak_min)
		{
			peak_min = current_temp;
			mini_sprintf(peak_min, min_temp + 3, 3);
			OLED_DrawString(min_temp, WHITE, 90, 15);
		}
		HAL_Delay(100);

	}
	mini_sprintf(current_temp, temp + 2, 3);
	OLED_DrawString(temp, WHITE, 0, 15);
	mini_sprintf(peak_max, max_temp + 3, 3);
	OLED_DrawString(max_temp, WHITE, 42, 15);
	mini_sprintf(peak_min, min_temp + 3, 3);
	OLED_DrawString(min_temp, WHITE, 90, 15);

	mini_sprintf((current_time - start_time)/1000, cur_time + 6, 4);
	OLED_DrawString(cur_time, WHITE, 64-strlen(cur_time)*3, 30);
	mini_sprintf(current_cycle, cycle + 7, 1);
	OLED_DrawString(cycle, WHITE, 66, 45);
	OLED_UpdateScreen();

	PWM_data[0] = MAX_PWM;

	while(current_cycle < cycle_to_run && current_time < timeout + start_time)
	{
		mini_sprintf((current_time - start_time)/1000, cur_time + 6, 4);
		OLED_DrawString(cur_time, WHITE, 64-strlen(cur_time)*3, 30);
		mini_sprintf(current_temp, temp + 2, 3);
		OLED_DrawString(temp, WHITE, 0, 15);
		OLED_UpdateScreen();

		current_time = HAL_GetTick();
		current_temp = (NTC_Read_Value(&hadc1) + NTC_Read_Value(&hadc1) + NTC_Read_Value(&hadc1))/3;
		if(current_temp > target_temp && PWM_data[0] == MAX_PWM)
		{
			PWM_data[0] = 0;

			if (t_start_cycle != 0) {
				t_period += (HAL_GetTick() - t_start_cycle);
				current_cycle++;
			}
			t_start_cycle = HAL_GetTick();
			mini_sprintf(current_cycle, cycle + 7, 1);
			OLED_DrawString(cycle, WHITE, 66, 45);
		}
		else if(current_temp < target_temp && PWM_data[0] == 0)
		{
			PWM_data[0] = MAX_PWM;
		}

		if (current_temp > peak_max)
		{
			peak_max = current_temp;
			mini_sprintf(peak_max, max_temp + 3, 3);
			OLED_DrawString(max_temp, WHITE, 42, 15);
		}
		if (current_temp < peak_min)
		{
			peak_min = current_temp;
			mini_sprintf(peak_min, min_temp + 3, 3);
			OLED_DrawString(min_temp, WHITE, 90, 15);
		}
		HAL_Delay(100);
	}

	PWM_data[0] = 0;

	t_period = t_period/cycle_to_run;

	//Åström-Hägglund method

	float ampl = (peak_max - peak_min)/2;
	float Ku = (4.0f * MAX_PWM) / (3.14f * ampl);
	float Tu = t_period/1000.0;

	//Ziegler–Nichols method for PI
	PID_auto.Kp = 0.45*Ku;
	PID_auto.Ki = 0.54*Ku/Tu;
	PID_auto.Kd = 0;
	HAL_TIM_PWM_Stop_DMA(&htim1, TIM_CHANNEL_1);

	return PID_auto;
}


void HAL_TIM_PWM_PulseFinishedCallback(TIM_HandleTypeDef *htim)
{
  PWM_cycle++;
}
