/*
 * OVEN_CTRL.h
 *
 *  Created on: 20 févr. 2026
 *      Author: valentin
 */

#ifndef OVEN_CTRL_INC_OVEN_CTRL_H_
#define OVEN_CTRL_INC_OVEN_CTRL_H_

#include <string.h>
#include "main.h"
#include "app_state.h"


#define OVEN GPIO_PIN_8
#define RED_LED GPIO_PIN_11

#define MAX_TEMP 270 //270°C
#define MAX_DELAY 600 //10min

typedef struct {
	uint16_t ph_temp;
	uint16_t ph_time;
	uint16_t sk_temp;
	uint16_t sk_time;
	uint16_t rf_temp;
	uint16_t rf_time;
}ReflowSetting;

typedef enum {
    TUNE_IDLE,
    TUNE_START,
    TUNE_MEASURE_LAG,
    TUNE_MEASURE_SLOPE,
    TUNE_COOLING,
    TUNE_DONE
} TuneState;

typedef enum {
    CTRL_IDLE,
    CTRL_START,
    CTRL_HEAT,
	CTRL_FAST_HEAT,
    CTRL_COOL,
    CTRL_DONE
} CtrlState;

typedef enum{
	REFLOW_IDLE,
	REFLOW_START,
	REFLOW_PREHEAT,
	REFLOW_SOAK,
	REFLOW_REFLOW,
	REFLOW_COOLING,
	REFLOW_DONE
}ReflowState;

typedef enum{
	OVEN_ERROR_NONE,
	OVEN_ERROR_BB_ZERO,
	OVEN_ERROR_RS_ZERO,
	OVEN_ERROR_BB_MAX,
	OVEN_ERROR_RS_MAX
}OvenError;

void OVEN_CTRL_AUTOTUNE(AppContext *ctx);
void OVEN_CTRL_CMD(AppContext *ctx);
void OVEN_CTRL_REFLOW(AppContext *ctx);
OvenError OVEN_CTRL_ERROR(AppContext *ctx);

#endif /* OVEN_CTRL_INC_OVEN_CTRL_H_ */
