/*
 * interface.c
 *
 *  Created on: 14 févr. 2026
 *      Author: valentin
 */


#include "interface.h"
#include "OLED_096.h"
#include "EEPROM_EMUL.h"
#include "OVEN_CTRL.h"

extern ADC_HandleTypeDef hadc1;

extern TuneState tune_state;
extern CtrlState ctrl_state;
extern ReflowState reflow_state;
extern uint16_t reflow_peak_temp;
extern uint32_t reflow_peak_time;

Menu oled_menu;
uint8_t update_oled_screen;
uint8_t update_oled_menu;

BB_sel BBsel = L_sel;

char version[5] = "v0.0";
char L_str[7] = "L: 999";
char v_str[9] = "v: 99.99";
char ov_str[9] = "ov: 100%";
char nb1[2] = "0";
char nb2[2] = "0";
char nb3[2] = "0";
char nb4[2] = "0";
char temp_str[8] = "t*: 999";
//char predic_temp_str[9] = "Prt* 999";
char SP_str[8] = "SP: 999";
char time_str[10] = "time: 999";
char tune_step_str[6] = "IDLE ";
const char* tune_step_table[6] = {"IDLE", "START", "LAG", "SLOPE", "COOL", "DONE"};
char ctrl_step_str[10] = "IDLE";
const char* ctrl_step_table[6] = {"IDLE", "START", "HEAT", "FAST_HEAT",  "COOL", "DONE"};
char reflow_step_str[8] = "PREHEAT";
const char* reflow_step_table[7] = {"IDLE", "START", "PREHEAT", "SOAK", "REFLOW", "COOLING", "DONE"};
char reflow_time_str[18] = "time to peak 999s";
char reflow_peak_str[16] = "peak temp 999*C";

char* start_labels[3] = {version, "made by :", "V.Lefebvre"};
char* config_labels[4] = {"Config 1", "Config 2", "Config 3", "Config 4"};
char* temp_prof_labels[4] = {"Sn42Bi58", "SAC305", "Sn63Pb37", "Manual"};
char* auto_tune_labels[6] = {temp_str, time_str, L_str, v_str, ov_str, tune_step_str};
char* config_menu_labels[6] = {"AutoTune", L_str, "Reset", v_str, "Reflow", temp_str};
char* config_set_labels[4] = {nb1, nb2, nb3, nb4};
char* reflow_labels[5] = {temp_str, SP_str, time_str, ctrl_step_str, reflow_step_str};
char* reflow_end_labels[3] = {temp_str, reflow_time_str, reflow_peak_str};

uint8_t conf_digit = 0;

