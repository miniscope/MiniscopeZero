// Daniel NanEye.c code is copied from the NanEye SAME70 code
// and has not been properly ported from SAME70 to this SAMD framework

#include "MS_config.h"
#include "MS_definitions.h"


#ifdef NANEYE_ENABLE
#include "NanEye.h"
// send data
void NanEyeInit(void)
{
	// Let's start naneye communication. To do this we will finish setting up the interface mode stuff and then turn on the DMAs
	naneye_new_reg_val_received = 1;
	
	naneyec_reg_val[0] = NANEYE_REG0_DEFAULT_VALUE | 0b1100; // Sets offset ramp to recommended 2.2V value
	naneyec_reg_val[0] = naneyec_reg_val[0] | 0b11; // Sets output current to max (might only effect LVDS mode)
	// sets naneye register
	// |= sets particular value to 1
	// &= with a ~ means and not, sets value to 0
	naneyec_reg_val[1] = NANEYE_REG1_DEFAULT_VALUE;
	naneyec_reg_val[1] |= (1<<10); // Increase 2x bias current, reduces settling time for high speed apps (not sure what this does)
	naneyec_reg_val[1] &= ~(1<<9); // Sets CDS gain to recommended value of 1.3 (turns a 1--> 0, which sets CDS gain to 1.3)
	naneyec_reg_val[1] &= ~(1<<8); // Sets mode to SEIM
	naneyec_reg_val[1] = (naneyec_reg_val[1] & (0b1111111111001111)) | (0b10 << 4); // Sets vref to recommended value of 2.1V
	naneyec_reg_val[1] = (naneyec_reg_val[1] & (0b1111111111110011)) | (0b01 << 2); // Sets CVC current to recommended value
	naneyec_reg_val[1] &= ~(1<<1); // Turns off idle mode
	
	interface_buffer_reg_set(naneyec_reg_val[0],naneyec_reg_val[1]);
	
	for (int i = 0; i<INTERFACE_MODE_SIZE; i++)
	{
		SERCOM4->SPI.DATA.reg = spi_interface_mode_tx_buffer[i];
	}
	//	SERCOM4->SPI.DATA.reg = naneyec_reg_val[1];
}
void startRecordingNE()
{
	writeFrameNum=0;
	writeBufferCount=0;
	droppedBufferCount= 0;
	droppedFrameCount = 0;
	framesToDrop = 0;
	
	deviceState &= ~(DEVICE_STATE_IDLE);
	deviceState &= ~(DEVICE_STATE_START_RECORDING);
	deviceState |= DEVICE_STATE_START_RECORDING_WAITING;
	
	
}

void stopRecordingNE()
{
	deviceState &= ~(DEVICE_STATE_STOP_RECORDING);
	deviceState &= ~(DEVICE_STATE_RECORDING);
	deviceState |= DEVICE_STATE_IDLE;
	setConfigBlockProp(CONFIG_BLOCK_NUM_BUFFERS_RECORDED_POS, writeBufferCount);
	setConfigBlockProp(CONFIG_BLOCK_NUM_BUFFERS_DROPPED_POS, droppedBufferCount);
}

#endif // NANEYE_ENABLE


// Below is Daniel's original NE code for SAME70 
#if 0
volatile uint32_t state = STATE_IDLE; // holds the current state of the SPI Naneye state machine

volatile uint32_t ms_ticks = 0; // Should count up in 1 ms increments
volatile uint32_t tick_factor = 1000; // Used to configure SysTick to interrupt at a specific interval

volatile uint32_t us_ticks = 0; // Should count up in 1 ms increments
volatile uint32_t us_tick_factor = 1000000; // Used to configure SysTick to interrupt at a specific interval

volatile uint32_t spi_dma_trash[2]; // Used for tx and rx in DMA when we don't need to save the data

volatile uint8_t spi_interface_mode_tx_buffer[INTERFACE_MODE_SIZE]; // Used to tx during interface mode
volatile uint8_t spi_initial_interface_mode_tx_buffer[INITIAL_INTERFACE_MODE_SIZE]; // Used to tx during interface mode

volatile uint8_t spi_readout_mode_rx_buffer[BUFFER_FRAME_SIZE][READOUT_MODE_SIZE];

// static xdmac_channel_config_t xdmac_cfg[8]; //XDMAC channel configuration.

volatile uint32_t spi_buffer_write_index = 0;

volatile uint8_t naneye_new_reg_val_received = 0;
volatile uint16_t naneyec_reg_val[2];

volatile bool frame_requested = false;
volatile bool debug_mode = false;

volatile uint32_t clk_freq;
// ------------------------------------------
void NanEyeRUN (void); // DANIEL Added the main loop function here

// -------------- Functions -----------------
void systick_init(void); // DANIEL Probably dont need this

