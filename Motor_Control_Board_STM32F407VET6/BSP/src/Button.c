#include "Button.h"
#include "string.h"

static button *head_handle = NULL;//单链表表头指针

uint8_t read_button_gpio(uint8_t button_id)
	{
		switch(button_id)
		{
			case 1:
				return (uint8_t)HAL_GPIO_ReadPin(KEY1_GPIO_Port,KEY1_Pin);//这里注意break是否需要，有return可以直接结束函数，不需要break；
			case 2:
				return (uint8_t)HAL_GPIO_ReadPin(KEY2_GPIO_Port,KEY2_Pin);
			default: 
				return 0;
		}
	}
	
void button_init(button *handle,uint8_t (*hal_read_button_level)(uint8_t),uint8_t button_id,uint8_t active_level)//这里函数指针形参写uint8_t也可以,不用button_id
	{
		memset(handle,0,sizeof(*handle));//初始化一块内存为0,起点handle,终点sizeof(*handle)
		handle -> button_id = button_id;
		handle -> active_level = active_level;
		handle ->button_level = !active_level;//active_level是激活电平，这里传入1，表示高电平为按下， button_level初始化永远为0，表示初始化为没有按下
		handle ->debounce_cnt = 0;
		handle ->state = BTN_STATE_IDLE;
		handle ->event = BTN_NONE_PRESS;
		handle -> hal_read_button_level = hal_read_button_level;
	}	

uint8_t button_read_level(button *handle)
	{
		return handle -> hal_read_button_level(handle -> button_id);
	}
     
void button_debounce(button *handle)
	{
		uint8_t read_gpio_level = button_read_level(handle);
		if(read_gpio_level != handle ->button_level)
			{
				handle ->debounce_cnt++;
				if(handle ->debounce_cnt >= DEBOUNCE_TICKS)
					{
						handle ->debounce_cnt = 0;
						handle ->button_level = read_gpio_level;
					}
			}
		else
			{
				handle->debounce_cnt = 0;
			}		
	}


	
//------------注册回调-------------//
void button_attach(button *handle,buttonEvent event,BtnCallBack callback,void *user_data)
	{
		if(handle == NULL || (event >= BTN_EVENT_COUNT))
			{
				return;
			}
			
		handle ->cb[event] = callback;
		handle ->user_data = user_data;
	}
//--------------内部触发函数	-----------------//
static void button_emit(button *handle, buttonEvent event)
	{
		handle ->event = event;
		
		if(handle ->cb[event] != NULL)
			{
				handle ->cb[event](handle, handle ->user_data);
			}
	}
	
	//-----------------状态机------------------/
static void button_handle(button *handle)
	{
		//-----非空闲状态下每5ms累加一次---------//
		if(handle ->state > BTN_STATE_IDLE)
			{
				 if(handle ->ticks < UINT16_MAX)
					{
						handle ->ticks++;
					}
			}
		button_debounce(handle);
		switch (handle ->state)
		{
			case BTN_STATE_IDLE:
				if(handle ->button_level == handle ->active_level)
					{
						//handle ->event = BTN_PRESS_DOWN;
						button_emit(handle,BTN_PRESS_DOWN);
						handle ->ticks = 0;
						handle ->repeat = 1;
						handle ->state = BTN_STATE_PRESS;
					}
				else 
					{
						handle ->event = BTN_NONE_PRESS;
					}
				break;
					
			case BTN_STATE_PRESS:
				if(handle ->button_level != handle ->active_level)
					{
						//handle ->event = BTN_PRESS_UP;
						button_emit(handle, BTN_PRESS_UP);
						handle->ticks = 0;
						handle ->state = BTN_STATE_RELEASE;
					}
				else if(handle ->ticks > LONG_TICKS)
					{
						button_emit(handle,BTN_LONG_PRESS_START);//start事件只有一次，case 长按后，hold事件每5ms都会触发一次
						handle ->state = BTN_STATE_LONG_HOLD;
					}	
					break;
					
			case BTN_STATE_RELEASE:
				if(handle ->button_level == handle ->active_level)
					{
						button_emit(handle,BTN_PRESS_DOWN);
						
						handle -> repeat++;
						button_emit(handle,BTN_PRESS_REPEAT);
						
						handle ->ticks = 0;
						handle ->state = BTN_STATE_REPEAT;
					}
					else if(handle->ticks > SHORT_TICKS)
						{
							/* 等待结束，根据按下次数判断单击或双击 */
							if(handle ->repeat == 1)
								{
									button_emit(handle,BTN_SINGLE_CLICK);
								}
							else if(handle ->repeat == 2)
								{
									button_emit(handle,BTN_DOUBLE_CLICK);
								}
								
							handle ->state = BTN_STATE_IDLE;
						}
					break;
						
				case BTN_STATE_REPEAT:	
					if(handle ->button_level != handle ->active_level)
						{
							button_emit(handle,BTN_PRESS_UP);
							
							if(handle ->ticks < SHORT_TICKS)
								{
									handle ->ticks = 0;
									handle ->state = BTN_STATE_RELEASE;
								}
							else
								{
									handle->state = BTN_STATE_IDLE;
								}
						}
					else if (handle->ticks > SHORT_TICKS)
						{
								/* 第二次按得太久，不再当作双击 */
								handle->ticks = 0;
								handle->repeat = 0;
								handle->state = BTN_STATE_PRESS;//点击后又长按的保护
						}
						break;
						
				case BTN_STATE_LONG_HOLD:	
					if(handle ->button_level == handle ->active_level)
						{
								button_emit(handle,BTN_LONG_PRESS_HOLD);//start事件只有一次，case 长按后，hold事件每5ms都会触发一次
						}
					else
						{
							button_emit(handle,BTN_PRESS_UP);
							handle->state = BTN_STATE_IDLE;
						}
					break;
				
				default:
					handle->state = BTN_STATE_IDLE;
					handle->event = BTN_NONE_PRESS;
					handle->ticks = 0;
					handle->repeat = 0;
					break;		
		}
	}

int button_enroll(button *handle)
{
	button *target;
	if(handle == NULL)
		{
			return -2;
		}                                                                                     
		
	/* ---------查重-----------*/
	target = head_handle;
		
	while(target != NULL)
		{
			if(target == handle)
				{
					return -1;
				}				
				
			target = target ->next;
		}
	
	/*----头插法----*/
	handle ->next = head_handle;
	head_handle = handle;
	
	return 0;
}

/*-------------在单链表中删除按键---------------------------------------------------------------------------------------*/
void button_stop(button *handle)
	{
		button **curr = &head_handle;
		
		if(handle == NULL)
			{
				return;
			}
		
		while(*curr != NULL)
			{
				if(*curr == handle)
					{
						*curr = handle ->next;
						// *curr = *curr ->next; 这里要关注运算符优先级的问题,-> 大于 *,所以编译器解析成了 *(curr ->next)这里就是错误的;可以用*curr = (*curr) ->next 代替231行;
						
						handle ->next = NULL;//防野指针
						return;		
					}	
					
				curr = &((*curr) ->next);
					
			}
	}	

/*-----------------------按键主任务------------------------------------------------------------------------------------*/
void button_Task(void)
	{
		button *target = head_handle;
		
		while(target != NULL)
			{
				button_handle(target);
				target = target ->next;
			}
	}
	