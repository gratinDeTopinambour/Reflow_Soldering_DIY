/*
 * OVEN_CTRL.c
 *
 *  Created on: 20 févr. 2026
 *      Author: valentin
 */
#include "OVEN_CTRL.h"
#include "NTC_thermistor.h"

extern ADC_HandleTypeDef hadc1;

TuneState tune_state = TUNE_IDLE;
CtrlState ctrl_state = CTRL_IDLE;
ReflowState reflow_state = REFLOW_IDLE;

#define SN42BI58 {100, 90, 138, 100, 170, 60}
#define SAC305 {0, 0, 0, 0, 0, 0}
#define SN63PB37 {0, 0, 0, 0, 0, 0}
#define MANUAL {0, 0, 0, 0, 0, 0}
ReflowSetting RS[4] = {SN42BI58, SAC305, SN63PB37, MANUAL};

uint16_t tune_temp_initial = 0;
uint16_t tune_temp_peak = 0;
uint16_t tune_temp_slope_start = 0;
uint16_t tune_temp_cutoff = 0;
uint32_t tune_t_start = 0;
uint32_t tune_t_slope_start = 0;
uint8_t  tune_cooling_count = 0;

uint32_t ctrl_timeout = 600000;
uint32_t ctrl_start_timer = 0;
uint32_t ctrl_heat_start_timer = 0;
uint16_t ctrl_start_temp = 0;
uint16_t ctrl_predicted_temp = 0;
uint32_t ctrl_heat_timer = 0;
int ctrl_pred_heat_timer = 0;
uint8_t ctrl_temp_increase = 0;
uint16_t ctrl_temp_rise = 0;
uint8_t ctrl_L_update = 0;
uint16_t peak_temp_value = 0;

uint32_t reflow_timer_start = 0;
uint32_t reflow_timer_soak = 0;
uint16_t reflow_peak_temp = 0;
uint32_t reflow_peak_time = 0;




void OVEN_CTRL_AUTOTUNE(AppContext *ctx)
{
	switch (tune_state)
	{
	case TUNE_IDLE:
		break;

	case TUNE_START:
		ctx->timer_start_ms = ctx->current_time_ms;
		tune_temp_initial = ctx->current_temp_avg;
		tune_t_start = ctx->current_time_ms;

		HAL_GPIO_WritePin(GPIOA, OVEN, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOA, RED_LED, GPIO_PIN_SET);

		tune_state = TUNE_MEASURE_LAG;
		break;

	case TUNE_MEASURE_LAG:
		// Wait for +2°C rise
		if (ctx->current_temp_avg >= tune_temp_initial + 2)
		{
			// Calculate L
			ctx->settings[ctx->current_conf_index].L = (ctx->current_time_ms - tune_t_start) / 1000;


			// Prepare next step
			tune_t_slope_start = ctx->current_time_ms;
			tune_temp_slope_start = ctx->current_temp_avg;
			tune_state = TUNE_MEASURE_SLOPE;
		}
		break;

	case TUNE_MEASURE_SLOPE:
		// Wait for 80% of target
		if (ctx->current_temp_avg >= (ctx->set_point * 8 / 10))
		{
			uint32_t t_slope_end = ctx->current_time_ms;

			// Calculate v (Prevent division by zero)
			uint32_t duration_sec = (t_slope_end - tune_t_slope_start) / 1000;
			if(duration_sec == 0) duration_sec = 1;

			ctx->settings[ctx->current_conf_index].v = 100 * (ctx->current_temp_avg - tune_temp_slope_start) / duration_sec;

			HAL_GPIO_WritePin(GPIOA, OVEN, GPIO_PIN_RESET); // OVEN OFF
			HAL_GPIO_WritePin(GPIOA, RED_LED, GPIO_PIN_RESET);

			tune_temp_cutoff = ctx->current_temp_avg;
			tune_temp_peak = ctx->current_temp_avg;
			tune_cooling_count = 0;

			tune_state = TUNE_COOLING;
		}
		break;

	case TUNE_COOLING:

		if (ctx->current_temp_avg > tune_temp_peak)
		{
			tune_temp_peak = ctx->current_temp_avg;
			tune_cooling_count = 0; // Reset counter if temp still rising
		}
		else
		{
			tune_cooling_count++;
		}

		// If temp hasn't risen for 10 seconds
		if (tune_cooling_count >= 20)
		{
			ctx->settings[ctx->current_conf_index].overshoot = 100*(tune_temp_peak - tune_temp_initial)/(tune_temp_cutoff - tune_temp_initial) - 100;
			tune_state = TUNE_DONE;
		}
		break;

	case TUNE_DONE:
		// Tuning complete
		HAL_GPIO_WritePin(GPIOA, OVEN, GPIO_PIN_RESET); // Ensure OFF
		HAL_GPIO_WritePin(GPIOA, RED_LED, GPIO_PIN_RESET);
		tune_state = TUNE_IDLE; // Reset FSM
		break;

	default:
		HAL_GPIO_WritePin(GPIOA, OVEN, GPIO_PIN_RESET); // Ensure OFF
		HAL_GPIO_WritePin(GPIOA, RED_LED, GPIO_PIN_RESET);
		tune_state = TUNE_IDLE; // Reset FSM
		break;
	}
}