void spi_init(void); // DANIEL Probably dont need this
void spi_xdmac_configure(Spi *const pspi); // DANIEL Probably dont need this
void spi_disable_xdmac(void); 
void spi_enable_xdmac(void); 

void interface_buffer_init(void);
void initial_interface_buffer_init(void);
void interface_buffer_reg_set(uint32_t, uint32_t);
void interface_buffer_reg_clear(void);

void spi_config_interface_mode(void); // need to switch polarity and not tristate MOSI
void spi_config_readout_mode(void);   // need to switch polarity and tristate MOSI
void spi_config_initial_interface_mode(void); // HS added for initializing IIM (initial interface mode)
void uart_handle_input(void);  // DANIEL Probably dont need this
// ------------------------------------------

void interface_buffer_init(void) {
	// Fill interface tx buffer with all 1's
	for (uint32_t i = 0; i < INTERFACE_MODE_SIZE; i++)
	spi_interface_mode_tx_buffer[i] = 0xFF;
}
void initial_interface_buffer_init(void) {
	// Fill interface tx buffer with all 0's
	for (uint32_t i = 0; i < INITIAL_INTERFACE_MODE_SIZE; i++)
	spi_interface_mode_tx_buffer[i] = 0x00;
}
void interface_buffer_reg_set(uint32_t reg0, uint32_t reg1) {
	// Cannot update regs in the first SPI clock of interface mode so we will start in the second byte location
	
	// For reg0
	spi_interface_mode_tx_buffer[1] = 0b10010000 | ((reg0 >> 15) & 0x01);
	spi_interface_mode_tx_buffer[2] = ((reg0 >> 7) & 0xFF);
	spi_interface_mode_tx_buffer[3] = ((reg0 << 1) & 0xFF);
	
	// For reg1
	// Lets give a 3 byte gap between writing the 2 registers
	spi_interface_mode_tx_buffer[7] = 0b10010010 | ((reg1 >> 15) & 0x01);
	spi_interface_mode_tx_buffer[8] = ((reg1 >> 7) & 0xFF);
	spi_interface_mode_tx_buffer[9] = ((reg1 << 1) & 0xFF);
}

void interface_buffer_reg_clear(void) {
	for (uint8_t i = 1; i < 10; i++)
	spi_initial_interface_mode_tx_buffer[i] = 0xFF;
}
// instead of using atmel start, this is written from scratch
// Daniel original NE code, configuration of MISO/MOSI pins, might be redundant
// this might be included in atmel_start_pins.h
// also I have some SPI initialization stuff at the end of this document

void spi_init(void) {
		// DANIEL THIS SHOULD BE IOPORT FROM THE atmel_start_pin.h or configuration, SEE BELOW 3 lines
// 	#define PB13 GPIO(GPIO_PORTB, 13) // DANIEL SCK
// 	#define PB14 GPIO(GPIO_PORTB, 14) // DANIEL MISO
// 	#define PB15 GPIO(GPIO_PORTB, 15) // DANIEL MO
	// Initial setup of SPI peripheral. We will need to tristate MOSI during Interface Mode
	ioport_set_pin_mode(SPI_MISO_PIN,SPI_MISO_MODE);
	ioport_disable_pin(SPI_MISO_PIN);
	ioport_set_pin_mode(SPI_MOSI_PIN,SPI_MOSI_MODE);
	ioport_disable_pin(SPI_MOSI_PIN);
	ioport_set_pin_mode(SPI_SPCK_PIN,SPI_SPCK_MODE);
	ioport_disable_pin(SPI_SPCK_PIN);
	ioport_set_pin_mode(SPI_NPCS_PIN,SPI_NPCS_MODE);
	ioport_disable_pin(SPI_NPCS_PIN);
	
	
// 	pmc_enable_periph_clk(SPI_ID);// turns on module/clock DANIEL Probably dont need

	SPI_MASTER_BASE->SPI_CR = SPI_CR_SPIDIS; //Disabled SPI
	SPI_MASTER_BASE->SPI_CR = SPI_CR_SWRST; //Resets SPI
	SPI_MASTER_BASE->SPI_CR = SPI_CR_LASTXFER; //De-asserts NPCS after every TD transfer.
	SPI_MASTER_BASE->SPI_MR |= SPI_MR_MSTR; //Sets to master mode
	SPI_MASTER_BASE->SPI_MR |= SPI_MR_MODFDIS; //Disables fault detection
	
	// Sets Peripheral Chip Select (chip select selects the integrated device circuit)
	// The Naneye doesn't actually use chip select but might be still nice to have for debugging on a scope
	SPI_MASTER_BASE->SPI_MR &= (~SPI_MR_PCS_Msk); //Empties out the PCS bits
	SPI_MASTER_BASE->SPI_MR |= SPI_MR_PCS(SPI_CHIP_PCS); //Sets the PCS bits
	
	// Sets Polarity for chip select
	// 0 means the clock is normally resting at 0
	SPI_MASTER_BASE->SPI_CSR[SPI_CHIP_SEL] &= (~SPI_CSR_CPOL); //0
	
	// Peripheral chip select line rises after each transfer for a minimum of DLYBCS
	//SPI_MASTER_BASE->SPI_CSR[SPI_CHIP_SEL] |= SPI_CSR_CSNAAT;
	
	//Bits Per Transfer
	SPI_MASTER_BASE->SPI_CSR[SPI_CHIP_SEL] &= (~SPI_CSR_BITS_Msk);
	SPI_MASTER_BASE->SPI_CSR[SPI_CHIP_SEL] |= SPI_CSR_BITS_8_BIT;

	//Set BAUD rate div CHECK ACTUAL SCLK OUTPUT!!!
	SPI_MASTER_BASE->SPI_CSR[SPI_CHIP_SEL] &= (~SPI_CSR_SCBR_Msk);
	SPI_MASTER_BASE->SPI_CSR[SPI_CHIP_SEL] |= SPI_CSR_SCBR(sysclk_get_peripheral_hz()/ SPI_CLK_RATE);

	//Set Delays
	SPI_MASTER_BASE->SPI_CSR[SPI_CHIP_SEL] &= ~(SPI_CSR_DLYBS_Msk | SPI_CSR_DLYBCT_Msk);
	SPI_MASTER_BASE->SPI_CSR[SPI_CHIP_SEL] |= SPI_CSR_DLYBS(SPI_DLYBS) | SPI_CSR_DLYBCT(SPI_DLYBCT);
	
	//Sets Delay of NPCS is held high between transfers
	SPI_MASTER_BASE->SPI_MR &= (~SPI_MR_DLYBCS_Msk);
	SPI_MASTER_BASE->SPI_MR |= SPI_MR_DLYBCS(SPI_DLYBCS);
	
	
	SPI_MASTER_BASE->SPI_CR = SPI_CR_SPIEN; //Enable SPI!
}

