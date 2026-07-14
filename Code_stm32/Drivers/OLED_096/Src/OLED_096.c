/*
 * OLED_096.c
 *
 *  Created on: Feb 7, 2026
 *      Author: valentin
 */

#include <OLED_096.h>

extern I2C_HandleTypeDef hi2c1;
#define OLED_096_I2C &hi2c1

#define OLED_WRITECOMMAND(command)      OLED_Write(0x00, (command))

uint8_t OLED_Buffer[OLED_096_WIDTH * OLED_096_HEIGHT / 8];

void OLED_Write(uint8_t reg, uint8_t data)
{
	uint8_t buffer[2];
	buffer[0] = reg;
	buffer[1] = data;
	HAL_I2C_Master_Transmit(OLED_096_I2C, OLED_096_ADDR, buffer, 2, 1000);
}

void OLED_Write_nByte(uint8_t reg, uint8_t* data, uint8_t len)
{
	uint8_t buffer[256];
	buffer[0] = reg;
	for(uint8_t i = 0; i < len; i++)
	{
		buffer[i+1] = data[i];
	}
	HAL_I2C_Master_Transmit(OLED_096_I2C, OLED_096_ADDR, buffer, len+1, 1000);
}


void OLED_Read_nByte(uint8_t addr, uint8_t *buffer, uint32_t len)
{
	HAL_I2C_Master_Transmit(OLED_096_I2C, OLED_096_ADDR, &addr, 1, 1000);
	HAL_Delay(100);
	HAL_I2C_Master_Receive(OLED_096_I2C, OLED_096_ADDR, buffer, len, 1000);
	HAL_Delay(100);
}


int OLED_Init(void)
{
	if(HAL_I2C_IsDeviceReady(OLED_096_I2C, OLED_096_ADDR, 1, 20000) != HAL_OK)
	{
		return 0;
	}

	OLED_WRITECOMMAND(OLED_DISPLAY_OFF);

	OLED_WRITECOMMAND(OLED_MEM_ADDR_MODE);
	OLED_WRITECOMMAND(0x02); //Page addr mode

	OLED_WRITECOMMAND(OLED_PAGE_START);

	OLED_WRITECOMMAND(OLED_COM_SCAN_DIR_INV);

	OLED_WRITECOMMAND(OLED_LOW_COL_ADDR);
	OLED_WRITECOMMAND(OLED_HIGH_COL_ADDR);
	OLED_WRITECOMMAND(OLED_DISPLAY_START_LINE);

	OLED_WRITECOMMAND(OLED_CONTRAST);
	OLED_WRITECOMMAND(0xFF); //highest contrast

	OLED_WRITECOMMAND(OLED_SEGMENT_REMAP_0);

	OLED_WRITECOMMAND(OLED_NORMAL_DISPLAY);

	OLED_WRITECOMMAND(OLED_MUX_RATION);
	OLED_WRITECOMMAND(0x3F); //height 64pixels

	OLED_WRITECOMMAND(OLED_ENTIRE_DISPLAY_ON_RAM);

	OLED_WRITECOMMAND(OLED_DISPLAY_OFFSET);
	OLED_WRITECOMMAND(0x00); //offset 30

	OLED_WRITECOMMAND(OLED_CLOCK_DIV_RATIO);
	OLED_WRITECOMMAND(0xF0); //max freq

	OLED_WRITECOMMAND(OLED_PRE_CHARGE_PERIOD);
	OLED_WRITECOMMAND(0x22); //default

	OLED_WRITECOMMAND(OLED_COM_PIN);
	OLED_WRITECOMMAND(0x12); //default

	OLED_WRITECOMMAND(OLED_VCOM_LEVEL);
	OLED_WRITECOMMAND(0x20); //default

	OLED_WRITECOMMAND(0x8D); // Charge Pump
	OLED_WRITECOMMAND(0x14); // Enable

	OLED_WRITECOMMAND(OLED_DISPLAY_ON);

	OLED_WRITECOMMAND(OLED_STOP_SCROLL);

	OLED_Fill(BLACK);

	OLED_UpdateScreen();

	return 1;
}