void OVEN_CTRL_CMD(AppContext *ctx)
{
	uint16_t slope = (ctx->settings[ctx->current_conf_index].v == 0) ? 1 : ctx->settings[ctx->current_conf_index].v;
	uint32_t lag_ms = ctx->settings[ctx->current_conf_index].L * 1000;


	switch(ctrl_state)
	{
	case CTRL_IDLE:
		break;
	case CTRL_START:
		ctrl_start_timer = ctx->current_time_ms;
		ctrl_heat_start_timer = ctx->current_time_ms;
		ctrl_temp_rise = (ctx->set_point - ctx->current_temp_avg);
		ctrl_pred_heat_timer = 0;
		ctrl_start_temp = ctx->current_temp_avg;
		ctrl_L_update = 1;
		if(ctrl_heat_timer -  ctrl_heat_start_timer> 0)
		{
			if(ctrl_temp_rise > 20)
			{
				ctrl_state = CTRL_FAST_HEAT;
			}
			else
			{
				ctrl_heat_timer = ctrl_heat_start_timer + 1000*100*(ctrl_temp_rise)/slope;
				ctrl_state = CTRL_HEAT;
			}
			HAL_GPIO_WritePin(GPIOA, OVEN, GPIO_PIN_SET);
			HAL_GPIO_WritePin(GPIOA, RED_LED, GPIO_PIN_SET);
		}
		else
		{
			ctrl_state = CTRL_COOL;
			HAL_GPIO_WritePin(GPIOA, OVEN, GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOA, RED_LED, GPIO_PIN_RESET);
		}
		break;

	case CTRL_HEAT:
		uint32_t heat_time = ctx->current_time_ms - ctrl_heat_start_timer;
		if (ctx->current_temp_avg > ctrl_start_temp + 1 && ctrl_L_update)
		{
			ctx->settings[ctx->current_conf_index].L = heat_time/1000;
			ctrl_L_update = 0;
		}

		if(heat_time > lag_ms)
		{
			ctrl_predicted_temp = ctrl_start_temp + (heat_time - lag_ms)*slope/(100*1000);
			ctrl_pred_heat_timer = 1000*100*(ctrl_predicted_temp - ctx->current_temp_avg)/slope;
		}
		else
		{
			ctrl_predicted_temp = ctrl_start_temp;
			ctrl_pred_heat_timer = 1000*100*(ctrl_predicted_temp - ctx->current_temp_avg)/slope;
		}

		if(ctx->current_temp_avg > ctx->set_point || ctx->current_time_ms >= ctrl_heat_timer + ctrl_pred_heat_timer)
		{
			ctrl_state = CTRL_COOL;
			HAL_GPIO_WritePin(GPIOA, OVEN, GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOA, RED_LED, GPIO_PIN_RESET);
			peak_temp_value = ctx->current_temp_avg;
			ctrl_temp_increase = 0;
		}

		if(ctx->current_time_ms - ctrl_start_timer > ctrl_timeout)
		{
			ctrl_state = CTRL_DONE;
		}
		break;

	case CTRL_FAST_HEAT:
		if (ctx->current_temp_avg > ctrl_start_temp + 1 && ctrl_L_update)
		{
			ctx->settings[ctx->current_conf_index].L = (ctx->current_time_ms - ctrl_heat_start_timer)/1000;
			ctrl_L_update = 0;
		}

		if((ctx->current_time_ms - ctrl_heat_start_timer) > lag_ms)
		{
			ctrl_predicted_temp = ctrl_start_temp + ((ctx->current_time_ms - ctrl_heat_start_timer) - lag_ms)*slope/(100*1000);
			ctrl_pred_heat_timer = 1000*100*(ctrl_predicted_temp - ctx->current_temp_avg)/slope;
		}
		else
		{
			ctrl_predicted_temp = ctrl_start_temp;
			ctrl_pred_heat_timer = 1000*100*(ctrl_predicted_temp - ctx->current_temp_avg)/slope;
		}

		if(ctx->current_temp_avg > (ctrl_start_temp + ctrl_temp_rise*(100-ctx->settings[ctx->current_conf_index].overshoot)/100))
		{
			ctrl_state = CTRL_COOL;
			HAL_GPIO_WritePin(GPIOA, OVEN, GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOA, RED_LED, GPIO_PIN_RESET);
			peak_temp_value = ctx->current_temp_avg;
			ctrl_temp_increase = 0;
		}

		if(ctx->current_time_ms - ctrl_start_timer > ctrl_timeout)
		{
			ctrl_state = CTRL_DONE;
		}
		break;

	case CTRL_COOL:
		if(ctx->current_temp_avg > peak_temp_value)
		{
			peak_temp_value = ctx->current_temp_avg;
			ctrl_temp_increase = 0;
		}
		else
		{
			ctrl_temp_increase++;
		}

		if(ctx->current_temp_avg < ctx->set_point && ctrl_temp_increase > 2)
		{
			ctrl_start_temp = ctx->current_temp_avg;
			ctrl_heat_start_timer = ctx->current_time_ms;
			ctrl_temp_rise = (ctx->set_point - ctx->current_temp_avg);
			ctrl_heat_timer = ctrl_heat_start_timer + 1000*100*(ctrl_temp_rise)/slope;
			ctrl_pred_heat_timer = 0;
			ctrl_L_update = 0;

			if(ctrl_temp_rise > 20)
			{
				ctrl_state = CTRL_FAST_HEAT;
			}
			else
			{
				ctrl_heat_timer = ctrl_heat_start_timer + 1000*100*(ctrl_temp_rise)/slope;
				ctrl_state = CTRL_HEAT;
			}
			HAL_GPIO_WritePin(GPIOA, OVEN, GPIO_PIN_SET);
			HAL_GPIO_WritePin(GPIOA, RED_LED, GPIO_PIN_SET);
		}

		if(ctx->current_time_ms - ctrl_start_timer > ctrl_timeout)
		{
			ctrl_state = CTRL_DONE;
		}
		break;

	case CTRL_DONE:
		HAL_GPIO_WritePin(GPIOA, OVEN, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOA, RED_LED, GPIO_PIN_RESET);
		ctrl_state = CTRL_IDLE;
		break;

	default:
		HAL_GPIO_WritePin(GPIOA, OVEN, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOA, RED_LED, GPIO_PIN_RESET);
		ctrl_state = CTRL_IDLE;
		break;
	}
}