void spi_config_readout_mode(void) {
	// Set MOSI as tristate and set phase
	// DANIEL, this should be changed to the SAMD framework
	SPI_MASTER_BASE->SPI_CR = SPI_CR_SPIDIS; //Disabled SPI
	
	ioport_enable_pin(SPI_MOSI_PIN); // This should give PIO control back of the pin
	ioport_set_pin_dir(SPI_MOSI_PIN, IOPORT_DIR_INPUT);
	
	
	// In readout mode the naneye transmits a new bit on reach rising redge
	SPI_MASTER_BASE->SPI_CSR[SPI_CHIP_SEL] &= (~SPI_CSR_NCPHA); //0
	
	SPI_MASTER_BASE->SPI_CR = SPI_CR_SPIEN; //Enable SPI!
}

void spi_xdmac_configure(Spi *const pspi)
{
	// Here we will configure all 6 DMA transfer types
	// We need to toggle on and off the tristate for MOSI so using linked lists won't be too useful

	pmc_enable_periph_clk(ID_XDMAC);  // DANIEL probably dont need this function
	
	// Set transfer size
	xdmac_cfg[INTERFACE_MODE_TX_POS].mbr_ubc = INTERFACE_MODE_SIZE;
	xdmac_cfg[INTERFACE_MODE_RX_POS].mbr_ubc = INTERFACE_MODE_SIZE;
	// set delay mode size
	xdmac_cfg[SYNC_AND_DELAY_MODE_TX_POS].mbr_ubc = SYNC_MODE_SIZE + DELAY_MODE_MIN_SIZE;
	xdmac_cfg[SYNC_AND_DELAY_MODE_RX_POS].mbr_ubc = SYNC_MODE_SIZE + DELAY_MODE_MIN_SIZE;
	// set readout mode size
	xdmac_cfg[READOUT_MODE_TX_POS].mbr_ubc = READOUT_MODE_SIZE;
	xdmac_cfg[READOUT_MODE_RX_POS].mbr_ubc = READOUT_MODE_SIZE;
	// set readout mode size
	xdmac_cfg[INITIAL_INTERFACE_MODE_TX_POS].mbr_ubc = INITIAL_INTERFACE_MODE_SIZE;
	xdmac_cfg[INITIAL_INTERFACE_MODE_RX_POS].mbr_ubc = INITIAL_INTERFACE_MODE_SIZE;
	
	
	// Set source (interface mode) address
	xdmac_cfg[INTERFACE_MODE_TX_POS].mbr_sa = (uint32_t)&spi_interface_mode_tx_buffer[0]; // memory to memory
	xdmac_cfg[INTERFACE_MODE_RX_POS].mbr_sa = (uint32_t)&pspi->SPI_RDR; // receive register of the naneye
	// set sync mode address
	xdmac_cfg[SYNC_AND_DELAY_MODE_TX_POS].mbr_sa = (uint32_t)&spi_dma_trash[0];
	xdmac_cfg[SYNC_AND_DELAY_MODE_RX_POS].mbr_sa = (uint32_t)&pspi->SPI_RDR;
	// set readout mode address
	xdmac_cfg[READOUT_MODE_TX_POS].mbr_sa = (uint32_t)&spi_dma_trash[0];
	xdmac_cfg[READOUT_MODE_RX_POS].mbr_sa = (uint32_t)&pspi->SPI_RDR; // naneye camera read in is set to spi_rdr (read data register for SPI)
	// set readout mode address
	xdmac_cfg[INITIAL_INTERFACE_MODE_TX_POS].mbr_sa = (uint32_t)&spi_interface_mode_tx_buffer[0];
	xdmac_cfg[INITIAL_INTERFACE_MODE_RX_POS].mbr_sa = (uint32_t)&pspi->SPI_RDR; // naneye camera read in is set to spi_rdr (read data register for SPI)
	
	// Set destination interface mode address
	xdmac_cfg[INTERFACE_MODE_TX_POS].mbr_da = (uint32_t)&(pspi->SPI_TDR);
	xdmac_cfg[INTERFACE_MODE_RX_POS].mbr_da = (uint32_t)&spi_dma_trash[1];
	// set sync and delay mode address
	xdmac_cfg[SYNC_AND_DELAY_MODE_TX_POS].mbr_da = (uint32_t)&(pspi->SPI_TDR);
	xdmac_cfg[SYNC_AND_DELAY_MODE_RX_POS].mbr_da = (uint32_t)&spi_dma_trash[1];
	// set readout mode address
	xdmac_cfg[READOUT_MODE_TX_POS].mbr_da = (uint32_t)&(pspi->SPI_TDR);
	xdmac_cfg[READOUT_MODE_RX_POS].mbr_da = (uint32_t)&spi_readout_mode_rx_buffer[spi_buffer_write_index][0]; // This will change to different buffer positions during frame acq
	// set initial interface mode address
	xdmac_cfg[INITIAL_INTERFACE_MODE_TX_POS].mbr_da = (uint32_t)&(pspi->SPI_TDR);
	xdmac_cfg[INITIAL_INTERFACE_MODE_RX_POS].mbr_da = (uint32_t)&spi_dma_trash[1]; // This will change to different buffer positions during frame acq
	
	// Set rest of DMA config values
	// NOTE: Potentially can increase DWIDTH, CHUCK size, and/or MBSIZE to improve performance but not sure
	uint32_t common_tx_cfg = XDMAC_CC_TYPE_PER_TRAN |
	XDMAC_CC_MBSIZE_SINGLE |
	XDMAC_CC_DSYNC_MEM2PER |
	XDMAC_CC_CSIZE_CHK_1 |
	XDMAC_CC_DWIDTH_BYTE |
	XDMAC_CC_SIF_AHB_IF0 |
	XDMAC_CC_DIF_AHB_IF1 |
	XDMAC_CC_DAM_FIXED_AM |
	XDMAC_CC_PERID(SPI0_XDMAC_TX_CH_NUM);
	
	uint32_t common_rx_cfg = XDMAC_CC_TYPE_PER_TRAN |
	XDMAC_CC_MBSIZE_SINGLE |
	XDMAC_CC_DSYNC_PER2MEM |
	XDMAC_CC_CSIZE_CHK_1 |
	XDMAC_CC_DWIDTH_BYTE|
	XDMAC_CC_SIF_AHB_IF1 |
	XDMAC_CC_DIF_AHB_IF0 |
	XDMAC_CC_SAM_FIXED_AM |
	XDMAC_CC_PERID(SPI0_XDMAC_RX_CH_NUM);
	//
	xdmac_cfg[INTERFACE_MODE_TX_POS].mbr_cfg = common_tx_cfg | XDMAC_CC_SAM_INCREMENTED_AM;
	xdmac_cfg[INTERFACE_MODE_RX_POS].mbr_cfg = common_rx_cfg | XDMAC_CC_DAM_FIXED_AM;
	
	xdmac_cfg[SYNC_AND_DELAY_MODE_TX_POS].mbr_cfg = common_tx_cfg | XDMAC_CC_SAM_FIXED_AM;
	xdmac_cfg[SYNC_AND_DELAY_MODE_RX_POS].mbr_cfg = common_rx_cfg | XDMAC_CC_DAM_FIXED_AM;
	
	xdmac_cfg[READOUT_MODE_TX_POS].mbr_cfg = common_tx_cfg | XDMAC_CC_SAM_FIXED_AM;
	xdmac_cfg[READOUT_MODE_RX_POS].mbr_cfg = common_rx_cfg | XDMAC_CC_DAM_INCREMENTED_AM;
	
	xdmac_cfg[INITIAL_INTERFACE_MODE_TX_POS].mbr_cfg = common_tx_cfg | XDMAC_CC_SAM_FIXED_AM;
	xdmac_cfg[INITIAL_INTERFACE_MODE_RX_POS].mbr_cfg = common_rx_cfg | XDMAC_CC_DAM_INCREMENTED_AM;


	// Setting some final dma config stuff
	for (uint8_t i = 0; i < 6; i++) {
		xdmac_cfg[i].mbr_bc = 0;
		xdmac_cfg[i].mbr_ds =  0;
		xdmac_cfg[i].mbr_sus = 0;
		xdmac_cfg[i].mbr_dus = 0;
	}

	// Set up first DMA tx and rx configuration
	// Can remove this
	xdmac_configure_transfer(XDMAC, XDMAC_TX_CH, &xdmac_cfg[INTERFACE_MODE_TX_POS]);
	xdmac_configure_transfer(XDMAC, XDMAC_RX_CH, &xdmac_cfg[INTERFACE_MODE_RX_POS]);

	

}

