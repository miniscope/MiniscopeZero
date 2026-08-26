/**
@file
@brief Utility functions for Miniscope
@author Takuya
*/

#include <atmel_start.h>
#include "MS_config.h"
#include "MS_definitions.h"

#include "python480.h"
#include "i2c_bb.h"
#include <hpl_dma.h>


#ifdef DMA_TO_SD_ENABLE
#include "sd_mmc.h"
#endif

#ifdef DMA_TO_SD_ENABLE
uint32_t lastTime = 0;
bool lastMonitor0 = 0;
bool thisMonitor0 = 0;
#endif

#ifdef DMA_TO_SD_ENABLE
uint8_t loadSDCardHeader(void){
	sd_mmc_init_read_blocks(0,HEADER_BLOCK,1);
	sd_mmc_start_read_blocks(headerBlock,1);
	if (sd_mmc_wait_end_of_read_blocks(false) == SD_MMC_OK)
	return MS_SUCCESS;
	else
	return MS_ERROR;
}
#endif // DMA_TO_SD_ENABLE

uint32_t getPropFromHeader(uint8_t headerPos) {
	uint32_t *header32bit = (uint32_t *)headerBlock;
	
	return header32bit[headerPos];
}

void debugHeaderProp(void){
	ewlvalue = getPropFromHeader(HEADER_EWL_POS);
	batteryvalue = getPropFromHeader(HEADER_BATT_CUTOFF_POS);
	ledvalue = getPropFromHeader(HEADER_LED_POS);
	frameratevalue = getPropFromHeader(HEADER_FRAME_RATE_POS);
	delayvalue = getPropFromHeader(HEADER_DELAY_START_POS);
	reclengthvalue = getPropFromHeader(HEADER_RECORD_LENGTH_POS);
	
	ewlScanEnable = getPropFromHeader(HEADER_EWL_SCAN_ENABLE_POS);
	ewlStart = getPropFromHeader(HEADER_EWL_START_POS);
	ewlStop = getPropFromHeader(HEADER_EWL_STOP_POS);
	ewlStep = getPropFromHeader(HEADER_EWL_STEP_POS);
	ewlStepTime = getPropFromHeader(HEADER_EWL_STEP_TIME_POS);
}

void getBuffersPerFrame(void)
{
	#if defined(PYTHON480_ENABLE) || defined(NANEYE_ENABLE)
	numBuffersPerFrame = (num_pixels) / (BUFFER_BLOCK_LENGTH * SD_BLOCK_SIZE - ((BUFFER_HEADER_LENGTH + DUMMY_WORD_LENGTH) * 4));
	if((num_pixels) % (BUFFER_BLOCK_LENGTH * SD_BLOCK_SIZE - ((BUFFER_HEADER_LENGTH + DUMMY_WORD_LENGTH) * 4)) != 0) {
		// Need to add 1 to account for partially filled buffer
		numBuffersPerFrame += 1;
	}	
	#endif // defined(PYTHON480_ENABLE) || defined(NANEYE_ENABLE)
}

void initImageSize(void){
	//read 1 pixel, skip 1 pixel in both x and y axis
	#ifdef PYTHON480_608PX_SUBSAMPLE
	image_width = 608;
	image_height = 608;
	subsampleEnable = 1;
	#endif

	#ifdef PYTHON480_400PX_SUBSAMPLE
	image_width = 400;
	image_height = 400;
	subsampleEnable = 1;
	#endif

	#ifdef PYTHON480_320PX_SUBSAMPLE
	image_width = 320;
	image_height = 320;
	subsampleEnable = 1;
	#endif

	#ifdef PYTHON480_304PX_SUBSAMPLE
	image_width = 304;
	image_height = 304;
	subsampleEnable = 1;
	#endif
	// red all pixels
	#ifdef PYTHON480_304PX_NOSUBSAMPLE
	image_width = 304;
	image_height = 304;
	subsampleEnable = 0;
	#endif

	#ifdef PYTHON480_200PX_NOSUBSAMPLE
	image_width = 200;
	image_height = 200;
	subsampleEnable = 0;
	#endif

	#ifdef PYTHON480_160PX_NOSUBSAMPLE
	image_width = 160;
	image_height = 160;
	subsampleEnable = 0;
	#endif

	#ifdef PYTHON480_152PX_NOSUBSAMPLE
	image_width = 152;
	image_height = 152;
	subsampleEnable = 0;
	#endif
}