void OVEN_CTRL_REFLOW(AppContext *ctx)
{
	uint32_t reflow_time_offset = 50; //in seconde

	ctrl_timeout = (RS[ctx->current_profile_index].ph_time + RS[ctx->current_profile_index].sk_time + RS[ctx->current_profile_index].rf_time + reflow_time_offset)*1000;

	if(ctx->current_temp_avg > reflow_peak_temp)
	{
		reflow_peak_temp = ctx->current_temp_avg;
		reflow_peak_time = (ctx->current_time_ms - reflow_timer_start)/1000;
	}

	switch(reflow_state)
	{
	case REFLOW_IDLE:
		break;

	case REFLOW_START:
		ctx->timer_start_ms = ctx->current_time_ms;
		reflow_peak_temp = 0;
		ctx->set_point = RS[ctx->current_profile_index].ph_temp;
		reflow_timer_start = ctx->current_time_ms;
		reflow_state = REFLOW_PREHEAT;
		break;

	case REFLOW_PREHEAT:
		if(ctx->current_temp_avg + 5 >= ctx->set_point)
		{
			reflow_state = REFLOW_SOAK;
			reflow_timer_soak = ctx->current_time_ms;
		}

		if(ctx->current_time_ms - reflow_timer_start > ctrl_timeout)
		{
			reflow_state = REFLOW_DONE;
		}

		break;

	case REFLOW_SOAK:
		if(ctx->current_time_ms - reflow_timer_soak > RS[ctx->current_profile_index].sk_time * 1000)
		{
			reflow_state = REFLOW_REFLOW;
		}
		ctx->set_point = RS[ctx->current_profile_index].ph_temp + (ctx->current_time_ms - reflow_timer_soak)*(RS[ctx->current_profile_index].sk_temp - RS[ctx->current_profile_index].ph_temp)/(1000*RS[ctx->current_profile_index].sk_time);

		if(ctx->current_time_ms - reflow_timer_start > ctrl_timeout)
		{
			reflow_state = REFLOW_DONE;
		}

		break;

	case REFLOW_REFLOW:
		ctx->set_point = RS[ctx->current_profile_index].rf_temp;
		if(ctx->current_temp_avg > ctx->set_point-5){
			ctx->set_point = 20;
			reflow_state = REFLOW_COOLING;
		}

		if(ctx->current_time_ms - reflow_timer_start > ctrl_timeout)
		{
			reflow_state = REFLOW_DONE;
		}

		break;

	case REFLOW_COOLING:
		ctx->set_point = 20;
		if(ctx->current_temp_avg < RS[ctx->current_profile_index].sk_temp){
			reflow_state = REFLOW_DONE;
		}

		if(ctx->current_time_ms - reflow_timer_start > ctrl_timeout)
		{
			reflow_state = REFLOW_DONE;
		}

		break;

	case REFLOW_DONE:
		ctx->set_point = 20;
		reflow_state = REFLOW_IDLE;
		break;

	default:
		ctx->set_point = 20;
		reflow_state = REFLOW_IDLE;
		break;
	}

}

