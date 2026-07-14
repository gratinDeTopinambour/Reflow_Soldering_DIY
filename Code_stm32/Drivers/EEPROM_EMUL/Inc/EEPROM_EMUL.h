/*
 * EEPROM_EMUL.h
 *
 *  Created on: 27 févr. 2026
 *      Author: valentin
 */

#ifndef EEPROM_EMUL_INC_EEPROM_EMUL_H_
#define EEPROM_EMUL_INC_EEPROM_EMUL_H_

#include <string.h>
#include "main.h"
#include "app_state.h"


#define EE_PAGE_ADDR    (FLASH_BASE + FLASH_SIZE - FLASH_PAGE_SIZE)
#define EE_PAGE_INDEX   ((EE_PAGE_ADDR - FLASH_BASE) / FLASH_PAGE_SIZE)
#define DATA_SIZE          sizeof(BangBangSetting) * 4 // 24 bytes
#define MAX_SLOTS          (FLASH_PAGE_SIZE / DATA_SIZE)

void EE_Read(BangBangSetting *data);
void EE_Write(BangBangSetting *data);

#endif /* EEPROM_EMUL_INC_EEPROM_EMUL_H_ */