void calcImaceSize(void){
	num_pixels = ((image_width * image_height) / (subsampleEnable + 1) ^ 2);
}
void peripheralInit(void)
{
	initImageSize();
	calcImaceSize();
	
	#ifdef EXLED_PWM_ENABLE
	// We need to change the PWM mode from MPWM to NPWM because we are using WO[0] as waveform output
	hri_tc_write_WAVE_reg(TC0, TC_WAVE_WAVEGEN_NPWM_Val);
	#endif
	
	#ifdef PYTHON480_ENABLE
	// Enable the 3.3V regulator
	gpio_set_pin_level(EN_3V3, true);
	#endif

	#ifdef BATTERY_ENABLE
	// Enable ADC for checking battery voltage
	adc_sync_enable_channel(&ADC_0, 0);
	#endif
	
	#ifdef WPT_ADC_ENABLE
	// Enable ADC for checking voltage bus voltage
	adc_sync_enable_channel(&ADC_1, 0);
	#endif
	
	#ifdef IR_UART_ENABLE
	usart_async_register_callback(&USART_0, USART_ASYNC_RXC_CB, usart_rx_cb);
	#endif
	
	#ifdef EWL_ENABLE
	I2C_BB_init();
	#endif

	timerInit();
	
	irqInit();
	
	#ifdef PYTHON480_ENABLE
	PCCLinkedListInit();
	#endif

	#ifdef NANEYE_ENABLE
	NELinkedListInit(); // Connects to MS_dma.c
	#endif

	#if defined(DMA_TO_SPI_ENABLE) || defined(DMA_TO_USART_ENABLE)
	TXLinkedListInit();
	#endif
	
	#ifdef DMA_TO_SD_ENABLE
	SDCardInit();
	#endif

	#ifdef PYTHON480_ENABLE
	imageSensorInit();
	#endif
	

	#ifdef EWL_ENABLE
	setEWL(getPropFromHeader(HEADER_EWL_POS));
	//setExcitationLED(getPropFromHeader(HEADER_LED_POS), 1);
	#endif
	
	#ifdef DMA_TO_USART_ENABLE
	sercom_sdo->USART.CTRLA.bit.ENABLE = 0; // Disable UART
	while (sercom_sdo->USART.SYNCBUSY.bit.ENABLE)
	;                                 // Wait for disable
	//hri_sercomusart_write_BAUD_reg(sercom_sdo, USART_BAUD_MS);
	//sercom_sdo->USART.CTRLC.bit.DATA32B = 1; // Enable 32-bit mode. Still packet structure is 8-bit
	sercom_sdo->USART.CTRLA.bit.ENABLE = 1;  // Re-enable USART
	while (sercom_sdo->USART.SYNCBUSY.bit.ENABLE)
	;                                 // Wait for disable
	#endif
	
	#ifdef IR_UART_ENABLE
	sercom_ir->USART.CTRLA.bit.ENABLE = 0;
	while (sercom_ir->USART.SYNCBUSY.bit.ENABLE)
	;
	sercom_ir->USART.CTRLA.bit.ENABLE = 1;
	while (sercom_ir->USART.SYNCBUSY.bit.ENABLE)
	;
	#endif
	
	#ifdef DMA_TO_SPI_ENABLE
	sercom_sdo->SPI.CTRLA.bit.ENABLE = 0; // Disable SPI
	while (sercom_sdo->SPI.SYNCBUSY.bit.ENABLE)
	;                                 // Wait for disable
	#ifdef SPI_ICSPACE_MS
	hri_sercomspi_set_CTRLC_ICSPACE_bf(sercom_sdo, SPI_ICSPACE_MS);
	#endif

	#ifdef SPI_BAUD_MS
	hri_sercomspi_write_BAUD_reg(sercom_sdo, SPI_BAUD_MS);
	#endif

	#ifdef SDO_32BIT_ENABLE
	sercom_sdo->SPI.CTRLC.bit.DATA32B = 1; // Enable 32-bit mode
	#endif
	
	sercom_sdo->SPI.CTRLA.bit.ENABLE = 1;  // Re-enable SPI
	while (sercom_sdo->SPI.SYNCBUSY.bit.ENABLE)
	;                                 // Wait for enable
	#endif

// HS CHECK: MIght need to add something for the NE Camera here:
// if peripheral is running, refuses to change - disables SPI and then waits until SPI gets disabled, and then changes
// the parameters, and then restarts SPI, and restarts peripherals
	/*
	#if defined(DMA_TO_SPI_ENABLE) && defined(SPI_SERCOM0_ENABLE)
	SERCOM0->SPI.CTRLA.bit.ENABLE = 0; // Disable SPI
	while (SERCOM0->SPI.SYNCBUSY.bit.ENABLE)
	;                                 // Wait for disable
	hri_sercomspi_set_CTRLC_ICSPACE_bf(SERCOM0, SPI_ICSPACE_MS);
	hri_sercomspi_write_BAUD_reg(SERCOM0, SPI_BAUD_MS);
	SERCOM0->SPI.CTRLC.bit.DATA32B = 1; // Enable 32-bit mode
	SERCOM0->SPI.CTRLA.bit.ENABLE = 1;  // Re-enable SPI
	while (SERCOM0->SPI.SYNCBUSY.bit.ENABLE)
	;                                 // Wait for disable
	#endif
*/
	
	//Following are codes for preliminary tests. Might not be needed anymore
	#if defined(DMA_TO_SPI_ENABLE) && defined(SPI_SERCOM7_ENABLE)
	hri_sercomspi_set_CTRLC_ICSPACE_bf(SERCOM7, SPI_ICSPACE_MS);
	hri_sercomspi_write_BAUD_reg(SERCOM7, SPI_BAUD_MS);
	spi_m_sync_enable(&SPI_0);
	#endif
	
	#if defined(NODMA_SPI_ENABLE) && defined(SPI_SERCOM0_ENABLE)
	SERCOM0->SPI.CTRLA.bit.ENABLE = 0x01;
	SERCOM0->SPI.DATA.reg = (uint32_t) dataBuffer[0][1];
	#endif
}

