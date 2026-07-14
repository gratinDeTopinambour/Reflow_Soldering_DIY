/*
 * EEPROM_EMUL.c
 *
 *  Created on: 27 févr. 2026
 *      Author: valentin
 */

#include "EEPROM_EMUL.h"

void EE_Read(BangBangSetting *data)
{
    uint8_t *ptr = (uint8_t*)EE_PAGE_ADDR;
    int latest_slot = -1;

    for (int i = 0; i < MAX_SLOTS; i++) {
        // Check if the first 8 bytes of the slot are 0xFF (Empty)
        if (*(uint64_t*)(ptr + (i * DATA_SIZE)) == 0xFFFFFFFFFFFFFFFF) {
            break;
        }
        latest_slot = i;
    }

    if (latest_slot == -1) {
        // No data found (Fresh chip). Set defaults.
        memset(data, 0, DATA_SIZE);
    } else {
        memcpy(data, (ptr + (latest_slot * DATA_SIZE)), DATA_SIZE);
    }
}

void EE_Write(BangBangSetting *data)
{
	uint64_t buffer[3];
	memcpy(buffer, data, sizeof(uint64_t) * 3);

    uint8_t *ptr = (uint8_t*)EE_PAGE_ADDR;
    int next_slot = 0;

    // 1. Find the first empty slot
    for (int i = 0; i < MAX_SLOTS; i++) {
        if (*(uint64_t*)(ptr + (i * DATA_SIZE)) == 0xFFFFFFFFFFFFFFFF) {
            next_slot = i;
            break;
        }
        if (i == MAX_SLOTS - 1) next_slot = -1; // Page is full
    }

    __disable_irq();

    HAL_FLASH_Unlock();

    __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR | FLASH_FLAG_PGAERR | FLASH_FLAG_PGSERR);

    // 2. If page is full, erase and reset to slot 0
    if (next_slot == -1) {
        FLASH_EraseInitTypeDef EraseInitStruct;
        uint32_t PageError;
        EraseInitStruct.TypeErase = FLASH_TYPEERASE_PAGES;
        EraseInitStruct.Page = EE_PAGE_INDEX;
        EraseInitStruct.NbPages = 1;
        HAL_FLASHEx_Erase(&EraseInitStruct, &PageError);
        next_slot = 0;
    }

    uint32_t target_addr = EE_PAGE_ADDR + (next_slot * DATA_SIZE);

    for (int j = 0; j < 3; j++) {
        HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, target_addr, buffer[j]);
        target_addr += 8;
    }

    HAL_FLASH_Lock();

    __enable_irq();
}