void Update_Menu(Menu* up_menu)
{
	uint8_t y_pos = 16;
	uint8_t x_pos = 10;

	OLED_Fill(BLACK);
	OLED_DrawString(up_menu->title, WHITE, 64-strlen(up_menu->title)*3, 2);
	OLED_DrawLine(WHITE, 20, 13, 107, 13);

	if(up_menu->cur_screen == START)
	{
		OLED_DrawString(up_menu->labels[0], WHITE, 64-strlen(up_menu->labels[0])*3, 17);
		OLED_DrawString(up_menu->labels[1], WHITE, 64-strlen(up_menu->labels[1])*3, 32);
		OLED_DrawString(up_menu->labels[2], WHITE, 64-strlen(up_menu->labels[2])*3, 47);
	}
	else if(up_menu->cur_screen == START_AUTO_CONFIG)
	{
		OLED_DrawString("Prgm oven: max 150*C", WHITE, 2, 31);
		OLED_DrawString("The Auto Config will run for around 10min", WHITE, 2, 42);
	}
	else if(up_menu->cur_screen == NOT_IMPLEMENTED)
	{
		OLED_DrawString("Functionality not", WHITE, 64-strlen("Functionality not")*3, 31);
		OLED_DrawString("implemented yet", WHITE, 64-strlen("implemented yet")*3, 41);
	}
	else if(up_menu->cur_screen == CONFIG_SET)
	{
		x_pos = 42;
		y_pos = 20;
		for(uint8_t i = up_menu->num_labels; i > 0; i--)
		{
			if(up_menu->label_sel == i-1)
			{
				OLED_BoxString(up_menu->labels[i-1], BLACK, x_pos + 2, y_pos + 2, x_pos, y_pos, 8, 10);
			}
			else
			{
				OLED_BoxString(up_menu->labels[i-1], WHITE, x_pos + 2, y_pos + 2, x_pos, y_pos, 8, 10);
			}
			x_pos += 12;
		}
	}
	else if(up_menu->num_labels <= 3)
	{
		x_pos = 10;
		y_pos = 16;
		for(uint8_t i = 0; i < up_menu->num_labels; i++)
		{
			if(up_menu->label_sel == i)
			{
				OLED_BoxString(up_menu->labels[i], BLACK, 64-strlen(up_menu->labels[i])*3, y_pos + 3, x_pos, y_pos, 128-20, 13);
			}
			else
			{
				OLED_BoxString(up_menu->labels[i], WHITE, 64-strlen(up_menu->labels[i])*3, y_pos + 3, x_pos, y_pos, 128-20, 13);
			}
			y_pos += 17;
		}
	}
	else
	{
		y_pos = 16;
		for(uint8_t i = 0; i < up_menu->num_labels; i++)
		{
			x_pos = (i%2) ? 66 : 6;
			if(up_menu->label_sel == i)
			{
				OLED_BoxString(up_menu->labels[i], BLACK, x_pos + 28 -strlen(up_menu->labels[i])*3, y_pos + 3, x_pos, y_pos, 56, 13);
			}
			else
			{
				OLED_BoxString(up_menu->labels[i], WHITE, x_pos + 28 -strlen(up_menu->labels[i])*3, y_pos + 3, x_pos, y_pos, 56, 13);
			}
			y_pos += (i%2) ? 17 : 0;
		}
	}

	OLED_UpdateScreen();
	update_oled_screen = 0;
}

