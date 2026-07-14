/*
 * app_state.h
 *
 *  Created on: 28 févr. 2026
 *      Author: valentin
 */

#ifndef OLED_096_INC_APP_STATE_H_
#define OLED_096_INC_APP_STATE_H_

#include <stdint.h>

typedef struct {
    uint16_t L;              // Retard (dead time) en s
    uint16_t v;              // Vitesse de montée (°C/s) *100
    uint16_t overshoot;      // Overshoot total mesuré
} BangBangSetting;

// System States
typedef enum {
    STATE_IDLE,
    STATE_REFLOW,
    STATE_AUTOTUNE,
    STATE_ERROR
} SystemState;

typedef enum {
    PROFILE_SN42BI58,
    PROFILE_SAC305,
    PROFILE_SN63PB37,
    PROFILE_MANUAL
} ReflowProfileType;


typedef struct {
    // Sensor Data
    uint16_t current_temp_avg;
    uint32_t current_time_ms;
    uint32_t timer_start_ms;

    // Control Targets
    uint16_t set_point;
    SystemState system_state;

    // Active Settings
    uint8_t current_conf_index; // 0-3
    ReflowProfileType current_profile_index;
    BangBangSetting settings[4]; // Loaded from EEPROM
} AppContext;


#endif /* OLED_096_INC_APP_STATE_H_ */