// NOTE: No idea of switching the mode of the MOSI pin is allowed
void spi_config_interface_mode(void) {
	// Set MOSI as output and set phase
	
	SPI_MASTER_BASE->SPI_CR = SPI_CR_SPIDIS; //Disabled SPI
	// sets the pin to SPI MISO/MOSI pinsO
	ioport_set_pin_mode(SPI_MOSI_PIN,SPI_MOSI_MODE);
	ioport_disable_pin(SPI_MOSI_PIN);
	
	// In interface mode the naneye samples the data pin on the rising edge
	SPI_MASTER_BASE->SPI_CSR[SPI_CHIP_SEL] |= SPI_CSR_NCPHA; //1
	
	SPI_MASTER_BASE->SPI_CR = SPI_CR_SPIEN; //Enable SPI!
}


// NOTE: No idea of switching the mode of the MOSI pin is allowed
void spi_config_initial_interface_mode(void) {
	// Set MOSI as output and set phase
	SPI_MASTER_BASE->SPI_CR = SPI_CR_SPIDIS; //Disabled SPI
	// sets the pin to SPI MISO/MOSI pinsO
	ioport_set_pin_mode(SPI_MOSI_PIN,SPI_MOSI_MODE);
	ioport_disable_pin(SPI_MOSI_PIN);
	
	// In interface mode the naneye samples the data pin on the rising edge
	SPI_MASTER_BASE->SPI_CSR[SPI_CHIP_SEL] |= SPI_CSR_NCPHA; //1

	SPI_MASTER_BASE->SPI_CR = SPI_CR_SPIEN; //Enable SPI!
	
	SPI_MASTER_BASE->SPI_TDR= 0b00; // HS added sends blank signal to initialize, per NE sequence of operation
	// 	SPI_MASTER_BASE->SPI_TDR= 0b00; // HS added sends blank signal to initialize, per NE sequence of operation

}