void OLED_UpdateScreen(void)
{
	uint8_t m;

	for (m = 0; m < 8; m++) {
		OLED_WRITECOMMAND(OLED_PAGE_START + m);
		OLED_WRITECOMMAND(OLED_LOW_COL_ADDR);
		OLED_WRITECOMMAND(OLED_HIGH_COL_ADDR);

		/* Write multi data */
		OLED_Write_nByte(0x40, &OLED_Buffer[OLED_096_WIDTH*m], OLED_096_WIDTH);
	}
}

//black is 0 white is 1
void OLED_Fill(uint8_t color)
{
	/* Set memory */
	memset(OLED_Buffer, (color == BLACK) ? 0x00 : 0xFF, sizeof(OLED_Buffer));
}

int OLED_DrawPixel(uint8_t color, uint16_t x, uint16_t y)
{
	if ( x >= OLED_096_WIDTH || y >= OLED_096_HEIGHT )
	{
		return 0;
	}


	/* Set color */
	if (color == WHITE) {
		OLED_Buffer[x + (y / 8) * OLED_096_WIDTH] |= 1 << (y % 8);
	} else {
		OLED_Buffer[x + (y / 8) * OLED_096_WIDTH] &= ~(1 << (y % 8));
	}

	return 1;
}

int OLED_DrawLine(uint8_t color, uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1)
{
	if ( x0 >= OLED_096_WIDTH )
	{
		x0 = OLED_096_WIDTH - 1;
	}
	if ( y0 >= OLED_096_HEIGHT )
	{
		y0 = OLED_096_HEIGHT - 1;
	}
	if ( x1 >= OLED_096_WIDTH )
	{
		x1 = OLED_096_WIDTH - 1;
	}
	if ( y1 >= OLED_096_HEIGHT )
	{
		y1 = OLED_096_HEIGHT - 1;
	}

	//Bresenham's principles

	int16_t dx, dy, sx, sy, error, e2;

	dx = (x0 < x1) ? (x1 - x0) : (x0 - x1);
	dy = -((y0 < y1) ? (y1 - y0) : (y0 - y1));
	sx = (x0 < x1) ? 1 : -1;
	sy = (y0 < y1) ? 1 : -1;
	error = dx + dy;

	while(1)
	{
		OLED_DrawPixel(color, x0, y0);
		e2 = 2*error;
		if( e2 >= dy)
		{
			if(x0 == x1) break;
			error = error + dy;
			x0 = x0 + sx;
		}
		if( e2 <= dx)
		{
			if(y0 == y1) break;
			error = error + dx;
			y0 = y0 + sy;
		}
	}

	return 1;
}

int OLED_DrawRect(uint8_t color, uint16_t x, uint16_t y, uint16_t w, uint16_t h)
{
	if ( x + w >= OLED_096_WIDTH )
	{
		return 0;
	}
	if ( y + h >= OLED_096_HEIGHT )
	{
		return 0;
	}

	OLED_DrawLine(color, x, y, x+w, y);
	OLED_DrawLine(color, x, y, x, y+h);
	OLED_DrawLine(color, x, y+h, x+w, y+h);
	OLED_DrawLine(color, x+w, y, x+w, y+h);

	return 1;
}

int OLED_FillRect(uint8_t color, uint16_t x, uint16_t y, uint16_t w, uint16_t h)
{
	if ( x + w >= OLED_096_WIDTH )
	{
		return 0;
	}
	if ( y + h >= OLED_096_HEIGHT )
	{
		return 0;
	}

	for(uint16_t i = x; i < x+w; i++)
	{
		for(uint16_t j = y; j < y+h; j++)
		{
			OLED_DrawPixel(color, i, j);
		}
	}

	return 1;
}

