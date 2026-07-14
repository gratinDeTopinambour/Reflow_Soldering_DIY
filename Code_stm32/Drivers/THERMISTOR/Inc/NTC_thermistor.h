/*
 * NTC_thermistor.h
 *
 *  Created on: 10 févr. 2026
 *      Author: valentin
 */

#ifndef THERMISTOR_INC_NTC_THERMISTOR_H_
#define THERMISTOR_INC_NTC_THERMISTOR_H_

#include "main.h"

extern const uint32_t thermistor_ohm_n40_40_LUT[];
extern const uint16_t thermistor_ohm_41_230_LUT[];
extern const uint8_t thermistor_ohm_231_300_LUT[];

int NTC_Read_Value(ADC_HandleTypeDef *Rth_ADC);

#endif /* THERMISTOR_INC_NTC_THERMISTOR_H_ */
