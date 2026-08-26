/**
@file
@brief Timer setup functions
@author Takuya
*/

#include "MS_definitions.h"
#include "MS_config.h"
//#include <math.h>

#if defined(BATTERY_ENABLE) || defined(WPT_ADC_ENABLE) || defined(PYTHON480_ENABLE) || defined(DMA_TO_SPI_ENABLE) || defined(DMA_TO_USART_ENABLE)
static struct timer_task TIMER_0_task1, TIMER_0_task2, TIMER_0_task3, TIMER_0_task4;
#endif

void timerInit(void)
{
	#if defined(PYTHON480_ENABLE)
	// Setup a timer to count in milliseconds
	TIMER_0_task1.interval	= 1; // Need to check this value
	TIMER_0_task1.cb		= millisecondTimer_cb;
	TIMER_0_task1.mode		= TIMER_TASK_REPEAT;
	timer_add_task(&TIMER_0, &TIMER_0_task1);
	#endif
	
	#if defined(BATTERY_ENABLE) || defined(WPT_ADC_ENABLE)
	TIMER_0_task2.interval = 500; // Units are in ms so 1000 should check every 1 second
	TIMER_0_task2.cb       = checkBattVoltage_cb;
	TIMER_0_task2.mode     = TIMER_TASK_REPEAT;
	timer_add_task(&TIMER_0, &TIMER_0_task2);
	#endif
	
	//just for starting up -> inserted in pcc_cb which should be a little more appropriate
	#if 0
	#if defined(PYTHON480_ENABLE)
	// Setup a timer for recording functions.
	//TIMER_0_task3.interval	= round(1000/numBuffersPerFrame/FRAME_RATE/2); // Need to check this value
	TIMER_0_task3.interval	= 25; // Need to check this value
	TIMER_0_task3.cb		= recording_cb;
	TIMER_0_task3.mode		= TIMER_TASK_REPEAT;
	timer_add_task(&TIMER_0, &TIMER_0_task3);
	#endif
	
//	#if defined(DMA_TO_SPI_ENABLE) || defined(DMA_TO_USART_ENABLE)
	// Setup a timer for recording functions.
	//TIMER_0_task3.interval	= round(1000/numBuffersPerFrame/FRAME_RATE/2); // Need to check this value
	TIMER_0_task4.interval	= 100; // Need to check this value
	TIMER_0_task4.cb		= sdo_dma_transfer_control_cb;
	TIMER_0_task4.mode		= TIMER_TASK_REPEAT;
	timer_add_task(&TIMER_0, &TIMER_0_task4);
	#endif
	
	timer_start(&TIMER_0);
}

uint32_t getCurrentTimeMS(void)
{
	return timeMS;
}


