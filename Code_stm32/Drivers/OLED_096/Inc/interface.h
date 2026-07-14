/*
 * interface.h
 *
 *  Created on: 14 févr. 2026
 *      Author: valentin
 */

#ifndef OLED_096_INC_INTERFACE_H_
#define OLED_096_INC_INTERFACE_H_

#include <string.h>
#include "main.h"
#include "app_state.h"
#include "OVEN_CTRL.h"

#define RETURN GPIO_PIN_3
#define VALID GPIO_PIN_2
#define UP GPIO_PIN_4
#define DOWN GPIO_PIN_5
#define LEFT GPIO_PIN_6
#define RIGHT GPIO_PIN_7

typedef enum
{
	START,
	CONFIG,
	CONFIG_MENU,
	CONFIG_SET,
	TEMP_PROFIL,
	REFLOW,
	REFLOW_END,
	START_AUTO_CONFIG,
	AUTO_CONFIG,
	NOT_IMPLEMENTED,
}Screen;

typedef enum
{
	L_sel,
	v_sel,
	ov_sel,
}BB_sel;

typedef struct
{
	const char* title;
	char** labels;
	uint8_t num_labels;
	uint8_t label_sel;
	Screen cur_screen;
	Screen next_screen;
	Screen prev_screen;
} Menu;



void Update_Menu(Menu* up_menu);
void Button_Update(Menu* up_menu, AppContext* ctx);
void Change_Menu(Menu* up_menu, AppContext* ctx);
void interface_init(Menu* up_menu);

void Temp_Update(uint16_t temp);
void Time_Update(uint32_t time);
void SetPoint_Update(uint16_t set_point);
void BangBangSetting_Update(BangBangSetting settings);
void TuneStep_Update(TuneState tune_state);
void CtrlStep_Update(CtrlState ctrl_state);
void ReflowStep_Update(ReflowState reflow_state);

void mini_sprintf(int n, char *s, uint8_t width, char pad, uint8_t add_terminator);

#endif /* OLED_096_INC_INTERFACE_H_ */
