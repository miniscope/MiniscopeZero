/**
@file
@brief Camera related initiation functions
@author Takuya
@note This could be integrated into python480.c in the future
*/

#include "MS_config.h"
#include "MS_definitions.h"
#include "python480.h"

#ifdef PYTHON480_ENABLE
void imageSensorInit(void){
	// Setup Image Sensor
	// Trigger pin gets init'ed as output low and shouldn't need to be adjusted
	gpio_set_pin_level(RESET_CMOS, 0); // Make sure N_RESET of the PYTHON480 is low for a bit before going high. Shouldn't be needed
	delay_ms(100);
	gpio_set_pin_level(RESET_CMOS, 1);
	delay_us(100); // minimum delay is 10us
	
	python480Init();
	chip_id = spi_BB_Read(0x00); // can use this to make sure MCU can talk to Python480

	python480sequenceInit();		
	
	#ifndef PRESET_HEADER_ENABLE
	python480SetGain(getPropFromHeader(HEADER_GAIN_POS));
	python480SetFPS(getPropFromHeader(HEADER_FRAME_RATE_POS));
	#endif
	python480SetFPS(FRAME_RATE);

}
#endif // python 480 enable