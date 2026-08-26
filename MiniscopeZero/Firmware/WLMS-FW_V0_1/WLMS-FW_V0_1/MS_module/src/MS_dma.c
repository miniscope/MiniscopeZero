/**
@file
@brief DMA related functions
@author Takuya
*/

#include "MS_config.h"
#include "MS_definitions.h"
#include "dma_custom_driver.h"

#include <hpl_dmac_config.h>
#ifdef PYTHON480_ENABLE
#include <hpl_pcc_config.h>
#endif 
#include <hpl_dma.h>

COMPILER_ALIGNED(16)
volatile DmacDescriptor TXLinkedList[NUM_BUFFERS];

#ifdef NANEYE_ENABLE
COMPILER_ALIGNED(16) // not sure if necessary
volatile DmacDescriptor NE_LinkedList[NUM_BUFFERS];// naneye linked list
#endif

#ifdef PYTHON480_ENABLE
COMPILER_ALIGNED(16)
volatile DmacDescriptor PCCLinkedList[NUM_BUFFERS];
#endif

#ifdef TEST_PRBS_BUFFER_ENABLE
// PRBS-15 generator (x^15 + x^14 + 1), period 32767
static uint16_t prbs15_state;

#define DMAC_SINK_MAX_BEATS (BUFFER_BLOCK_LENGTH * PCC_BLOCK_SIZE_IN_WORDS - (BUFFER_HEADER_LENGTH + DUMMY_WORD_LENGTH))
static uint32_t dmac_sink_buf[DMAC_SINK_MAX_BEATS];


// Reset to a fixed test seed. Using 0 here is fine; we map to 1.
static inline void prbs15_reset_default(void) {
	prbs15_state = 1u;  // change here if you want a different phase
}

// Reset using a caller-supplied seed (e.g. buffer index). 15-bit; 0 maps to 1.
static inline void prbs15_reset_seed(uint16_t seed) {
	prbs15_state = (uint16_t)(seed & 0x7FFFu);
	if (prbs15_state == 0) prbs15_state = 1u;
}

// Return 32 PRBS bits packed MSB-first into a 32-bit word.
static inline uint32_t prbs15_next_word(void) {
	uint32_t w = 0;
	for (int k = 0; k < 32; ++k) {
		uint16_t newbit = ((prbs15_state >> 14) ^ (prbs15_state >> 13)) & 1u; // taps 15,14
		prbs15_state = (uint16_t)(((prbs15_state << 1) | newbit) & 0x7FFFu);  // keep 15 bits
		if (prbs15_state == 0) prbs15_state = 1u;                              // avoid all-zero lockup
		w = (w << 1) | (prbs15_state & 1u);
	}
	return w;
}
#endif

