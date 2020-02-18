#ifndef SSD1306_SSD1306_CFG_H_
#define SSD1306_SSD1306_CFG_H_

/*
 * Header file used for configuring any microcontroller specific logic, specifically
 * logic related to i2c operations(for the ssd1306 peripheral, only i2c writes are needed)
 * */

#include "SSD1306.h"
#include "main.h"

/*
 * Platform dependent definitions/variables
 * */
extern I2C_HandleTypeDef hi2c1;
extern ssd1306 oled;
uint32_t I2C_TIMEOUT = 50;

/*
 * Wrapper around the microcontroller dependent i2c write function. In STM32 HAL, this function is called
 * HAL_I2C_Mem_Write
 *
 * PARAMETERS
 * -----------
 * addr: i2c device address
 * reg: register within i2c device we wish to write to
 * *data: pointer to the starting address of the data we wish to write
 * len: length(in bytes) of data we wish to write
 * */
void _i2c_write_reg(uint8_t addr, uint8_t reg, uint8_t *data, uint16_t len) {
	HAL_I2C_Mem_Write(&hi2c1, addr, reg, I2C_MEMADD_SIZE_8BIT, data, len,
			I2C_TIMEOUT);
}

/*
 * Function that binds the microcontroller's i2c write function to the ssd1306 implementation
 *
 * PARAMETERS
 * ----------
 * oled: ssd1306 instance we wish to bind the i2c write function to
 * */
void cfg_ssd1306(ssd1306 *oled){
	ssd1306_register_i2c_write_reg(oled, _i2c_write_reg);
}



#endif /* SSD1306_SSD1306_CFG_H_ */
