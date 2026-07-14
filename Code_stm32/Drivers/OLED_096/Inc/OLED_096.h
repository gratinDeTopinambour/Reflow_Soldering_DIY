/*
 * OLED_096.h
 *
 *  Created on: Feb 7, 2026
 *      Author: valentin
 */

#ifndef OLED_096_H_
#define OLED_096_H_

#include "main.h"
#include "stdlib.h"
#include "string.h"
#include "pixel_font.h"

#define OLED_096_ADDR 0x78

#define OLED_096_WIDTH 128
#define OLED_096_HEIGHT 64

#define WHITE 1
#define BLACK 0

#define OLED_CONTRAST 0x81
#define OLED_ENTIRE_DISPLAY_ON_RAM 0xA4
#define OLED_ENTIRE_DISPLAY_ON 0xA5
#define OLED_NORMAL_DISPLAY 0xA6
#define OLED_INVERSE_DISPLAY 0xA7
#define OLED_DISPLAY_OFF 0xAE
#define OLED_DISPLAY_ON 0xAF

#define OLED_RIGHT_HOR_SCROLL 0x26
#define OLED_LEFT_HOR_SCROLL 0x27
#define OLED_VER_RIGHT_HOR_SCROLL 0x29
#define OLED_VER_LEFT_HOR_SCROLL 0x2A
#define OLED_STOP_SCROLL 0x2E
#define OLED_START_SCROLL 0x2F

#define OLED_VER_SCROLL_AREA 0xA3

#define OLED_LOW_COL_ADDR 0x00
#define OLED_HIGH_COL_ADDR 0x10
#define OLED_MEM_ADDR_MODE 0x20
#define OLED_COL_ADDR 0x21
#define OLED_PAGE_ADDR 0x22
#define OLED_PAGE_START 0xB0

#define OLED_DISPLAY_START_LINE 0x40
#define OLED_SEGMENT_REMAP_0 0xA1
#define OLED_SEGMENT_REMAP_127 0xA2
#define OLED_MUX_RATION 0xA8
#define OLED_COM_SCAN_DIR_NORMAL 0xC0
#define OLED_COM_SCAN_DIR_INV 0xC8
#define OLED_DISPLAY_OFFSET 0xD3
#define OLED_COM_PIN 0xDA

#define OLED_CLOCK_DIV_RATIO 0xD5
#define OLED_PRE_CHARGE_PERIOD 0xD9
#define OLED_VCOM_LEVEL 0xDB
#define OLED_NOP 0xE3

extern uint8_t OLED_Buffer[OLED_096_WIDTH * OLED_096_HEIGHT / 8];

void OLED_Write(uint8_t reg, uint8_t data);
void OLED_Write_nByte(uint8_t reg, uint8_t* data, uint8_t len);
void OLED_Read_nByte(uint8_t addr, uint8_t *buffer, uint32_t len);
int OLED_Init(void);
void OLED_UpdateScreen(void);
void OLED_Fill(uint8_t color);
int OLED_DrawPixel(uint8_t color, uint16_t x, uint16_t y);
int OLED_DrawLine(uint8_t color, uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);
int OLED_DrawRect(uint8_t color, uint16_t x, uint16_t y, uint16_t w, uint16_t h);
int OLED_FillRect(uint8_t color, uint16_t x, uint16_t y, uint16_t w, uint16_t h);
void OLED_DrawChar(char ch, uint8_t color, uint16_t x, uint16_t y);
void OLED_DrawString(const char* str, uint8_t color, uint16_t x, uint16_t y);
void OLED_BoxString(const char* str, uint8_t color, uint16_t x, uint16_t y, uint16_t xr, uint16_t yr, uint16_t w, uint16_t h);
void OLED_DrawGraph(uint16_t* graph, uint16_t size_graph, uint8_t color, uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);


#endif /* SSD1306_INC_OLED_096_H_ */