void spi_enable_xdmac(void) {
	// This will enable the interface, sync_and_delay, or readout mode
	uint32_t xdmaint;

	xdmaint = (XDMAC_CIE_BIE);// |
	//XDMAC_CIE_DIE   |
	//XDMAC_CIE_FIE   |
	//XDMAC_CIE_RBIE  |
	//XDMAC_CIE_WBIE  |
	//XDMAC_CIE_ROIE);
	
	xdmac_channel_set_descriptor_control(XDMAC, XDMAC_TX_CH, 0);
	xdmac_channel_enable_interrupt(XDMAC, XDMAC_TX_CH, xdmaint);
	xdmac_enable_interrupt(XDMAC, XDMAC_TX_CH);
	//xdmac_channel_enable(XDMAC, XDMAC_TX_CH);
	
	
	xdmac_channel_set_descriptor_control(XDMAC, XDMAC_RX_CH, 0);
	xdmac_channel_enable_interrupt(XDMAC, XDMAC_RX_CH, xdmaint);
	xdmac_enable_interrupt(XDMAC, XDMAC_RX_CH);
	//xdmac_channel_enable(XDMAC, XDMAC_RX_CH);
	

	XDMAC->XDMAC_GE = (XDMAC_GE_EN0 << XDMAC_TX_CH);// starting tx/rx dma after configuring earlier
	XDMAC->XDMAC_GE = (XDMAC_GE_EN0 << XDMAC_RX_CH);
	

}
void spi_disable_xdmac(void)
{
	uint32_t xdmaint;
	
	xdmaint = (XDMAC_CIE_BIE);// |
	//XDMAC_CIE_DIE   |
	//XDMAC_CIE_FIE   |
	//XDMAC_CIE_RBIE  |
	//XDMAC_CIE_WBIE  |
	//XDMAC_CIE_ROIE);

	//xdmac_channel_disable_interrupt(XDMAC, XDMAC_RX_CH, xdmaint);
	xdmac_channel_disable(XDMAC, XDMAC_RX_CH);
	//xdmac_disable_interrupt(XDMAC, XDMAC_RX_CH);
	//
	//xdmac_channel_disable_interrupt(XDMAC, XDMAC_TX_CH, xdmaint);
	xdmac_channel_disable(XDMAC, XDMAC_TX_CH);
	//xdmac_disable_interrupt(XDMAC, XDMAC_TX_CH);

	//NVIC_ClearPendingIRQ(XDMAC_IRQn);
	//NVIC_DisableIRQ(XDMAC_IRQn);
}
//
void uart_handle_input(void) {
	
	uint8_t data;
	// HS translation: if usart reads a uint32_t vector that has appropriate data,
	// this goes into switch/if loop for user request over serial
	if (usart_read(CONF_UART, (uint32_t *)&data) == 0) {
		// New data received
		switch (data) {
			case ('f'): // send a frame
			frame_requested = true;
			break;
			case ('d'):
			if (debug_mode == false)
			debug_mode = true;
			else
			debug_mode = false;
			break;
			case ('r'):
			
			break;
			case ('s'):
			
			break;
		}
	}
}
// --------------- TODO LIST: ------------------
// - Detect when clocking is shifted or off and somehow fix it
// - Set up USART
// - Make sure SPI config is correct
// - Figure out tristating MOSI
// - documentation for bit shift register changes
// DANIEL main function for NE camera, Initial interface mode here as well
// we should probably call this something different, or put this in the main loop
// int main (void)
int NanEyeRUN (void)
{

	uint32_t lastToggleTime;
	
	WDT->WDT_MR = WDT_MR_WDDIS; //Disables Watch dog timer, independent timer
	
	// Init interrupt handling
	irq_initialize_vectors();
	cpu_irq_enable();
	
	// Init system clock and board stuff
	sysclk_init(); // HS starts the clock
	board_init(); // NEEDED TO DISABLE CASHE!!!!!!!
	
	
	// Init SPI master
	spi_init(); // initializes SPI transfer
	
	// Init DMA handling SPI data reads for 3 tx and 3 rx DMA types
	// link one byte without stopping between bytes over spi
	spi_xdmac_configure(SPI_MASTER_BASE);
	
	// Setup interrupt priority
	// NVIC: Nested vector interrupt controller
	NVIC_ClearPendingIRQ(XDMAC_IRQn); //Clear a device specific interrupt from pending.
	NVIC_DisableIRQ(XDMAC_IRQn); // Disable a device specific interrupt.
	NVIC_SetPriority( XDMAC_IRQn ,1); // Set priority grouping -
	NVIC_EnableIRQ(XDMAC_IRQn); // Enable a device specific interrupt
	
	// Systick Init for handling keeping track of time
	//systick_init(); // DANIEL PROBABLY DONT NEED THIS
	
	// USART enable UART for serial comm
	//pmc_enable_periph_clk(ID_USART1); // Enable the specified peripheral clock.

	/* // DANIEL WE DONT NEED THIS
	usart_serial_options_t usart_options = {
		.baudrate = CONF_UART_BAUDRATE,
		.charlength = US_MR_CHRL_8_BIT,
		.paritytype = US_MR_PAR_NO,
		.stopbits = US_MR_NBSTOP_1_BIT
	};
	usart_serial_init(CONF_UART, &usart_options);
	*/
	// Fill interface buffer with all 1's
	interface_buffer_init(); // Just sets all values to 0xFF here
	
	// sets state of the Naneye, this is a bitshift operation
	state = STATE_IDLE;
	
	// Used for tx and rx in DMA when we don't need to save the data
	spi_dma_trash[0] = 0x00;
	spi_dma_trash[1] = 0x00;
	lastToggleTime = ms_ticks;
	// Pause for a few 100ms to make sure the naneye is ready
	while (ms_ticks < (lastToggleTime + 1000)) {}
	
	// Let's start naneye communication. To do this we will finish setting up the interface mode stuff and then turn on the DMAs
	naneye_new_reg_val_received = 1;
	
	// Initial initialization sequence
	//spi_enable_xdmac(); // HS added, but this is a roundabout way of doing this (we should be using xdmac configure transfer)
	//spi_disable_xdmac();// HS added, but this is a roundabout way of doing this (we should be using xdmac configure transfer)

	//xdmac_configure_transfer(XDMAC, XDMAC_TX_CH, &xdmac_cfg[0]);// HS gives the address for the 1st array in xdmac_cfg array
	//xdmac_configure_transfer(XDMAC, XDMAC_RX_CH, &xdmac_cfg[0]);// HS gives the address for the 2nd array in xdmac_cfg array
	
	// sets naneye register
	// the 10th bit is the 11th value when read from the right to left
	// |= sets particular value to 1
	// &= with a ~ means and not, sets value to 0
	// ~ also inverts the orientation means and not, sets value to 0
	
	naneyec_reg_val[0] = NANEYE_REG0_DEFAULT_VALUE | 0b1100; // Sets offset ramp to recommended 2.2V value
	naneyec_reg_val[0] = naneyec_reg_val[0] | 0b11; // HS turned off Sets output current to max (might only affect LVDS mode)
	

	naneyec_reg_val[1] = NANEYE_REG1_DEFAULT_VALUE;
	naneyec_reg_val[1] |= (1<<10); // Increase 2x bias current, reduces settling time for high speed apps (not sure what this does)
	naneyec_reg_val[1] &= ~(1<<9); // Sets CDS gain to recommended value of 1.3 (turns a 1--> 0, which sets CDS gain to 1.3)
	naneyec_reg_val[1] &= ~(1<<8); // Sets mode to SEIM
	naneyec_reg_val[1] = (naneyec_reg_val[1] & (0b1111111111001111)) | (0b10 << 4); // Sets vref to recommended value of 2.1V
	naneyec_reg_val[1] = (naneyec_reg_val[1] & (0b1111111111110011)) | (0b01 << 2); // Sets CVC current to recommended value
	naneyec_reg_val[1] &= ~(1<<1); // Turns off idle mode
	spi_config_initial_interface_mode(); // HS added function initializes IIM sequence
	// SPI_MASTER_BASE->SPI_TDR= 0b00; // HS Added sends signal to the SPI_TDR transfer register to send out

	interface_buffer_reg_set(naneyec_reg_val[0],naneyec_reg_val[1]);
	//// ADD PRESYNC HERE
	spi_config_interface_mode(); // This enables the MOSI peripheral pin and sets the correct clock phase
	// Set the correct DMA transfer config (forcing MOSI pin to be internally connected in readout mode)
	// 2 DMA running in parallel
// 	xdmac_configure_transfer(XDMAC, XDMAC_TX_CH, &xdmac_cfg[INTERFACE_MODE_TX_POS]);//  gives the address for the 1st array in xdmac_cfg array
// 	xdmac_configure_transfer(XDMAC, XDMAC_RX_CH, &xdmac_cfg[INTERFACE_MODE_RX_POS]);//  gives the address for the 2nd array in xdmac_cfg array
	delay_ms(1000);
	// Later will need to add a 10us delay after sending out the configuration
	spi_enable_xdmac(); // this is where the pulses begin to get sent out
	state = STATE_INTERFACE;
	
	uint8_t naneye_data[2];
	uint8_t debug_data[120];
	uint32_t start_pos = 0;
	
	uint32_t tempVal;
	uint8_t frame_idx_to_send = 0;
	// this portion of the code is inefficient because each loop checks the register bit values to see if there is data, and then runs the dmac controller.
	// runs forever
	while (1) {
		// Serial communication over USB
		uart_handle_input(); // This checks if there is uart char received and then does stuff with it
		// if there is a frame requested,
		if (frame_requested) {
			// send all pixels over UART
			frame_idx_to_send = (spi_buffer_write_index + 1) % 2; // grabs the frame not being written to currently
			// sends 12 bit pixel 2x value over 8 bits after 3x
			for (uint32_t i = 0; i < READOUT_MODE_SIZE; i += 3) {
				// loop that writes out data from frame buffer if frame is requested by user
				tempVal = spi_readout_mode_rx_buffer[frame_idx_to_send][start_pos + i] << 16 | spi_readout_mode_rx_buffer[frame_idx_to_send][start_pos + (i + 1)] << 8 | spi_readout_mode_rx_buffer[frame_idx_to_send][start_pos + (i + 2)];
				naneye_data[0] = (tempVal >> 15) & 0xFF;
				naneye_data[1] = (tempVal >> 3) & 0xFF;
				// does this squelch out a data vector of the naneye?
				usart_serial_write_packet(CONF_UART,naneye_data,2);
				
			}
			frame_requested = false;
			
		}
		// LED toggling for debugging
		if (ms_ticks > (lastToggleTime + 500)) {
			if (debug_mode) {
				// output debug info to console
				for (uint8_t i = 0; i < 120; i++)
				debug_data[i] = spi_readout_mode_rx_buffer[frame_idx_to_send][i];
				usart_serial_write_packet(CONF_UART,debug_data,120);
			}
			
			lastToggleTime = ms_ticks;;
			ioport_toggle_pin_level(LED0_GPIO);
		}
	}

	/* Insert application code here, after the board has been initialized. */
}