void Change_Menu(Menu* up_menu, AppContext * ctx)
{
	static char title_str[17];
	uint16_t nb = 0;

	switch (up_menu->cur_screen)
	{
	case START:
		up_menu->label_sel = 99;
		up_menu->labels = start_labels;
		up_menu->num_labels = 3;
		up_menu->next_screen = CONFIG;
		up_menu->prev_screen = START;
		up_menu->title = "Reflow-oven";
		break;

	case CONFIG:
		up_menu->label_sel = 0;
		up_menu->labels = config_labels;
		up_menu->num_labels = 4;
		up_menu->next_screen = CONFIG_MENU;
		up_menu->prev_screen = START;
		up_menu->title = "Config";
		break;

	case CONFIG_MENU:
		strcpy(title_str, "Config ");
		mini_sprintf(ctx->current_conf_index + 1, title_str + 7, 1, ' ', 1);

		BangBangSetting_Update(ctx->settings[ctx->current_conf_index]);

		up_menu->label_sel = 0;
		up_menu->labels = config_menu_labels;
		up_menu->num_labels = 6;
		up_menu->next_screen = TEMP_PROFIL;
		up_menu->prev_screen = CONFIG;
		up_menu->title = title_str;
		break;

	case CONFIG_SET:
		if(up_menu->label_sel == 1)
		{
			BBsel = L_sel;
			strcpy(title_str, "L manual tuning");
			nb = ctx->settings[ctx->current_conf_index].L;
		}
		else if(up_menu->label_sel == 3)
		{
			BBsel = v_sel;
			strcpy(title_str, "v manual tuning");
			nb = ctx->settings[ctx->current_conf_index].v;
		}
		else if(up_menu->label_sel == 5)
		{
			BBsel = ov_sel;
			strcpy(title_str, "ov manual tuning");
			nb = ctx->settings[ctx->current_conf_index].overshoot;
		}
		for(uint8_t i = 0; i < 4; i++)
		{
			config_set_labels[i][0] = (nb%10) + '0';
			nb /= 10;
		}

		up_menu->label_sel = 0;
		up_menu->labels = config_set_labels;
		up_menu->num_labels = 4;
		up_menu->next_screen = CONFIG_MENU;
		up_menu->prev_screen = CONFIG_MENU;
		up_menu->title = title_str;
		break;

	case TEMP_PROFIL:
		up_menu->label_sel = 0;
		up_menu->labels = temp_prof_labels;
		up_menu->num_labels = 4;
		up_menu->next_screen = REFLOW;
		up_menu->prev_screen = CONFIG_MENU;
		up_menu->title = "Reflow profile";
		break;

	case REFLOW:
		ctx->system_state = STATE_REFLOW;
		ctrl_state = CTRL_START;
		reflow_state = REFLOW_START;
		CtrlStep_Update(ctrl_state);
		ReflowStep_Update(reflow_state);
		ctx->set_point = 20;

		SetPoint_Update(ctx->set_point);

		up_menu->label_sel = 99;
		up_menu->labels = reflow_labels;
		up_menu->num_labels = 5;
		up_menu->next_screen = REFLOW_END;
		up_menu->prev_screen = TEMP_PROFIL;
		up_menu->title = "Reflow";
		break;

	case REFLOW_END:

		mini_sprintf(reflow_peak_temp, reflow_peak_str + 10, 3, ' ', 0);
		mini_sprintf(reflow_peak_time, reflow_time_str + 13, 3, ' ', 0);

		up_menu->label_sel = 99;
		up_menu->labels = reflow_end_labels;
		up_menu->num_labels = 3;
		up_menu->next_screen = CONFIG_MENU;
		up_menu->prev_screen = TEMP_PROFIL;
		up_menu->title = "Open oven door";
		break;

	case START_AUTO_CONFIG:
		up_menu->label_sel = 0;
		up_menu->labels = NULL;
		up_menu->num_labels = 0;
		up_menu->next_screen = AUTO_CONFIG;
		up_menu->prev_screen = CONFIG_MENU;
		up_menu->title = " Auto tuning start ";
		break;

	case AUTO_CONFIG:
		ctx->system_state = STATE_AUTOTUNE;
		tune_state = TUNE_START;
		TuneStep_Update(tune_state);
		ctx->timer_start_ms = ctx->current_time_ms;
		ctx->set_point = 100;

		up_menu->label_sel = 99;
		up_menu->labels = auto_tune_labels;
		up_menu->num_labels = 6;
		up_menu->next_screen = CONFIG_MENU;
		up_menu->prev_screen = CONFIG_MENU;
		up_menu->title = "Auto tuning";
		break;

	case NOT_IMPLEMENTED:
		up_menu->label_sel = 99;
		up_menu->labels = NULL;
		up_menu->num_labels = 0;
		up_menu->next_screen = up_menu->prev_screen;
		up_menu->title = "NOT IMPLEMENTED";
		break;

	default:
		up_menu->label_sel = 0;
		up_menu->labels = start_labels;
		up_menu->num_labels = 3;
		up_menu->next_screen = CONFIG;
		up_menu->prev_screen = START;
		up_menu->title = "Reflow-oven";
		break;
	}


	update_oled_menu = 0;
	update_oled_screen = 1;
}