void dmaEnable(void){
	#ifdef PYTHON480_ENABLE
	// Enables DMA Transfer complete interrupt. Should be put in better place
	DMAC->Channel[CONF_PCC_DMA_CHANNEL].CHINTENSET.reg = DMAC_CHINTENSET_TCMPL;
	
	// Sets the callback for when each DMA buffer is full
	camera_async_register_callback(&CAMERA_0, pcc_dma_cb);

	// This should already be done in init but trying here as well
	PCC->MR.reg = PCC_MR_CID(0x3) | PCC_MR_ISIZE(CONF_PCC_ISIZE) | CONF_PCC_FRSTS << PCC_MR_FRSTS_Pos
	| CONF_PCC_HALFS << PCC_MR_HALFS_Pos | CONF_PCC_ALWYS << PCC_MR_ALWYS_Pos
	| CONF_PCC_SCALE << PCC_MR_SCALE_Pos | PCC_MR_DSIZE(CONF_PCC_DSIZE);
	#endif

	#if defined(DMA_TO_SPI_ENABLE) || defined(DMA_TO_USART_ENABLE)
	//NVIC_SetPriority(DMAC_1_IRQn, 0);    // Set the Nested Vector Interrupt Controller (NVIC) priority for DMAC Channel 1
	//NVIC_EnableIRQ(DMAC_1_IRQn);         // Connect DMAC Channel 1 to Nested Vector Interrupt Controller (NVIC)
	DMAC->Channel[SDO_DMA_CHANNEL].CHINTENSET.reg = DMAC_CHINTENSET_TCMPL;
	//DMAC->Channel[SDO_DMA_CHANNEL].CHINTENSET.reg = DMAC_CHINTENSET_SUSP;
	//DMAC->Channel[SDO_DMA_CHANNEL].CHINTENSET.reg = DMAC_CHINTENSET_TERR;
	//DMAC->Channel[SDO_DMA_CHANNEL].CHINTENCLR.reg = 0;                    // Activate the transfer complete (TCMPL) interrupt on DMAC channel 0
	//DMAC->Channel[SDO_DMA_CHANNEL].CHPRILVL.reg = DMAC_CHPRILVL_PRILVL_LVL0;
	dmac_register_callback(SDO_DMA_CHANNEL, sdo_dma_transfer_complete_cb);
	#endif
	// registering the callbacks after DMA transfer, if transfer is completed
	// call the SDO_DMA transfer function
	#if 0 // HS CHECK should be #ifdef NANEYE_ENABLE
	DMAC->Channel[NE_DMA_CHANNEL].CHINTENSET.reg = DMAC_CHINTENSET_TCMPL;
	dmac_register_callback(NE_DMA_CHANNEL, sdo_dma_transfer_complete_cb);// HS CHECK, registers call back function for NanEYE camera
	#endif
	// goes into controlling function, if < 2 buffers, stops
}
// initiating TX linked list
#if defined(DMA_TO_SPI_ENABLE) || defined(DMA_TO_USART_ENABLE)
void TXLinkedListInit(void)
{
	// transfer linked list
	for (uint8_t i = 0; i < NUM_BUFFERS; i++) {
		if (i == (NUM_BUFFERS - 1)) TXLinkedList[i].DESCADDR.reg = (uint32_t)&TXLinkedList[0];
		// Last buffer in list. Need to loop back
		else TXLinkedList[i].DESCADDR.reg = (uint32_t)&TXLinkedList[i + 1];
		
		
		TXLinkedList[i].BTCNT.reg = BUFFER_BLOCK_LENGTH * SDO_BLOCK_SIZE_IN_WORDS;

		// We aren't actually using the STEPSIZE part of incrementing the source address.
		TXLinkedList[i].BTCTRL.reg = DMAC_BTCTRL_STEPSIZE(0) | (CONF_DMAC_STEPSEL_1 << DMAC_BTCTRL_STEPSEL_Pos)\
		| (CONF_DMAC_DSTINC_1 << DMAC_BTCTRL_DSTINC_Pos) | (CONF_DMAC_SRCINC_1 << DMAC_BTCTRL_SRCINC_Pos)\
		| DMAC_BTCTRL_BEATSIZE(CONF_DMAC_BEATSIZE_1) | DMAC_BTCTRL_BLOCKACT(CONF_DMAC_BLOCKACT_1 | 0x01)\
		| DMAC_BTCTRL_EVOSEL(CONF_DMAC_EVOSEL_1) | DMAC_BTCTRL_VALID;
		
		// For sending out data
		#ifdef SDO_32BIT_ENABLE
		TXLinkedList[i].SRCADDR.reg = (uint32_t)(&dataBuffer[i][0]) + TXLinkedList[i].BTCNT.reg * 4;
		#endif
		#ifdef SDO_8BIT_ENABLE
		TXLinkedList[i].SRCADDR.reg = (uint32_t)(&dataBuffer[i][0]) + TXLinkedList[i].BTCNT.reg;
		#endif
		// Destination address when incrementing address needs to be the end address and not the start address.
		// I think the last scale multiplication needs to be either 3 or 5 but _dma_set_data_amount() uses a 4.
		
		// SDO_DATA_REG address should be SERCOM data register. This is defined in MS_definition.h
		//TXLinkedList[i].DSTADDR.reg = (uint32_t) &sercom_sdo->SPI.DATA.reg;
		#ifdef DMA_TO_SPI_ENABLE
		TXLinkedList[i].DSTADDR.reg = (uint32_t) &sercom_sdo->SPI.DATA.reg;
		#elif defined(DMA_TO_USART_ENABLE)
		TXLinkedList[i].DSTADDR.reg = (uint32_t) &sercom_sdo->USART.DATA.reg;
		#endif
	}
	setTXLinkedListPosition(0);
}