// ------------- Handlers -------------------------
// DANIEL Handlers for State machine switching
void XDMAC_Handler(void) {
	// checks status of DMA
	// overflow, everything is fine, no data etc--> dma handler checks, sets control for moving to delay/sync mode
	uint32_t dma_status = 0;
	uint32_t trash = 0;
	uint32_t DMA_Init = XDMAC->XDMAC_GIS;
	dma_status = XDMAC->XDMAC_CHID[XDMAC_RX_CH].XDMAC_CIS; //This checks the dma_status of the SPI DMA
	
	trash = XDMAC->XDMAC_CHID[XDMAC_TX_CH].XDMAC_CIS;
	
	if (dma_status & XDMAC_CIS_BIS) { //If SPI DMA is done
		
		// We will disable DMA, setup for next mode, and then start DMA
		spi_disable_xdmac();
		// whatever state we were in, now we move states
		// disables the mosi pin
		switch (state) {
			case (STATE_INTERFACE):
			// Just finished interface mode
			if (naneye_new_reg_val_received) { // this me we just wrote new reg values to the NE
				interface_buffer_reg_clear();
				naneye_new_reg_val_received = 0;
			}
			spi_config_readout_mode();
			
			// Set the correct DMA transfer config
			xdmac_configure_transfer(XDMAC, XDMAC_TX_CH, &xdmac_cfg[SYNC_AND_DELAY_MODE_TX_POS]);
			xdmac_configure_transfer(XDMAC, XDMAC_RX_CH, &xdmac_cfg[SYNC_AND_DELAY_MODE_RX_POS]);
			
			state = STATE_SYNC_AND_DELAY; // bit shifts to the left when it finishes the state interface
			break;
			case (STATE_SYNC_AND_DELAY):
			// Just finished Sync and Delay modes
			
			// update the readout rx buffer location for a new frame
			
			
			if (frame_requested == false) { // increment frame buffer if not currently outputting a frame
				spi_buffer_write_index++;
				if (spi_buffer_write_index >= BUFFER_FRAME_SIZE)
				spi_buffer_write_index = 0;
			}
			xdmac_cfg[READOUT_MODE_RX_POS].mbr_da = (uint32_t)&spi_readout_mode_rx_buffer[spi_buffer_write_index][0];
			// Set the correct DMA transfer config
			xdmac_configure_transfer(XDMAC, XDMAC_TX_CH, &xdmac_cfg[READOUT_MODE_TX_POS]);
			xdmac_configure_transfer(XDMAC, XDMAC_RX_CH, &xdmac_cfg[READOUT_MODE_RX_POS]);
			
			state = STATE_READOUT; // bit shifts to the left when it finishes the state sync/delay (sync/delay-> state readout)
			break;
			case (STATE_READOUT):
			// Just finished readout mode
			if (naneye_new_reg_val_received) { // this means we need to update the interface tx buffer to send new reg vals to naneye
				interface_buffer_reg_set(naneyec_reg_val[0],naneyec_reg_val[1]);
			}
			spi_config_interface_mode(); // This enables the MOSI peripheral pin and sets the correct clock phase
			
			// Set the correct DMA transfer config
			xdmac_configure_transfer(XDMAC, XDMAC_TX_CH, &xdmac_cfg[INTERFACE_MODE_TX_POS]);// & means pass pointer location
			xdmac_configure_transfer(XDMAC, XDMAC_RX_CH, &xdmac_cfg[INTERFACE_MODE_RX_POS]);
			
			state = STATE_INTERFACE; // bit shifts to the left when it finishes the state readout, (state-readout->state interface)
			break;
		}
		
		spi_enable_xdmac();
	}
	
}