void Button_Update(Menu* up_menu, AppContext* ctx)
{
	if (!HAL_GPIO_ReadPin(GPIOA, VALID))
	{
		if(up_menu->cur_screen == CONFIG)
		{
			ctx->current_conf_index = up_menu->label_sel;
		}
		else if(up_menu->cur_screen == CONFIG_MENU)
		{
			if(up_menu->label_sel % 2)
			{
				up_menu->next_screen = CONFIG_SET;
			}
			else if(up_menu->label_sel == 0)
			{
				up_menu->next_screen = START_AUTO_CONFIG;
			}
			else if(up_menu->label_sel == 2)
			{
				up_menu->next_screen = CONFIG_MENU;
				ctx->settings[ctx->current_conf_index].L = 1;
				ctx->settings[ctx->current_conf_index].v = 1;
				ctx->settings[ctx->current_conf_index].overshoot = 20;

			}
			else
			{
				up_menu->next_screen = TEMP_PROFIL;
			}

		}
		else if(up_menu->cur_screen == CONFIG_SET)
		{
			uint16_t final_val = 0;
			for(uint8_t i = up_menu->num_labels; i > 0; i--) {
				final_val = (final_val * 10) + (up_menu->labels[i-1][0] - '0');
			}

			if(BBsel == L_sel)        ctx->settings[ctx->current_conf_index].L = final_val%1000;
			else if(BBsel == v_sel)   ctx->settings[ctx->current_conf_index].v = final_val;
			else if(BBsel == ov_sel)  ctx->settings[ctx->current_conf_index].overshoot = final_val%100;
		}
		else if(up_menu->cur_screen == TEMP_PROFIL)
		{
			ctx->current_profile_index = up_menu->label_sel;
			if(ctx->current_profile_index == PROFILE_SAC305 || ctx->current_profile_index == PROFILE_SN63PB37 || ctx->current_profile_index == PROFILE_MANUAL)
			{
				up_menu->next_screen = NOT_IMPLEMENTED;
			}
		}
		else if(up_menu->cur_screen == AUTO_CONFIG)
		{
			if(tune_state == TUNE_IDLE)
			{
				EE_Write(ctx->settings);
			}

			HAL_GPIO_WritePin(GPIOA, OVEN, GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOA, RED_LED, GPIO_PIN_RESET);
			tune_state = TUNE_IDLE;
		}
		else if(up_menu->cur_screen == REFLOW)
		{

			HAL_GPIO_WritePin(GPIOA, OVEN, GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOA, RED_LED, GPIO_PIN_RESET);
			reflow_state = REFLOW_IDLE;
			ctrl_state = CTRL_IDLE;
		}


		ctx->system_state = STATE_IDLE;
		up_menu->cur_screen = up_menu->next_screen;
		update_oled_menu = 1;
		HAL_Delay(250);
		return;
	}

	if (!HAL_GPIO_ReadPin(GPIOA, RETURN)) {

		if(up_menu->cur_screen == AUTO_CONFIG)
		{
			HAL_GPIO_WritePin(GPIOA, OVEN, GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOA, RED_LED, GPIO_PIN_RESET);
			tune_state = TUNE_IDLE;
		}
		else if(up_menu->cur_screen == REFLOW)
		{

			HAL_GPIO_WritePin(GPIOA, OVEN, GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOA, RED_LED, GPIO_PIN_RESET);
			reflow_state = REFLOW_IDLE;
			ctrl_state = CTRL_IDLE;
		}

		up_menu->cur_screen = up_menu->prev_screen;
		update_oled_menu = 1;
		HAL_Delay(250);
		return;
	}

	if(up_menu->cur_screen == AUTO_CONFIG || up_menu->cur_screen == START || up_menu->cur_screen == REFLOW || up_menu->cur_screen == REFLOW_END)
	{
		// nothing should be read in those state
	}
	else if(up_menu->cur_screen == CONFIG_SET)
	{
		if (!HAL_GPIO_ReadPin(GPIOA, UP))
		{
			char c = up_menu->labels[up_menu->label_sel][0];
			up_menu->labels[up_menu->label_sel][0] = (c == '9') ? '0' : c + 1;
			update_oled_screen = 1;
			HAL_Delay(250);
		}
		else if (!HAL_GPIO_ReadPin(GPIOA, DOWN))
		{
			char c = up_menu->labels[up_menu->label_sel][0];
			up_menu->labels[up_menu->label_sel][0] = (c == '0') ? '9' : c - 1;
			update_oled_screen = 1;
			HAL_Delay(250);
		}
		else if (!HAL_GPIO_ReadPin(GPIOA, RIGHT)) {
			up_menu->label_sel = (up_menu->label_sel == 0) ? 3 : up_menu->label_sel - 1;
			conf_digit = (uint8_t)(up_menu->labels[up_menu->label_sel] - "0");
			update_oled_screen = 1;
			HAL_Delay(250);
		}
		else if(!HAL_GPIO_ReadPin(GPIOA, LEFT)) {
			up_menu->label_sel = (up_menu->label_sel >= 3) ? 0 : up_menu->label_sel + 1;
			conf_digit = (uint8_t)(up_menu->labels[up_menu->label_sel] - "0");
			update_oled_screen = 1;
			HAL_Delay(250);
		}

	}
	else if(up_menu->num_labels <= 3)
	{
		if (!HAL_GPIO_ReadPin(GPIOA, UP)) {
			up_menu->label_sel = (up_menu->label_sel == 0) ? up_menu->num_labels - 1 : up_menu->label_sel - 1;
			update_oled_screen = 1;
			HAL_Delay(250);
		}
		else if (!HAL_GPIO_ReadPin(GPIOA, DOWN)) {
			up_menu->label_sel = (up_menu->label_sel >= up_menu->num_labels - 1) ? 0 : up_menu->label_sel + 1;
			update_oled_screen = 1;
			HAL_Delay(250);
		}
	}
	else
	{
		if (!HAL_GPIO_ReadPin(GPIOA, UP))
		{
			up_menu->label_sel = (up_menu->label_sel < 2) ? up_menu->num_labels + up_menu->label_sel - 2 : up_menu->label_sel - 2;
			update_oled_screen = 1;
			HAL_Delay(250);
		}
		else if (!HAL_GPIO_ReadPin(GPIOA, DOWN))
		{
			up_menu->label_sel = (up_menu->label_sel + 2 >= up_menu->num_labels) ? up_menu->label_sel + 2 - up_menu->num_labels : up_menu->label_sel + 2;
			update_oled_screen = 1;
			HAL_Delay(250);
		}
		else if (!HAL_GPIO_ReadPin(GPIOA, RIGHT) || !HAL_GPIO_ReadPin(GPIOA, LEFT)) {
			up_menu->label_sel = (up_menu->label_sel % 2) ? up_menu->label_sel - 1 : up_menu->label_sel + 1;
			update_oled_screen = 1;
			HAL_Delay(250);
		}
	}
}

