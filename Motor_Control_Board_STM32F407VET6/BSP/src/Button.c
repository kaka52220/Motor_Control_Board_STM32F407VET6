#include "Button.h"


uint8_t read_button_gpio(uint8_t button_id)
	{
		switch(button_id)
		{
			case 1:
				return (uint8_t)HAL_GPIO_ReadPin(KEY1_GPIO_Port,KEY1_Pin);
			default: 
				return 8;
		}
	}
	
void button_init(button *handle,uint8_t (*hal_read_button_level)(uint8_t),uint8_t button_id,uint8_t active_level)//这里函数指针形参写uint8_t也可以,不用button_id
	{
		handle -> button_id = button_id;
		handle -> active_level = active_level;
		handle -> hal_read_button_level = hal_read_button_level;
	}	

uint8_t button_read_level(button *handle)
	{
		return handle -> hal_read_button_level(handle -> button_id);
	}

