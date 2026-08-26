/**
@file
@brief Functions to interface NanEyeC (image sensor)
@author Daniel, Hemal
@note this is where GS_Definitions.h will go
*/
// DANIEL These are the GS definitions from the previous NE Code
#ifndef DEFINITIONS_H_
#define DEFINITIONS_H_

// States
#define STATE_IDLE				0 // uint32 all 0
#define STATE_INTERFACE			1<<0 // uint31 ....00001
#define STATE_SYNC_AND_DELAY	1<<1 // uint31 ....00010
#define STATE_READOUT			1<<2 // uint31 ....00100

//NaneyeC values
#define NANEYE_REG0_DEFAULT_VALUE	0b1000000010010101
#define NANEYE_REG1_DEFAULT_VALUE	0b0000001101011010

#define NANEYE_LAST_INTERFACE_BYTE	0b000000010101

#define NANEYE_INITIAL_INTERFACE_BYTE	0b00

// DMA Transfer Definitions
// 
#define INTERFACE_MODE_SIZE		972 // (648 * 12 / 8) in bytes
#define SYNC_MODE_SIZE			984 // 656 * 12 / 8) in bytes
#define DELAY_MODE_MIN_SIZE		984 // 2 * 328 * 12 / 8) in bytes
#define READOUT_MODE_SIZE		157452 // (320 * (8 + 320) +8) * 12/8 in bytes
#define INITIAL_INTERFACE_MODE_SIZE	1 // 1 cycle in total, 342 *12/8 = 513 for full II sequence

#define BUFFER_FRAME_SIZE		2 // Number of frames that can be held in spi receive buffer

// DMA Config positions
#define INTERFACE_MODE_TX_POS		0
#define INTERFACE_MODE_RX_POS		1
#define SYNC_AND_DELAY_MODE_TX_POS	2
#define SYNC_AND_DELAY_MODE_RX_POS	3
#define READOUT_MODE_TX_POS			4
#define READOUT_MODE_RX_POS			5

// trying the initial interface mode HS added
#define INITIAL_INTERFACE_MODE_TX_POS		6
#define INITIAL_INTERFACE_MODE_RX_POS		7


// ------- SPI Definitions
#define SPI_MASTER_BASE     SPI0
#define SPI_IRQn			SPI0_IRQn
#define SPI_ID				ID_SPI0

#define SPI_CHIP_SEL		0
#define SPI_CHIP_PCS		spi_get_pcs(SPI_CHIP_SEL)
#define SPI_DLYBS			0x0 //Delay between NPCS activation to first SPCK (0 is half clock cycle)
#define SPI_DLYBCT			0x00 //Delay between consecutive transfers. A delay before CS is pulled up at end of transfer

#define SPI_DLYBCS			0x00 // Delay NPCS is held high before next transmit
#define SPI_CLK_RATE		16000000 // 16000000 // 24000000 // MHz

// pin defintions for first NE camera (MONO) sensor
#define SPI_MISO_PIN		PB14
#define SPI_MISO_MODE		IOPORT_MODE_MUX_B // DANIEL not sure if we need this? Located in driver_init.c
#define SPI_MOSI_PIN		PB15
#define SPI_MOSI_MODE		IOPORT_MODE_MUX_B// DANIEL not sure if we need this?
#define SPI_SPCK_PIN		PB13
#define SPI_SPCK_MODE		IOPORT_MODE_MUX_B// DANIEL not sure if we need this?
#define SPI_NPCS_PIN		PIO_PB2_IDX// DANIEL not sure if we need this?
#define SPI_NPCS_MODE		IOPORT_MODE_MUX_D// DANIEL not sure if we need this?

// SPI DMA
#define SPI0_XDMAC_TX_CH_NUM	1 //XDMAC Channel HW Interface for SPI0
#define SPI0_XDMAC_RX_CH_NUM	2 //XDMAC Channel HW Interface for SPI0
#define XDMAC_TX_CH				0 //XDMAC Channel
#define XDMAC_RX_CH				1 //XDMAC Channel


// pin defintions for first NE camera (RGB) sensor
// #define SPI_MISO_PIN		PIO_PD20_IDX
// #define SPI_MISO_MODE		IOPORT_MODE_MUX_B
// #define SPI_MOSI_PIN		PIO_PD25_IDX
// #define SPI_MOSI_MODE		IOPORT_MODE_MUX_B
// #define SPI_SPCK_PIN		PIO_PD28_IDX
// #define SPI_SPCK_MODE		IOPORT_MODE_MUX_B
// #define SPI_NPCS_PIN		PIO_PB2_IDX
// #define SPI_NPCS_MODE		IOPORT_MODE_MUX_D


// Variables
volatile uint16_t naneyec_reg_val[2];
volatile uint8_t naneye_new_reg_val_received = 0;
volatile uint8_t spi_interface_mode_tx_buffer[INTERFACE_MODE_SIZE]; // Used to tx during interface mode


#endif /* DEFINITIONS_H_ */