void Temp_Update(uint16_t temp)
{
	mini_sprintf(temp, temp_str + 4, 3, ' ', 1);
	update_oled_screen = 1;
}

void Time_Update(uint32_t time)
{
	mini_sprintf(time, time_str + 6, 3, ' ', 1);
	update_oled_screen = 1;
}

void SetPoint_Update(uint16_t set_point)
{
	mini_sprintf(set_point, SP_str + 4, 3, ' ', 1);
	update_oled_screen = 1;
}

void BangBangSetting_Update(BangBangSetting settings)
{
	mini_sprintf(settings.L, L_str + 3, 3, ' ', 1);
	mini_sprintf(settings.v/100, v_str + 3, 2, ' ', 0);
	mini_sprintf(settings.v%100, v_str + 6, 2, '0', 1);
	mini_sprintf(settings.overshoot, ov_str + 4, 3, ' ', 0);
	update_oled_screen = 1;
}

void TuneStep_Update(TuneState tune_state)
{
	strcpy(tune_step_str, tune_step_table[tune_state]);
	update_oled_screen = 1;
}

void CtrlStep_Update(CtrlState ctrl_state)
{
	strcpy(ctrl_step_str, ctrl_step_table[ctrl_state]);
	update_oled_screen = 1;
}

void ReflowStep_Update(ReflowState reflow_state)
{
	strcpy(reflow_step_str, reflow_step_table[reflow_state]);
	update_oled_screen = 1;
}

void interface_init(Menu* up_menu)
{
	tune_state = TUNE_IDLE;
	ctrl_state = CTRL_IDLE;
	reflow_state = REFLOW_IDLE;

	up_menu->label_sel = 0;
	up_menu->labels = start_labels;
	up_menu->num_labels = 3;
	up_menu->next_screen = CONFIG;
	up_menu->prev_screen = START;
	up_menu->title = "Reflow-oven";

	update_oled_menu = 1;
}

// Optimized: Converts int to string without using the heavy sprintf library
void mini_sprintf(int n, char *s, uint8_t width, char pad, uint8_t add_terminator)
{
	if(add_terminator)
	{
		s[width] = '\0';
	}
    // 1. Keep track of the sign and use the absolute value
    uint8_t is_negative = (n < 0);
    unsigned int num = is_negative ? -n : n;

    for (int8_t i = width - 1; i >= 0; i--) {
        // 2. Fill digits using the absolute value
        if (num > 0 || (i == width - 1 && num == 0)) {
            s[i] = (num % 10) + '0';
            num /= 10;
        }
        // 3. Place the negative sign once digits are done
        else if (is_negative) {
            s[i] = '-';
            is_negative = 0; // Sign is placed, don't place it again
        }
        // 4. Fill remaining space with padding
        else {
            s[i] = pad;
        }
    }
}