OvenError OVEN_CTRL_ERROR(AppContext *ctx)
{
	if(ctx->settings[ctx->current_conf_index].v == 0) return OVEN_ERROR_BB_ZERO;
	if(ctx->settings[ctx->current_conf_index].overshoot == 0) return OVEN_ERROR_BB_ZERO;

	if(ctx->settings[ctx->current_conf_index].L > 30) return OVEN_ERROR_BB_MAX;
	if(ctx->settings[ctx->current_conf_index].v > 10) return OVEN_ERROR_BB_MAX;
	if(ctx->settings[ctx->current_conf_index].overshoot > 100) return OVEN_ERROR_BB_MAX;

	if(RS[ctx->current_profile_index].ph_temp == 0) return OVEN_ERROR_RS_ZERO;
	if(RS[ctx->current_profile_index].rf_temp == 0) return OVEN_ERROR_RS_ZERO;
	if(RS[ctx->current_profile_index].sk_temp == 0) return OVEN_ERROR_RS_ZERO;
	if(RS[ctx->current_profile_index].ph_time == 0) return OVEN_ERROR_RS_ZERO;
	if(RS[ctx->current_profile_index].rf_time == 0) return OVEN_ERROR_RS_ZERO;
	if(RS[ctx->current_profile_index].sk_time == 0) return OVEN_ERROR_RS_ZERO;

	if(RS[ctx->current_profile_index].ph_temp > MAX_TEMP) return OVEN_ERROR_RS_MAX;
	if(RS[ctx->current_profile_index].rf_temp > MAX_TEMP) return OVEN_ERROR_RS_MAX;
	if(RS[ctx->current_profile_index].sk_temp > MAX_TEMP) return OVEN_ERROR_RS_MAX;
	if(RS[ctx->current_profile_index].ph_time + RS[ctx->current_profile_index].rf_time + RS[ctx->current_profile_index].sk_time > MAX_DELAY) return OVEN_ERROR_RS_MAX;

	return OVEN_ERROR_NONE;
}