#ifdef NANEYE_ENABLE

void NELinkedListInit(void)
{
	for (uint8_t i = 0; i < NUM_BUFFERS; i++) {
		if (i == (NUM_BUFFERS - 1)) NE_LinkedList[i].DESCADDR.reg = (uint32_t)&TXLinkedList[0];
		// Last buffer in list. Need to loop back
		else NE_LinkedList[i].DESCADDR.reg = (uint32_t)&NE_LinkedList[i + 1];
		
		// Daniel definition of linked list
		NE_LinkedList[i].BTCNT.reg = BUFFER_BLOCK_LENGTH * SDO_BLOCK_SIZE_IN_WORDS;

		// We aren't actually using the STEPSIZE part of incrementing the source address.
		NE_LinkedList[i].BTCTRL.reg = DMAC_BTCTRL_STEPSIZE(0) | (CONF_DMAC_STEPSEL_1 << DMAC_BTCTRL_STEPSEL_Pos)\
		| (CONF_DMAC_DSTINC_1 << DMAC_BTCTRL_DSTINC_Pos) | (CONF_DMAC_SRCINC_1 << DMAC_BTCTRL_SRCINC_Pos)\
		| DMAC_BTCTRL_BEATSIZE(CONF_DMAC_BEATSIZE_1) | DMAC_BTCTRL_BLOCKACT(CONF_DMAC_BLOCKACT_1 | 0x01)\
		| DMAC_BTCTRL_EVOSEL(CONF_DMAC_EVOSEL_1) | DMAC_BTCTRL_VALID;
		
		// For sending out data
		// DANIEL I think this is where the linked list get sent to a particular address
		#ifdef SDO_32BIT_ENABLE
		NE_LinkedList[i].DSTADDR.reg = (uint32_t)(&dataBuffer[i][0]) + NE_LinkedList[i].BTCNT.reg * 4;
		#endif
		#ifdef SDO_8BIT_ENABLE
		NE_LinkedList[i].DSTADDR.reg = (uint32_t)(&dataBuffer[i][0]) + NE_LinkedList[i].BTCNT.reg;
		#endif
		// Destination address when incrementing address needs to be the end address and not the start address.
		// I think the last scale multiplication needs to be either 3 or 5 but _dma_set_data_amount() uses a 4.
		
		NE_LinkedList[i].SRCADDR.reg = (uint32_t) &SERCOM4->SPI.DATA.reg; // SERCOM for NE Camera HS CHECK
// make sure to associate this SERCOM number in ATMEL Start
	}
	setNELinkedListPosition(0);
}

void setNELinkedListPosition(uint8_t pos)
{
	_dma_set_destination_address(NE_DMA_CHANNEL, (void *)NE_LinkedList[pos].DSTADDR.reg);
	_dma_set_data_amount(NE_DMA_CHANNEL, NE_LinkedList[pos].BTCNT.reg);
	_dma_set_BTCTRL(NE_DMA_CHANNEL, (void *)NE_LinkedList[pos].BTCTRL.reg); //block transfer control
	_dma_set_DESCADDR(NE_DMA_CHANNEL, NE_LinkedList[pos].DESCADDR.reg);
	_dma_set_source_address(NE_DMA_CHANNEL, (void *)NE_LinkedList[pos].SRCADDR.reg); // Overwrite source address since set_data_amount function modifies this
}

