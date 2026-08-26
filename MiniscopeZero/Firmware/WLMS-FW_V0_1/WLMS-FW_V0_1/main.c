#include <atmel_start.h>
#include "MS_config.h"
#include "MS_definitions.h"

int main(void)
{
	#ifdef PYTHON480_ENABLE
	getBuffersPerFrame();
	#endif
	
	/* Initializes MCU, drivers and middleware */
	atmel_start_init();

	//Note sure if this is needed. Takuya
	__disable_irq();

	DataBufferInit(); // For testing buffer to SERCOM DMA

	peripheralInit();

	#ifdef DMA_TO_SD_ENABLE
	debugHeaderProp();
	#endif
	
	dmaEnable();
	
	//Note sure if this is needed. Takuya
	__enable_irq();

	#ifdef AUTOSTART_ENABLE
	delay_ms(1000);
	deviceState = DEVICE_STATE_START_RECORDING;
	#endif
		
	while (1) { // mostly recording_cb and framevalid_cb runs here
		#if defined(PYTHON480_ENABLE)
		if (deviceState & DEVICE_STATE_START_RECORDING){
			startRecording();
		}
		
		if (deviceState & DEVICE_STATE_RECORDING) {
			#ifdef DMA_TO_SD_ENABLE
			sdmmc_dma_transfer_control();
			#endif
		}
		// Module for stopping recording. Only needed for SD card
		if (deviceState & DEVICE_STATE_STOP_RECORDING) {
			stopRecording();
		}
		#endif //#if defined(PYTHON480_ENABLE)
		
		//Only for testing DMA to serial data out
		#ifndef PYTHON480_ENABLE
		//sdo_dma_transfer_control(false);
		#endif
	}
}