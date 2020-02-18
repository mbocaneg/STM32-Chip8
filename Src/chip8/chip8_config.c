#include "chip8_config.h"
#include "../SSD1306/SSD1306.h"
#include "task.h"

#define I2C_SDA 21
#define I2C_SCL 22

extern ssd1306 oled;
void _window_init(){
	ssd1306_init(&oled);
    ssd1306_clear(&oled);
    ssd1306_refresh(&oled);
}
void _window_kill(){
    ssd1306_clear(&oled);
    ssd1306_refresh(&oled);
}

uint64_t _get_tick(){
	return (uint64_t)xTaskGetTickCount();
}