void configPropInit(void){
	// Set some parameters in config buffer to be written to SD card at end of recording

}

#ifdef DMA_TO_SD_ENABLE
void SDCardInit(void){
	// Wait for SD Card and then load config from it
	while (SD_MMC_OK != sd_mmc_check(0)) {}
	if (loadSDCardHeader() == MS_SUCCESS)
	deviceState |= DEVICE_STATE_CONFIG_LOADED;
	else
	deviceState |= DEVICE_STATE_ERROR;
	
	// Give capabilities info of sd card
	tempPCC[0] = SDHC0->CA0R.reg;
	tempPCC[1] = SDHC0->CA1R.reg;
	tempPCC[2] = SDHC0->HC1R.reg;
	
	// Select ADMA as the DMA to use. This should be moved to where other bits of HC1R get set.
	SDHC0->HC1R.reg |= 1<<4;
	
	// Set some parameters in config buffer to be written to SD card at end of recording

	setConfigBlockProp(CONFIG_BLOCK_WIDTH_POS, WIDTH / BINNING);
	setConfigBlockProp(CONFIG_BLOCK_HEIGHT_POS, HEIGHT / BINNING);
	setConfigBlockProp(CONFIG_BLOCK_FRAME_RATE_POS, getPropFromHeader(HEADER_FRAME_RATE_POS));
	setConfigBlockProp(CONFIG_BLOCK_BUFFER_SIZE_POS, BUFFER_BLOCK_LENGTH * SD_BLOCK_SIZE);
	setConfigBlockProp(CONFIG_BLOCK_NUM_BUFFERS_RECORDED_POS, 0);
	setConfigBlockProp(CONFIG_BLOCK_NUM_BUFFERS_DROPPED_POS,0);
	
	sd_mmc_init_write_blocks(0, CONFIG_BLOCK, 1);
	sd_mmc_start_write_blocks(configBlock, 1); // We will re-write this block at the end of recording too
	sd_mmc_wait_end_of_write_blocks(false);
}


void setSDDescriptor(uint32_t *address, uint16_t length, uint8_t attribute)
// address holds the pointer location to the front of a data buffer
// Length is in bytes
// attribute holds the lower 6 bits of the descriptor table
{
	uint64_t temp = address;
	temp = temp<<32;
	SDTransferDescriptor = (temp)|attribute|SD_DESCRIPTOR_LENGTH(length);
}

#endif // DMA_TO_SD_ENABLE

