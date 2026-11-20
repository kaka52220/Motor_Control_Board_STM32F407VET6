#ifndef __BUTTON_H__
#define __BUTTON_H__
#include "gpio.h"

#define DEBOUNCE_TICKS 3
#define TICKS_INTERVAL 5
#define SHORT_TICKS (300/TICKS_INTERVAL)
#define LONG_TICKS (1000/TICKS_INTERVAL)

typedef struct button button;//前向声明,打破循环依赖
typedef void (*BtnCallBack)(button *handle,void *user_data);

typedef enum{
	BTN_STATE_IDLE = 0,
	BTN_STATE_PRESS,
	BTN_STATE_RELEASE,
	BTN_STATE_REPEAT,
	BTN_STATE_LONG_HOLD
}buttonState;

typedef enum{
	BTN_PRESS_DOWN = 0,
	BTN_PRESS_UP,
	BTN_PRESS_REPEAT,
	BTN_SINGLE_CLICK,
	BTN_DOUBLE_CLICK,
	BTN_LONG_PRESS_START,
	BTN_LONG_PRESS_HOLD,
	BTN_EVENT_COUNT,
	BTN_NONE_PRESS
}buttonEvent;

struct button{
	uint16_t ticks;
	uint8_t repeat;
	uint8_t button_id;
	uint8_t active_level;
	uint8_t button_level;//去抖稳定后的电平
	uint8_t debounce_cnt;
	
	uint8_t state;
	uint8_t event;
	
	uint8_t (*hal_read_button_level)(uint8_t button_id);
	BtnCallBack cb[BTN_EVENT_COUNT];
	
	void *user_data;
	
	button *next;
};


uint8_t read_button_gpio(uint8_t button_id);
uint8_t button_read_level(button *handle);
void button_init(button *handle,uint8_t (*hal_read_button_level)(uint8_t),uint8_t button_id,uint8_t active_level);
void button_debounce(button *handle);
//void button_handle(button *handle);
void button_attach(button *handle,buttonEvent event,BtnCallBack callback);
int button_enroll(button *handle);
void button_stop(button *handle);
void button_Task(void);
#endif