static uint16_t font_index(char c)
{
    if (c >= '0' && c <= '9')
        return (c - '0') * 5;
    else if (c >= 'A' && c <= 'Z')
        return (c - 'A' + 10) * 5;
    else if (c >= 'a' && c <= 'z')
        return (c - 'a' + 36) * 5;
    else if (c == '*') // char for °
        return 62 * 5;
    else if (c == '-')
        return 63 * 5;
    else if (c == '%')
        return 64 * 5;
    else if (c == ':')
        return 65 * 5;
    else if (c == '.')
        return 66 * 5;
    else
        return 67 * 5; // space / unknown
}

void OLED_DrawChar(char ch, uint8_t color, uint16_t x, uint16_t y)
{
	if ( x + 5 >= OLED_096_WIDTH || y + 8 >= OLED_096_HEIGHT )
	{
		return;
	}

	uint16_t char_index = font_index(ch);
	uint8_t column_data;
	uint16_t render_byte;

	uint8_t shift = y % 8;
	uint16_t mask = 0x00FF << shift;

	for (uint8_t i = 0; i < 5; i++)
	{
		if(color == WHITE)
		{
			column_data = ~bpixel[char_index + i];
		}
		else
		{
			column_data = bpixel[char_index + i];
		}


		render_byte = column_data << shift;

		OLED_Buffer[x + i + (y / 8) * OLED_096_WIDTH] &= ~(uint8_t)(mask & 0xFF);
		OLED_Buffer[x + i + (y / 8) * OLED_096_WIDTH] |= (uint8_t)(render_byte & 0xFF);

		if ((y / 8) + 1 < (OLED_096_HEIGHT / 8)) {
			OLED_Buffer[x + i + ((y / 8) + 1) * OLED_096_WIDTH] &= ~(uint8_t)(mask >> 8);
			OLED_Buffer[x + i + ((y / 8) + 1) * OLED_096_WIDTH] |= (uint8_t)(render_byte >> 8);
		}
	}
}

void OLED_DrawString(const char* str, uint8_t color, uint16_t x, uint16_t y)
{
	while(*str)
	{
		OLED_DrawChar(*str, color, x, y);
		str++;
		x += 6;
		if (x + 5 >= OLED_096_WIDTH)
		{
			y += 8;
			x = 0;
		}
		if( y >= OLED_096_HEIGHT)
		{
			return;
		}
	}

}

void OLED_BoxString(const char* str, uint8_t color, uint16_t x, uint16_t y, uint16_t xr, uint16_t yr, uint16_t w, uint16_t h)
{
	if(color == BLACK)
	{
		OLED_FillRect(WHITE, xr, yr, w, h);
	}
	else
	{
		OLED_FillRect(BLACK, xr, yr, w, h);
	}

	OLED_DrawRect(color, xr, yr, w, h);
	OLED_DrawString(str, color, x, y);

}

static uint16_t max_array(uint16_t* array, uint16_t size)
{
	uint16_t max = array[0];
	for (uint16_t i = 1; i<size; i++){
		if (array[i] > max)
		{
			max = array[i];
		}
	}
	return max;
}

static uint16_t min_array(uint16_t* array, uint16_t size)
{
	uint16_t min = array[0];
	for (uint16_t i = 1; i<size; i++){
		if (array[i] < min)
		{
			min = array[i];
		}
	}
	return min;
}

void OLED_DrawGraph(uint16_t* graph, uint16_t size_graph, uint8_t color, uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1)
{
	uint16_t max = max_array(graph, size_graph);
	uint16_t min = min_array(graph, size_graph);
	int pixel_y;
	int range = y0 - y1;
	int t_range = max - min;

	if (t_range == 0)
	{
		t_range = 1;
	}

	for(uint16_t i = 0; i< x1 - x0; i++)
	{
		pixel_y = y1 + range * (graph[i] - min)/t_range;
		OLED_DrawPixel(color, x0 + i, (uint16_t)pixel_y);
	}

}