void irqInit(void){
	// Setup callbacks for interrupts
	#ifdef IR_TRIGGER_ENABLE
	ext_irq_register(PIN_PB22, irReceive_cb);
	#endif
	
	#ifdef BATTERY_ENABLE
	#ifndef BATTERY_CB_DISABLE
	ext_irq_register(PIN_PB23, battCharging_cb);
	#endif
	#endif
	
	#ifdef PYTHON480_ENABLE
	ext_irq_register(PIN_PB14, frameValid_cb);
	#endif
	
	#ifdef PUSH_BUT_ENABLE
	ext_irq_register(PIN_PA25, pushButton_cb);
	#endif
}

void setConfigBlockProp(uint8_t position, uint32_t value) {
	uint32_t *configBlock32bit = (uint32_t *)configBlock;
	
	configBlock32bit[position] = value;
}

#if defined(PYTHON480_ENABLE)
void setBufferHeader(uint32_t dataWordLength) {
	uint32_t numBuffer = bufferCount % NUM_BUFFERS;
	
	for (uint32_t i = 0; i<DUMMY_WORD_LENGTH; i++)
	{
		dataBuffer[numBuffer][i] = DUMMY_WORD; // Some dummy number that won't come out from the sensor.
	}
	
	#ifdef PREAMBLE_ENABLE
	dataBuffer[numBuffer][BUFFER_HEADER_HEADER_LENGTH_POS + DUMMY_WORD_LENGTH] = PREAMBLE_WORD;
	#else
	dataBuffer[numBuffer][BUFFER_HEADER_HEADER_LENGTH_POS + DUMMY_WORD_LENGTH] = BUFFER_HEADER_LENGTH;
	#endif
	dataBuffer[numBuffer][BUFFER_HEADER_LINKED_LIST_POS + DUMMY_WORD_LENGTH] = bufferCount % NUM_BUFFERS;
	dataBuffer[numBuffer][BUFFER_HEADER_FRAME_NUM_POS + DUMMY_WORD_LENGTH] = frameNum;
	dataBuffer[numBuffer][BUFFER_HEADER_BUFFER_COUNT_POS + DUMMY_WORD_LENGTH] = bufferCount;
	dataBuffer[numBuffer][BUFFER_HEADER_FRAME_BUFFER_COUNT_POS + DUMMY_WORD_LENGTH] = frameBufferCount;
	dataBuffer[numBuffer][BUFFER_HEADER_WRITE_BUFFER_COUNT_POS + DUMMY_WORD_LENGTH] = writeBufferCount;
	dataBuffer[numBuffer][BUFFER_HEADER_DROPPED_BUFFER_COUNT_POS + DUMMY_WORD_LENGTH] = droppedBufferCount;
	dataBuffer[numBuffer][BUFFER_HEADER_TIMESTAMP_POS + DUMMY_WORD_LENGTH] = getCurrentTimeMS() - startTimeMS;
	dataBuffer[numBuffer][BUFFER_HEADER_BATTERY_VOLTAGE_POS + DUMMY_WORD_LENGTH] = battVolt;
	dataBuffer[numBuffer][BUFFER_HEADER_WPT_VOLTAGE_POS + DUMMY_WORD_LENGTH] = wptVolt;
	
	// Note: this assumes a fully filled buffer; the value will differ for a partially filled buffer
	// (UBLEN in XDMAC_CUBC gets decremented by MBSIZE or CSIZE for each memory or chunk transfer, so it can be calculated from this)
	dataBuffer[numBuffer][BUFFER_HEADER_DATA_LENGTH_POS + DUMMY_WORD_LENGTH] = dataWordLength * 4; // In bytes
}
#endif

#ifdef EXLED_PWM_ENABLE
void setExcitationLED(uint32_t value, bool enable)
{
	// Value is a percentage of brightness from 0 to 100.
	// PWM runs at 1ms period using 16bit MAX counter and a ~60MHz clock
	if (value > 100)
	value = 100;
	
	if (value != 0){
		value = (0xFFFF * value ) /100;
		pwm_set_parameters(&PWM_0, value, 0); // value sets duty cycle out of 2^16. We aren't using CC1 so just send it 0
		pwm_enable(&PWM_0); //Only actually needs to be done once. Consider moving to init stuff at top of main()
		gpio_set_pin_level(ENT_LED, enable);		
	}
}
#endif

#ifdef EWL_ENABLE
void setEWL(uint32_t value)
{
	I2C_BB_write(EWL_I2C_ADDR,value);
}
#endif

#ifdef STATUS_LED_ENABLE
void setStatusLED(bool value)
{
	gpio_set_pin_level(LED_STATUS, value);
}
#endif