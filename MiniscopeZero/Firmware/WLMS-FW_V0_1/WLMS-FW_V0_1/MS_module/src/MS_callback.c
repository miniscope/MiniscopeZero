/**
@file
@brief Callback functions
@author Takuya
*/

#include "MS_config.h"
#include "MS_definitions.h"
#include "dma_custom_driver.h"
#include "python480.h"


#ifdef PYTHON480_ENABLE
#include <hpl_pcc_config.h>
#endif 
#include <hpl_dma.h>

#ifdef PYTHON480_ENABLE
void millisecondTimer_cb(const struct timer_task *const timer_task)
{
	timeMS++;
}
#endif

#ifdef BATTERY_ENABLE
void checkBattVoltage_cb(const struct timer_task *const timer_task)
{
	uint16_t adcValueBattery;
	uint8_t adcValueWPT;
	// Uses ADC0 to check battery voltage
	adc_sync_read_channel(&ADC_0, 0, &adcValueBattery, 1);
	battVolt = adcValueBattery;
	
	#ifdef WPT_ADC_ENABLE
	adc_sync_read_channel(&ADC_1, 0, &adcValueWPT, 1);
	wptVolt = adcValueWPT;
	#endif
	
	// If under voltage, set device state to ...
	// Compare to 1.1V band gap
	// Battery voltage goes through a 1/5x voltage divider
	
	// Raise issue if voltage is under 3.4V
	// 3.4V = 158
	// 3.3V = 148
	#if 0
	if (adcValueBattery < getPropFromHeader(HEADER_RECORD_LENGTH_POS)) {
		// Low voltage problem
		deviceState |= DEVICE_STATE_LOW_VOLTAGE;
		deviceState |= DEVICE_STATE_STOP_RECORDING;
	}
	#endif
	
	#ifdef DEBUGLED_ENABLE
	#ifdef WPT_ADC_ENABLE
	if (wptVolt > ADC_WPT_HIGH)
	{
		gpio_set_pin_level(DEBUGLED3, (timeMS/500)%2);
	}
	else if (wptVolt > ADC_WPT_LOW)
	{
		gpio_set_pin_level(DEBUGLED3, 1);
	}
	#endif
	
	#endif
	if (deviceState & DEVICE_STATE_IDLE)
	{
		gpio_set_pin_level(LED_STATUS, 0);
	}
	else if(battVolt < ADC_BATTERY_LOW)
	{
		gpio_set_pin_level(LED_STATUS, (timeMS/500)%2);
	}
}
#endif

#ifdef IR_TRIGGER_ENABLE
void irReceive_cb(void)
{
	bool pinState = gpio_get_pin_level(IR_REC);
	if (pinState == true) {
		delay_ms(13); //13 ms after the pin gets high we check if there is a 0
		if (gpio_get_pin_level(IR_REC) == false) {
			for( uint8_t i = 0; i < delayvalue; i++ ){
				gpio_toggle_pin_level(LED_STATUS);
				delay_ms(1000);
			}
			deviceState = DEVICE_STATE_START_RECORDING;
		}		
	}
}
#endif

#ifdef IR_UART_ENABLE
#define CMD_HEADER_MASK					0b11000000
#define CMD_USART_PAYLOAD_MASK			0b00111111

#define CMD_USART_ID_HEADER				0b00000000
#define CMD_USART_TARGET_HEADER			0b11000000
#define CMD_USART_VALUE_LSB_HEADER		0b01000000
#define CMD_USART_VALUE_MSB_HEADER		0b10000000

#define CMD_TARGET_EXLED				0
#define CMD_TARGET_GAIN					1
#define CMD_TARGET_ROI_X				2
#define CMD_TARGET_ROI_Y				3
#define CMD_TARGET_SUBSAMPLE			4
//#define CMD_TARGET_ROI_WIDTH			4
//#define CMD_TARGET_EWL				5
#define CMD_TARGET_DEVICE				50

#define CMD_UNDEFINED					0b11111111

uint8_t updateDevice	= CMD_UNDEFINED;
uint8_t updateTarget	= CMD_UNDEFINED;
uint8_t updateValueLSB	= CMD_UNDEFINED;
uint8_t updateValueMSB	= CMD_UNDEFINED;

