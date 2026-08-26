/**
@file
@brief Recording related control functions
@author Takuya
*/

#include "MS_config.h"
#include "MS_definitions.h"
#include "python480.h"
#include "hpl_dma.h"

#ifdef DMA_TO_SD_ENABLE
#include "sd_mmc.h"
#endif

#ifdef PYTHON480_ENABLE
void startRecording()
{
	writeFrameNum = 0;
	writeBufferCount = 0;
	droppedBufferCount = 0;
	droppedFrameCount = 0;
	framesToDrop = 0;
	
	#ifdef DMA_TO_SD_ENABLE
	// This gets the next set of blocks ready to be written into
	#ifndef ADMA_ENABLE
	sd_mmc_init_write_blocks(0, currentBlock, BUFFER_BLOCK_LENGTH * NB_BUFFER_WRITES_PER_CHUNK);
	initBlocksRemaining = BUFFER_BLOCK_LENGTH * NB_BUFFER_WRITES_PER_CHUNK;
	#endif // not ADMA_ENABLE
	#endif

	startTimeMS = getCurrentTimeMS();
	#ifdef RECORDTIME_DISABLE
	endTimeMS = 15 * 1000; // 10 min recording
	#else
	endTimeMS = getPropFromHeader(HEADER_RECORD_LENGTH_POS) * 1000;
	#endif
	
	#ifdef DMA_TO_SD_ENABLE
	#ifndef PRESET_HEADER_ENABLE
	setEWL(getPropFromHeader(HEADER_EWL_POS));
	python480SetGain(getPropFromHeader(HEADER_GAIN_POS));
	python480SetFPS(getPropFromHeader(HEADER_FRAME_RATE_POS));
	setExcitationLED(getPropFromHeader(HEADER_LED_POS), 1);
	setStatusLED(1);
	#endif
	#endif
	
	#ifdef PRESET_HEADER_ENABLE
	setEWL(0x33);   // test value. 0x01 to 0xFF.
	python480SetGain(1); // test value. 1, 2, 4.
	setExcitationLED(EXCITATION_LED_VALUE,1); // (Value, enable) Value: from 0 to 100.
	python480SetFPS(FRAME_RATE); // test value 1, 5, 10, 15, 20.
	setStatusLED(1);
	#endif
	
	deviceState &= ~(DEVICE_STATE_IDLE);
	deviceState &= ~(DEVICE_STATE_START_RECORDING);
	deviceState |= DEVICE_STATE_START_RECORDING_WAITING;	
}

void stopRecording()
{
	
	deviceState &= ~(DEVICE_STATE_STOP_RECORDING);
	deviceState &= ~(DEVICE_STATE_RECORDING);
	deviceState |= DEVICE_STATE_IDLE;
	
	// Must be a better way of doing this. This finishes up the remaining init blocks so we can then write to the config block
	
	#ifdef DMA_TO_SD_ENABLE
	#ifndef ADMA_ENABLE
	while (initBlocksRemaining > BUFFER_BLOCK_LENGTH) {
		if (sd_mmc_start_write_blocks(dataBuffer[0], BUFFER_BLOCK_LENGTH) != SD_MMC_OK)
		deviceState |= DEVICE_STATE_SDCARD_WRITE_ERROR;
		initBlocksRemaining -= BUFFER_BLOCK_LENGTH;
		sd_mmc_wait_end_of_write_blocks(false);
	}
	if (initBlocksRemaining > 0) {
		if (sd_mmc_start_write_blocks(dataBuffer[0], initBlocksRemaining) != SD_MMC_OK)
		deviceState |= DEVICE_STATE_SDCARD_WRITE_ERROR;
		initBlocksRemaining = 0;
		sd_mmc_wait_end_of_write_blocks(false);
	}
	#endif // not ADMA_ENABLE
	#endif
	
	//sd_mmc_wait_end_of_write_blocks(true); // Abort any initalized write blocks

	// Write end of recording info to a block
	setConfigBlockProp(CONFIG_BLOCK_NUM_BUFFERS_RECORDED_POS, writeBufferCount);
	setConfigBlockProp(CONFIG_BLOCK_NUM_BUFFERS_DROPPED_POS, droppedBufferCount);
	
	#ifdef DMA_TO_SD_ENABLE
	// Currently not using ADMA. Might consider switching everything over to ADMA to be consistent
	sd_mmc_init_write_blocks(0,CONFIG_BLOCK, 1);
	sd_mmc_start_write_blocks(configBlock, 1);
	sd_mmc_wait_end_of_write_blocks(false);
	#endif
	
	setExcitationLED(0, false);
	setEWL(0x00);	//Sets the EWL to standby mode
	setStatusLED(0);
}
#endif