void SysTick_Handler(void) {
	ms_ticks++;
}

// DANIEL MY CUSTOM CODE FOR NE

#include "MS_definitions.h"

#include "GS_definitions.h"
#include <utils.h>
#include <atmel_start.h>

#define INTERFACE_MODE_SIZE		972 // (648 * 12 / 8) in bytes

volatile uint8_t naneye_new_reg_val_received = 0;
volatile uint16_t naneyec_reg_val[2];
volatile uint8_t spi_interface_mode_tx_buffer[INTERFACE_MODE_SIZE]; // Used to tx during interface mode

void interface_buffer_init(void) {
	// Fill interface tx buffer with all 1's
	for (uint32_t i = 0; i < INTERFACE_MODE_SIZE; i++)
	spi_interface_mode_tx_buffer[i] = 0xFF;
}

void interface_buffer_reg_set(uint32_t reg0, uint32_t reg1) {
	// Cannot update regs in the first SPI clock of interface mode so we will start in the second byte location
	
	// For reg0
	spi_interface_mode_tx_buffer[1] = 0b10010000 | ((reg0 >> 15) & 0x01);
	spi_interface_mode_tx_buffer[2] = ((reg0 >> 7) & 0xFF);
	spi_interface_mode_tx_buffer[3] = ((reg0 << 1) & 0xFF);
	
	// For reg1
	// Lets give a 3 byte gap between writing the 2 registers
	spi_interface_mode_tx_buffer[7] = 0b10010010 | ((reg1 >> 15) & 0x01);
	spi_interface_mode_tx_buffer[8] = ((reg1 >> 7) & 0xFF);
	spi_interface_mode_tx_buffer[9] = ((reg1 << 1) & 0xFF);
}

#endif