void usart_rx_cb(void)
{
	volatile uint8_t uartBuffer = sercom_ir->USART.DATA.reg;
	
	if (((uartBuffer & CMD_HEADER_MASK) == CMD_USART_ID_HEADER) &&
		(((uartBuffer & CMD_USART_PAYLOAD_MASK) == DEVICE_ID) || ((uartBuffer & CMD_USART_PAYLOAD_MASK) == 0)) &&
		updateDevice == CMD_UNDEFINED &&
		updateTarget == CMD_UNDEFINED &&
		updateValueLSB == CMD_UNDEFINED &&
		updateValueMSB == CMD_UNDEFINED)
		{
			updateDevice = uartBuffer & CMD_USART_PAYLOAD_MASK;
	}
	else if (((uartBuffer & CMD_HEADER_MASK) == CMD_USART_TARGET_HEADER) &&
		updateDevice != CMD_UNDEFINED &&
		updateTarget == CMD_UNDEFINED &&
		updateValueLSB == CMD_UNDEFINED &&
		updateValueMSB == CMD_UNDEFINED)
		{
			updateTarget = uartBuffer & CMD_USART_PAYLOAD_MASK;
	}
	else if (((uartBuffer & CMD_HEADER_MASK) == CMD_USART_VALUE_LSB_HEADER) &&
		updateDevice != CMD_UNDEFINED &&
		updateTarget != CMD_UNDEFINED &&
		updateValueLSB == CMD_UNDEFINED &&
		updateValueMSB == CMD_UNDEFINED)
		{
		updateValueLSB = uartBuffer & CMD_USART_PAYLOAD_MASK;
	}
	else if (((uartBuffer & CMD_HEADER_MASK) == CMD_USART_VALUE_MSB_HEADER) &&
		updateDevice != CMD_UNDEFINED &&
		updateTarget != CMD_UNDEFINED &&
		updateValueLSB != CMD_UNDEFINED &&
		updateValueMSB == CMD_UNDEFINED)
		{
			updateValueMSB = uartBuffer & CMD_USART_PAYLOAD_MASK;
			uint16_t updateValue = (updateValueMSB << 6) + updateValueLSB;
	
			update_recording(updateTarget, updateValue);
			updateDevice = CMD_UNDEFINED;
			updateTarget = CMD_UNDEFINED;
			updateValueLSB = CMD_UNDEFINED;
			updateValueMSB = CMD_UNDEFINED;
	}
	else {
		updateDevice = CMD_UNDEFINED;
		updateTarget = CMD_UNDEFINED;
		updateValueLSB = CMD_UNDEFINED;
		updateValueMSB = CMD_UNDEFINED;
	}
}

void update_recording(uint8_t updateTarget, uint16_t updateValue)
{
	switch (updateTarget) {
		case CMD_TARGET_EXLED:
		setExcitationLED((uint32_t) updateValue, 1);
		break;
		case CMD_TARGET_GAIN:
		python480SetGain((uint32_t) updateValue);
		break;
		case CMD_TARGET_ROI_X:
		roi_x_shift = updateValue;
		setROI(image_width, roi_x_shift, roi_y_shift);
		break;
		case CMD_TARGET_ROI_Y:
		roi_y_shift = updateValue;
		setROI(image_width, roi_x_shift, roi_y_shift);
		break;
		case CMD_TARGET_SUBSAMPLE:
		if (subsampleEnable == 0 & updateValue == 1){
			image_width = image_width * 2;
			image_height = image_height * 2;
			subsampleEnable = updateValue;
			calcImaceSize();
			imageSensorInit();
		}
		else if (subsampleEnable == 1 & updateValue == 0)
		{
			image_width = image_width / 2;
			image_height = image_height / 2;
			subsampleEnable = updateValue;
			calcImaceSize();
			imageSensorInit();
		}
		break;
		case CMD_TARGET_DEVICE:
		if (updateValue == RESTART_KEY){
			NVIC_SystemReset();
		}
		default:
		return;
	}
}
#endif

#ifdef PUSH_BUT_ENABLE
void pushButton_cb(void)
{
	bool pinState = gpio_get_pin_level(PUSH_BUT_MCU);
	if (pinState == true) {
		
	}
	else {
		
	}
}
#endif

// HS CHECK, this function is called when there is data still remaining to be filled into a buffer,
// so if the camera i
// handle frame when buffer is not full

