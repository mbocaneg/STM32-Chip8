/*
 * SSD1306.c
 *
 *  Created on: Apr 18, 2018
 *      Author: ares
 */

#include "SSD1306.h"
#include "SSD1306_cfg.h"

/*
 * Helpful macro used for passing around literals without storing them in
 * a temporary variable
 * */
#define BYTE(X) &(uint8_t){X}

extern UART_HandleTypeDef huart1;
extern char str_buf[64];


/*
 * More wrappers around the i2c write function. These, in particular, communicate
 * to the ssd1306 whether the information being written to it is a command or data.
 * This is indicated by the fact that data is written to "register" 0x40 and
 * commands are written to "register" 0x00.
 * */
static void ssd1306_cmd(ssd1306 *oled, uint8_t data){
	oled->I2C_Write_Reg(SSD1306_DEFAULT_ADDRESS, 0x00, BYTE(data), 1);
}

static void ssd1306_data(ssd1306 *oled, uint8_t data){
	oled->I2C_Write_Reg(SSD1306_DEFAULT_ADDRESS, 0x40, BYTE(data), 1);
}

/*
 * Function that binds a function pointer to an oled i2c function pointer member
 * */
void ssd1306_register_i2c_write_reg(ssd1306 *oled, void (*i2c_write_reg)(uint8_t addr, uint8_t reg, uint8_t *data, uint16_t len)){
	oled->I2C_Write_Reg = i2c_write_reg;
}

void ssd1306_init(ssd1306 *oled){
	memset(str_buf, 0, 64);
	sprintf(str_buf, "init SSD1306\r\n");
	HAL_UART_Transmit(&huart1, (uint8_t *) str_buf, strlen(str_buf),
			50);

	// configure any platform specific logic before anything else
	cfg_ssd1306(oled);

	// boilerplate initialization logic. The arduino "Adafruit_SSD1306" and ESP32 "esp32-i2c-ssd1306-oled"
	// libraries and official datashet were used for reference
	ssd1306_cmd(oled, SSD1306_DISPLAYOFF);
	ssd1306_cmd(oled, SSD1306_SETDISPLAYCLOCKDIV);
	ssd1306_cmd(oled, 0x80);
	ssd1306_cmd(oled, 0x80);
	ssd1306_cmd(oled, SSD1306_SETMULTIPLEX);
	ssd1306_cmd(oled, 0x3F);
	ssd1306_cmd(oled, SSD1306_SETDISPLAYOFFSET);
	ssd1306_cmd(oled, 0x00);
	ssd1306_cmd(oled, SSD1306_SETSTARTLINE);
	ssd1306_cmd(oled, SSD1306_MEMORYMODE);
	ssd1306_cmd(oled, 0x00);
	ssd1306_cmd(oled, SSD1306_SEGREMAP | 0x1);
	ssd1306_cmd(oled, SSD1306_COMSCANDEC);
	ssd1306_cmd(oled, SSD1306_SETCOMPINS);
	ssd1306_cmd(oled, 0x12);
	ssd1306_cmd(oled, SSD1306_SETCONTRAST);
	ssd1306_cmd(oled, 0xCF);
	ssd1306_cmd(oled, SSD1306_SETPRECHARGE);
	ssd1306_cmd(oled, 0xF1);
	ssd1306_cmd(oled, SSD1306_SETVCOMDETECT);
	ssd1306_cmd(oled, 0x30);
	ssd1306_cmd(oled, SSD1306_CHARGEPUMP);
	ssd1306_cmd(oled, 0x14);
	ssd1306_cmd(oled, SSD1306_DEACTIVATE_SCROLL);
	ssd1306_cmd(oled, SSD1306_DISPLAYALLON_RESUME);
	ssd1306_cmd(oled, SSD1306_NORMALDISPLAY);
	ssd1306_cmd(oled, SSD1306_DISPLAYON);

	//after initialization is carried out, clear the screen
	ssd1306_clear(oled);
}