#endif

void setTXLinkedListPosition(uint8_t pos)
{
	_dma_set_destination_address(SDO_DMA_CHANNEL, (void *)TXLinkedList[pos].DSTADDR.reg);
	_dma_set_data_amount(SDO_DMA_CHANNEL, TXLinkedList[pos].BTCNT.reg);
	_dma_set_BTCTRL(SDO_DMA_CHANNEL, (void *)TXLinkedList[pos].BTCTRL.reg); //block transfer control
	_dma_set_DESCADDR(SDO_DMA_CHANNEL, TXLinkedList[pos].DESCADDR.reg);
	_dma_set_source_address(SDO_DMA_CHANNEL, (void *)TXLinkedList[pos].SRCADDR.reg); // Overwrite source address since set_data_amount function modifies this
}


void sdo_dma_transfer_trigger(void)
{
	DMAC->SWTRIGCTRL.reg = 0x2;
}

void sdo_dma_transfer_complete_cb(void)
{
	// call count for debug
	sdo_dma_cb_calls++;
	
	//increment if appropriate
	sdo_dma_transfer_control(true);
}
// HS CHECK with naneye camera here
void sdo_dma_transfer_control(bool callback_flag) // flag if called via callback
{
		// for first call; if not enabled
	if (DMAC->Channel[SDO_DMA_CHANNEL].CHCTRLA.bit.ENABLE == 0)
	{
		_dma_enable_transaction(SDO_DMA_CHANNEL, false);
		writeBufferCount++; // not sure if this should be counted
		return;
	}
	if (bufferCount - (writeBufferCount + droppedBufferCount) > 0){
		#ifdef PYTHON480_ENABLE
		// send out pending bits and return
		if(DMAC->Channel[SDO_DMA_CHANNEL].CHSTATUS.bit.PEND == 1){
			sdo_dma_transfer_resume();
			return;
		}

		//end if middle of transfer
		if(DMAC->Channel[SDO_DMA_CHANNEL].CHSTATUS.bit.BUSY == 1){
			return;
		}	

		// if coming from TRCMP callback and buffer is left just resume
		if(callback_flag == 1 && bufferCount - (writeBufferCount + droppedBufferCount) > 0){
			sdo_dma_transfer_resume();
			return;
		}
		#endif
	}


	#ifndef PYTHON480_ENABLE //just send out
	if(DMAC->Channel[SDO_DMA_CHANNEL].CHCTRLA.bit.ENABLE == 0)
	{
		_dma_enable_transaction(SDO_DMA_CHANNEL, false);
	}
	sdo_dma_transfer_resume();
	#endif
}
void sdo_dma_transfer_resume(void)
{
	writeBufferCount++;
	DMAC->Channel[SDO_DMA_CHANNEL].CHCTRLB.reg = 0x2;
	//sdo_dma_transfer_trigger(); // SERCOM 5 is triggering so not necessary
}

void sdo_dma_transfer_suspend(void)
{
	DMAC->Channel[SDO_DMA_CHANNEL].CHCTRLB.reg = 0x1;
}
#endif