#ifdef PYTHON480_ENABLE
void frameValid_cb(void)
{
	bool pinState = gpio_get_pin_level(FrameValid);
	
	//if (gpio_get_pin_level(LED_STATUS) == 1) {
	//setStatusLED(0);
	//
	//}
	//else {
	//setStatusLED(1);
	//}
	
	if (pinState == true) {	// beginning of new frame acquisition
	}
	else { // Handles end of frame
		
		if (deviceState & (DEVICE_STATE_RECORDING | DEVICE_STATE_STOP_RECORDING)) {
			// At the end of frame the current buffer is likely only partially filled.
			// Disable DMA to flush DMA FIFO then start DMA again but with the next linked list
			
			PCC->MR.reg &= ~(PCC_MR_PCEN); // Disables PCC
			DMAC->Channel[CONF_PCC_DMA_CHANNEL].CHCTRLA.reg &= ~(DMAC_CHCTRLA_ENABLE); // Disables PCC DMA
			
			// Some debugging stuff here
			//tempHeader[tempCount][0] = frameNum;
			//tempHeader[tempCount][1] = bufferCount;
			//tempHeader[tempCount][2] = 4 * dma_get_WRB_data(CONF_PCC_DMA_CHANNEL);
			//tempHeader[tempCount][3] = PCC->ISR.reg;;
			//if (tempCount < 99)
			//tempCount++;
			
			//I think this line sometimes doesn't catch up with the SD card transfer trigger (Takuya)
			setBufferHeader((BUFFER_BLOCK_LENGTH * PCC_BLOCK_SIZE_IN_WORDS - (BUFFER_HEADER_LENGTH + DUMMY_WORD_LENGTH)) - _dma_get_WRB_data(CONF_PCC_DMA_CHANNEL)); // This should get total beats transferred through DMA
			
			frameBufferCount = 0;
			bufferCount++; // A buffer has been filled (likely partially) and is ready for writing to SD card
			frameNum++; // Zero-Indexed
			
			if (deviceState & DEVICE_STATE_RECORDING) { // Keep recording
				// Update Linked List
				setPCCLinkedListPosition(bufferCount % NUM_BUFFERS); // Moves to next buffer/linked list element
				#if 0 // this part is probably not needed because the SDO linked list is independent of end of frame
				if (bufferCount % NUM_BUFFERS == 0)
				{
					setTXLinkedListPosition(NUM_BUFFERS - 1); // Moves to next buffer/linked list element
				}
				else{
					setTXLinkedListPosition(bufferCount % NUM_BUFFERS - 1); // Moves to next buffer/linked list element
				}
				#endif
				_dma_enable_transaction(CONF_PCC_DMA_CHANNEL, false); // Should enable DMA transfer
				
				PCC->MR.reg |= PCC_MR_PCEN; // Enables PCC
			}
			if (deviceState & DEVICE_STATE_STOP_RECORDING) {
				// Reset linked lists so we will be ready to start recording again in the future
				
				//deviceState &= ~(DEVICE_STATE_RECORDING);
				//deviceState &= ~(DEVICE_STATE_STOP_RECORDING);
				//deviceState |= DEVICE_STATE_IDLE;
			}
			#ifdef EWL_SWEEP_ENABLE
			if ((frameNum + 1) % 5 == 0 & ewlvalue < 255)
			{
				ewlvalue++;
				setEWL(ewlvalue);
			}		
			else if ((frameNum + 1) % 5 == 0 & ewlvalue >= 255)
			{				
				ewlvalue = 0;
				setEWL(ewlvalue);
			}
			#endif
		}
		else if (deviceState & DEVICE_STATE_START_RECORDING_WAITING) {
			// We wait till !FV to enable recording so the first buffer starts at the beginning and not middle of a frame
			
			frameNum = 0;
			bufferCount = 0;
			frameBufferCount = 0;
			
			#if defined(DMA_TO_SPI_ENABLE) || defined(DMA_TO_USART_ENABLE)
			TXLinkedListInit();
			#endif
			PCCLinkedListInit();
			setPCCLinkedListPosition(0); // Moves to next buffer/linked list element
			_dma_enable_transaction(CONF_PCC_DMA_CHANNEL, false); // Should enable DMA transfer
			
			PCC->MR.reg |= PCC_MR_PCEN; // Enables PCC
			
			deviceState &= ~(DEVICE_STATE_START_RECORDING_WAITING);
			deviceState |= DEVICE_STATE_RECORDING;
		}
	}
}
#endif
// HS Check image_sensor_dma_cb
// then place the ifdef inside the function
// flexibility to add segments of code that only compiles with the presence of specific image sensors
// THIS IS THE XDMA handler (when DMA is filled, then callback)
// this is the core difference
#ifdef PYTHON480_ENABLE
void pcc_dma_cb(struct camera_async_descriptor *const descr, uint32_t ch)
{
	if (ch == CONF_PCC_DMA_CHANNEL) {
		pcc_dma_cb_calls++;
			
		// add header to current buffer
		
		// Some debugging stuff here
		//tempHeader[tempCount][0] = frameNum;
		//tempHeader[tempCount][1] = bufferCount;
		//tempHeader[tempCount][2] = 4 * dma_get_WRB_data(CONF_PCC_DMA_CHANNEL);
		//tempHeader[tempCount][3] = PCC->ISR.reg;
		//if (tempCount < 99)
		//tempCount++;
		
		setBufferHeader(BUFFER_BLOCK_LENGTH * PCC_BLOCK_SIZE_IN_WORDS - (BUFFER_HEADER_LENGTH + DUMMY_WORD_LENGTH));
		bufferCount++;// increment counters
		frameBufferCount++;
		
		#if 0
		sdmmc_dma_transfer_control();
		#endif
		#if defined(DMA_TO_SPI_ENABLE) || defined(DMA_TO_USART_ENABLE)
		sdo_dma_transfer_control(false);		
		#endif
	}
}