/*
 * Function that sets the oled pixels in an inverted or non inverted state
 *
 * PARAMETERS
 * ----------
 * oled: oled instance we wish to modify
 *
 * inv: bool value that represents the state we wish to
 * write. A value of true inverts the display, where a
 * value of false sets the display to its normal state
 * */
void ssd1306_invert(ssd1306 *oled, bool inv){
	if(inv)
		ssd1306_cmd(oled, SSD1306_INVERTDISPLAY);
	else
		ssd1306_cmd(oled, SSD1306_NORMALDISPLAY);
}

//Function that clears the display by settings its graphics buffer to zeros
void ssd1306_clear(ssd1306 *oled){
	memset(oled->gfx_buf, 0, 1024);
}

//Function that refreshes the oled by writings it graphics buffer to its
//internal graphics ram
void ssd1306_refresh(ssd1306 *oled){
	ssd1306_cmd(oled, SSD1306_COLUMNADDR);
	ssd1306_cmd(oled, 0x00);
	ssd1306_cmd(oled, 0x7F);

	ssd1306_cmd(oled, SSD1306_PAGEADDR);
	ssd1306_cmd(oled, 0x00);
	ssd1306_cmd(oled, 0x07);

	for(int i = 0; i < 1024; i++){
		ssd1306_data(oled, oled->gfx_buf[i]);
	}

}


/*
 * The lowest level graphics primitive upon with other primitives are built
 * on. It simply draws/clears a pixel on a given x/y coordinate
 *
 * PARAMETERS
 * ----------
 * oled: ssd1306 instance we wish to write to
 *
 * x, y: coordinates we wish to write/clear a pixel to
 *
 * set: bool that indicates whether we wish to write(TRUE) or clear(FALSE) a
 * pixel
 * */
void ssd1306_draw_pixel(ssd1306 *oled, uint8_t x, uint8_t y, bool set){
	if(set)
		oled->gfx_buf[x+(y/8)*SSD1306_WIDTH] |= (1 << (y & 7));
	else
		oled->gfx_buf[x+(y/8)*SSD1306_WIDTH] &= ~(1 << (y & 7));
}

void ssd1306_draw_hline(ssd1306 *oled, uint8_t x, uint8_t y, uint8_t width, bool set){
	if(x + width > SSD1306_WIDTH)
		width = SSD1306_WIDTH - width;

	for(uint8_t i = 0; i < width; i++){
		ssd1306_draw_pixel(oled, x + i, y, set);
	}
}

/*
 * Primitive graphics function that draws/clears a vertical line at a given coordinate, of a given height
 * */
void ssd1306_draw_vline(ssd1306 *oled, uint8_t x, uint8_t y, uint8_t height, bool set){
	if(y + height > SSD1306_WIDTH)
		height = SSD1306_WIDTH - height;

	for(uint8_t i = 0; i < height; i++){
		ssd1306_draw_pixel(oled, x, y + i, set);
	}
}

/*
 * Primitive graphics function that draws/clears a solid rectangle at a given coordinate, of a given width/height
 * */
void ssd1306_fill_rectangle(ssd1306 *oled, uint8_t x, uint8_t y, uint8_t width, uint8_t height, bool set){
	for(uint8_t i = 0; i < width; i++){
		ssd1306_draw_vline(oled, x + i, y, height, set);
	}
}

/*
 * Primitive graphics function that draws/clears a rectangle outline at a given coordinate, of a given width/height
 * */
void ssd1306_draw_rectangle(ssd1306 *oled, uint8_t x, uint8_t y, uint8_t width, uint8_t height, bool set){
	ssd1306_draw_hline(oled, x, y, width, set);
	ssd1306_draw_hline(oled, x, y + height, width, set);
	ssd1306_draw_vline(oled, x, y, height, set);
	ssd1306_draw_vline(oled, x + width, y, height, set);
}

