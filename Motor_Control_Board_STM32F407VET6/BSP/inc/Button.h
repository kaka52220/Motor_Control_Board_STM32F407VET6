#ifndef __BUTTON_H__
#define __BUTTON_H__
#include "gpio.h"

typedef struct{
	uint8_t button_id;
	uint8_t active_level;
	uint8_t (*hal_read_button_level)(uint8_t button_id);
}button;

uint8_t read_button_gpio(uint8_t button_id);
uint8_t button_read_level(button *handle);
void button_init(button *handle,uint8_t (*hal_read_button_level)(uint8_t),uint8_t button_id,uint8_t active_level);

#endif