void recording_cb(const struct timer_task *const timer_task)
{
	sdmmc_dma_transfer_control();
}

void sdmmc_dma_transfer_control(void)
{
	// not sure if the bufferCount > 1 is needed.

	// The +1 is a hot fix. It prevents getting the buffer being sent before the number of stored pixels is written to the header.
	if (bufferCount > (writeBufferCount + droppedBufferCount) + 1) { // when camera data is ahead
	//if (bufferCount > (writeBufferCount + droppedBufferCount) && bufferCount > 1) { // when camera data is ahead
		// This means there are filled buffer(s) ready to be written to SD card


		// We need to check if the writing to sd card of data buffers has fallen too far behind where we are at risk
		// of writing overwritten data. We need to detect this and decide what to do in this case
		if (bufferCount > (writeBufferCount + droppedBufferCount + NUM_BUFFERS)) { // when data transfer isn't catching up
			// We  are at risk of at least the current buffer that we want to write to SD card being overflown with new image data
			// We are going to just drop writing the rest of this frame
			
			// Let's figure out how many buffers need to be dropped
			// TODO: I think NUM_BUFFERS here should actually be number_of_buffers_per_frame
			droppedBufferCount += (numBuffersPerFrame - (writeBufferCount + droppedBufferCount) % numBuffersPerFrame);
			//droppedBufferCount += bufferCount - writeBufferCount + droppedBufferCount + NUM_BUFFERS;
		}
		else { // Actual writing of good buffers
			
			bufferToWrite = (uint32_t)(&dataBuffer[(writeBufferCount + droppedBufferCount) % NUM_BUFFERS]);
			numBlocks = (bufferToWrite[BUFFER_HEADER_DATA_LENGTH_POS] + ((BUFFER_HEADER_LENGTH + DUMMY_WORD_LENGTH) * 4) + (SD_BLOCK_SIZE - 1)) / SD_BLOCK_SIZE;
			
			// This if statement shouldn't be needed
			//if (numBlocks > BUFFER_BLOCK_LENGTH)
			//numBlocks = BUFFER_BLOCK_LENGTH;
			
			bufferToWrite[BUFFER_HEADER_WRITE_BUFFER_COUNT_POS] = writeBufferCount;
			bufferToWrite[BUFFER_HEADER_DROPPED_BUFFER_COUNT_POS] = droppedBufferCount;
			bufferToWrite[BUFFER_HEADER_WRITE_TIMESTAMP_POS] = getCurrentTimeMS() - startTimeMS;
			bufferToWrite[BUFFER_HEADER_BATTERY_VOLTAGE_POS] = battVolt;
			bufferToWrite[BUFFER_HEADER_WPT_VOLTAGE_POS] = wptVolt;
			//bufferToWrite[BUFFER_HEADER_EWL_POS] = ewlvalue;
			
			tempTimestamp[(writeBufferCount + droppedBufferCount) % 100] = getCurrentTimeMS() - startTimeMS;
			
			#ifdef DMA_TO_SD_ENABLE
			#ifdef ADMA_ENABLE
			// Sets up ADMA descriptor for writing 1 full buffer
			setSDDescriptor(bufferToWrite, numBlocks * SD_BLOCK_SIZE,
			SD_DESCRIPTOR_ATT_TRANSFER|SD_DESCRIPTOR_ATT_VALID|SD_DESCRIPTOR_ATT_END);
			sd_mmc_write_with_ADMA(0, currentBlock, (uint32_t)&SDTransferDescriptor, numBlocks);
			sd_mmc_wait_end_of_ADMA_write(false);
			currentBlock += numBlocks;
			writeBufferCount++; // Not sure if this should be here.
			
			
			
			#else // not ADMA_ENABLE
			if (numBlocks < initBlocksRemaining) {
				
				#ifdef DMA_TO_SD_ENABLE
				// There are enough init blocks for this write
				if (sd_mmc_start_write_blocks(bufferToWrite, numBlocks) != SD_MMC_OK)
				deviceState |= DEVICE_STATE_SDCARD_WRITE_ERROR;
				sd_mmc_wait_end_of_write_blocks(false);
				#endif
				
				initBlocksRemaining -= numBlocks;
				currentBlock += numBlocks;
			}
			else if (numBlocks == initBlocksRemaining)
			{
				#ifdef DMA_TO_SD_ENABLE
				if (sd_mmc_start_write_blocks(bufferToWrite, numBlocks) != SD_MMC_OK)
				deviceState |= DEVICE_STATE_SDCARD_WRITE_ERROR;
				sd_mmc_wait_end_of_write_blocks(false);
				#endif
				
				currentBlock += numBlocks;
				
				#ifdef DMA_TO_SD_ENABLE
				if (sd_mmc_init_write_blocks(0, currentBlock, BUFFER_BLOCK_LENGTH * NB_BUFFER_WRITES_PER_CHUNK) != SD_MMC_OK)
				deviceState |= DEVICE_STATE_SDCARD_WRITE_ERROR;
				#endif
				
				initBlocksRemaining = (BUFFER_BLOCK_LENGTH * NB_BUFFER_WRITES_PER_CHUNK);
			}
			else {
				// This finishes up the remaining blocks in the current set of initialized blocks
				if (sd_mmc_start_write_blocks(bufferToWrite, initBlocksRemaining) != SD_MMC_OK)
				deviceState |= DEVICE_STATE_SDCARD_WRITE_ERROR;
				sd_mmc_wait_end_of_write_blocks(false);

				currentBlock += initBlocksRemaining;
				
				// We now initialize the next set of blocks
				if (sd_mmc_init_write_blocks(0, currentBlock, BUFFER_BLOCK_LENGTH * NB_BUFFER_WRITES_PER_CHUNK) != SD_MMC_OK)
				deviceState |= DEVICE_STATE_SDCARD_INIT_WRITE_ERROR;
				
				// And write remaining data from buffer
				if (sd_mmc_start_write_blocks((uint32_t)(&bufferToWrite[initBlocksRemaining * SD_BLOCK_SIZE / 4]), numBlocks - initBlocksRemaining) != SD_MMC_OK)
				deviceState |= DEVICE_STATE_SDCARD_WRITE_ERROR;
				sd_mmc_wait_end_of_write_blocks(false);
				
				currentBlock += numBlocks - initBlocksRemaining;
				initBlocksRemaining = (BUFFER_BLOCK_LENGTH * NB_BUFFER_WRITES_PER_CHUNK) - (numBlocks - initBlocksRemaining);
			}
			#endif // not ADMA_ENABLE
			#endif // DMA_TO_SD_ENABLE
		}

		if ((getCurrentTimeMS() - startTimeMS) >= endTimeMS){
			deviceState |= DEVICE_STATE_STOP_RECORDING; // Sets the flag to want to end current recording
			for (int i = 0; i<20; i++)
			{
				gpio_toggle_pin_level(LED_STATUS);
				delay_ms(200);
			}
		}
		if (((getCurrentTimeMS() - startTimeMS) <= getPropFromHeader(HEADER_RECORD_LENGTH_POS) * 1000) && (getPropFromHeader(HEADER_EWL_SCAN_ENABLE_POS) == 1) && (getPropFromHeader(HEADER_RECORD_LENGTH_POS) != 0)){
			//We set a new plane
			if(ewlvalue > 0 && ewlvalue<= 255 && ewlvalue <= ewlStop){  //We make sure that we don't take invalid EWL planes and that we stop at the last value of EWL
				if ((getCurrentTimeMS() - startTimeMS) >= ewlStepTime*1000 *(ewlCount+1)){
					ewlvalue = ewlStart + (ewlCount+1)*ewlStep;
					setEWL(ewlvalue);
					ewlCount++;
				}
						
			}
		}
	}
	
}
#endif

#ifdef BATTERY_ENABLE
void battCharging_cb(void)
{
	#if 0
	bool pinState = gpio_get_pin_level(nCHRG);
	if (pinState == true) {
		// Not charging
		while(deviceState &= ~(DEVICE_STATE_CHARGING)){
			gpio_toggle_pin_level(LED_STATUS);
			delay_ms(5000);
		}
	}

	else {
		// charging
		while(deviceState |= DEVICE_STATE_CHARGING){
			gpio_toggle_pin_level(LED_STATUS);
			delay_ms(250);
		}
	}
	#endif
}
#endif
