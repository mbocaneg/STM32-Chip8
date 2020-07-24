/*
 * SSD1306.h
 *
 *  Created on: Apr 18, 2018
 *      Author: ares
 */

#ifndef SSD1306_SSD1306_H_
#define SSD1306_SSD1306_H_

#include "SSD1306_defs.h"

/*
 * struct that is used for representing an instance of an ssd1306 oled display
 *
 * MEMBERS
 * -------
 * (*I2C_Write_Reg): function pointer that points to the microcontroller's onboard
 * i2c write function
 *
 * gfx_buf: array that holds oled pixel data
 * */
typedef struct ssd1306_t{
	void (*I2C_Write_Reg)(uint8_t addr, uint8_t reg, uint8_t *data, uint16_t len);
	uint8_t gfx_buf[1024];
}ssd1306;

/*
 * Function that formally binds the ssd1306 struct (*I2C_Write_Reg) pointer to a concrete
 * i2c write function
 * PARAMETERS:
 * -----------
 * oled: oled instance
 * (*i2c_write_reg): pointer to a concrete implementation of the i2c write function
 * */
void ssd1306_register_i2c_write_reg(ssd1306 *oled, void (*i2c_write_reg)(uint8_t addr, uint8_t reg, uint8_t *data, uint16_t len));

/*
 * Function that performs boilerplate actions necessary to initialize the ssd1306 oled
 * */
void ssd1306_init(ssd1306 *oled);

/*
 * Functions that refresh, invert, and clear pixels on the oled
 * */
void ssd1306_refresh(ssd1306 *oled);
void ssd1306_invert(ssd1306 *oled, bool inv);
void ssd1306_clear(ssd1306 *oled);

/*
 * Functions that draw graphics primitives onto the oled
 * */
void ssd1306_draw_pixel(ssd1306 *oled, uint8_t x, uint8_t y, bool set);
void ssd1306_draw_hline(ssd1306 *oled, uint8_t x, uint8_t y, uint8_t width, bool set);
void ssd1306_draw_vline(ssd1306 *oled, uint8_t x, uint8_t y, uint8_t height, bool set);
void ssd1306_fill_rectangle(ssd1306 *oled, uint8_t x, uint8_t y, uint8_t width, uint8_t height, bool set);
void ssd1306_draw_rectangle(ssd1306 *oled, uint8_t x, uint8_t y, uint8_t width, uint8_t height, bool set);

#endif /* SSD1306_SSD1306_H_ */