void DataBufferInit(void)
{
	for (uint32_t i = 0; i<NUM_BUFFERS; i++)
	{
		dataBuffer[i][0] = PREAMBLE_WORD;
		#ifdef TEST_PRBS_BUFFER_ENABLE
		// Seed PRBS with buffer index so each buffer has a distinct, reproducible pattern.
		prbs15_reset_seed((uint16_t)i);
		#endif
		for (uint32_t j = 1; j<BUFFER_BLOCK_LENGTH * PCC_BLOCK_SIZE_IN_WORDS; j++)
		{
            #ifdef TEST_PRBS_BUFFER_ENABLE
            if (j >= (BUFFER_HEADER_LENGTH + DUMMY_WORD_LENGTH)) {
	            dataBuffer[i][j] = prbs15_next_word();
	            } else {
	            dataBuffer[i][j] = 0;
            }
            #else
            dataBuffer[i][j] = 0;
            #endif
		}
	}
}
#ifdef PYTHON480_ENABLE
void PCCLinkedListInit(void)
{
	for (uint8_t i = 0; i < NUM_BUFFERS; i++) {
		if (i == (NUM_BUFFERS - 1))
		// Last buffer in list. Need to loop back
		PCCLinkedList[i].DESCADDR.reg = (uint32_t)&PCCLinkedList[0];
		else
		PCCLinkedList[i].DESCADDR.reg = (uint32_t)&PCCLinkedList[i + 1];
		
		PCCLinkedList[i].BTCNT.reg = (BUFFER_BLOCK_LENGTH * PCC_BLOCK_SIZE_IN_WORDS - (BUFFER_HEADER_LENGTH + DUMMY_WORD_LENGTH));
		// We aren't actually using the STEPSIZE part of incrementing the destination address.
		PCCLinkedList[i].BTCTRL.reg = DMAC_BTCTRL_STEPSIZE(0) | (CONF_DMAC_STEPSEL_0 << DMAC_BTCTRL_STEPSEL_Pos)\
		| (CONF_DMAC_DSTINC_0 << DMAC_BTCTRL_DSTINC_Pos) | (CONF_DMAC_SRCINC_0 << DMAC_BTCTRL_SRCINC_Pos)\
		| DMAC_BTCTRL_BEATSIZE(CONF_DMAC_BEATSIZE_0) | DMAC_BTCTRL_BLOCKACT(CONF_DMAC_BLOCKACT_0 | 0x01)\
		| DMAC_BTCTRL_EVOSEL(CONF_DMAC_EVOSEL_0) | DMAC_BTCTRL_VALID;
		
		PCCLinkedList[i].SRCADDR.reg = (uint32_t)(&PCC->RHR.reg); //(void *)&(((Pcc *)device->hw)->RHR.reg)
		
		
		#ifdef TEST_PRBS_BUFFER_ENABLE
		uint32_t beats = PCCLinkedList[i].BTCNT.reg;   // number of 32-bit words
		PCCLinkedList[i].DSTADDR.reg = (uint32_t)(dmac_sink_buf + beats);
		#else
		// Destination address when incrementing address needs to be the end address and not the start address.
		// I think the last scale multiplication needs to be either 3 or 5 but _dma_set_data_amount() uses a 4.
		PCCLinkedList[i].DSTADDR.reg = (uint32_t)(&dataBuffer[i][BUFFER_HEADER_LENGTH + DUMMY_WORD_LENGTH]) + PCCLinkedList[i].BTCNT.reg * 4;
		#endif
	}
	setPCCLinkedListPosition(0);
}

void setPCCLinkedListPosition(uint8_t pos)
{
	// Set up initial DMA descriptor for DMA channel handling PCC. BTCNT is already setup in DMA init step
	_dma_set_source_address(CONF_PCC_DMA_CHANNEL, (void *)PCCLinkedList[pos].SRCADDR.reg);
	//_dma_set_destination_address(CONF_PCC_DMA_CHANNEL, (void *)PCCLinkedList[pos].DSTADDR.reg);
	_dma_set_data_amount(CONF_PCC_DMA_CHANNEL, (void *)PCCLinkedList[pos].BTCNT.reg);
	_dma_set_BTCTRL(CONF_PCC_DMA_CHANNEL, (void *)PCCLinkedList[pos].BTCTRL.reg);
	_dma_set_destination_address(CONF_PCC_DMA_CHANNEL, (void *)PCCLinkedList[pos].DSTADDR.reg); // Overwrite destination address since set_data_amount function modifies this

	_dma_set_DESCADDR(CONF_PCC_DMA_CHANNEL, PCCLinkedList[pos].DESCADDR.reg);
}
#endif
