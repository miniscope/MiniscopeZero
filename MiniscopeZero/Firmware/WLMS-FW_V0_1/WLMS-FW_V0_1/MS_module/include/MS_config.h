/**
@file
@brief Configuration files of the project.
*_MODE flags refer to the hardware configuration used.
*_ENABLE flags refer to the peripherals that will be included.
The modes should be a set of peripherals used in the specified mode.
@author Takuya
*/


#ifndef MS_CONFIG_H_
#define MS_CONFIG_H_


// ------ DATA MODE ------------------------
//#define DEV_MODE

#ifdef DEV_MODE
#define TEST_PATTERN_ENABLE // For dev
#define TEST_PRBS_BUFFER_ENABLE
#endif


// ------ HARDWARE MODE ------------------------
//#define V4WF_MODE
#define WLMS_SPI_MODE
//#define BERT_MODE
//#define GS_MODE
//#define WLMS_USART_MODE
//#define WLMS_SD_MODE
//#define DMA_TO_SPI_TESTMODE
//#define DMA_TO_SPI_METRO_TESTMODE

// Peripheral enable based on mode
// PYTHON480_ENABLE: enables python480 to dataBuffer storage via PCC/DMA
// DMA_TO_SD_ENABLE: enables DMA to SD card ADMA
// DMA_TO_SPI_ENABLE: enables DMA to SPI via DMA

// ------ HARDWARE PERIPHERAL ENABLE ------------------------
#ifdef WLMS_SPI_MODE
//#define TEST_PATTERN_ENABLE // For dev
#define PYTHON480_ENABLE
//#define PYTHON480_608PX_SUBSAMPLE
//#define PYTHON480_400PX_SUBSAMPLE
//#define PYTHON480_320PX_SUBSAMPLE
//#define PYTHON480_304PX_SUBSAMPLE
//#define PYTHON480_152PX_NOSUBSAMPLE
#define PYTHON480_200PX_NOSUBSAMPLE
//#define PYTHON480_160PX_NOSUBSAMPLE
//#define PYTHON480_304PX_NOSUBSAMPLE
#define EWL_ENABLE
#define DMA_TO_SPI_ENABLE
#define EXLED_PWM_ENABLE
#define BATTERY_ENABLE
#define BATTERY_CB_DISABLE
#define WPT_ADC_ENABLE
#define PUSH_BUT_ENABLE
#define STATUS_LED_ENABLE
#define IR_UART_ENABLE
//#define IR_TRIGGER_ENABLE
//#define SPI_SERCOM0_ENABLE
#define SPI_SERCOM5_ENABLE
#define SDO_32BIT_ENABLE
#define PREAMBLE_ENABLE
#define PRESET_HEADER_ENABLE
#define AUTOSTART_ENABLE
//#define FRAMERATE_1FPS
//#define FRAMERATE_5FPS
//#define FRAMERATE_10FPS
#define FRAMERATE_20FPS
#define RESTART_KEY				200
#define BUFFER_BLOCK_LENGTH		10			// can be edited by user to optimize speed
#define NUM_BUFFERS				32			// Assumes 1024 KB program memory
#define ROI_XSTART_REG			76			// x shift
#define ROI_YSTART_REG			51			// y shift
#define EXCITATION_LED_VALUE	1			// Brightness value of excitation LED: from 0 to 100.
#define DUMMY_WORD_LENGTH		10
#define DUMMY_WORD				0xff00cccc
#define PREAMBLE_WORD			0x12345678
#define DEVICE_ID				0x00000005
//#define TEST_PRBS_BUFFER_ENABLE
#endif

#ifdef BERT_MODE
#define PYTHON480_ENABLE
#define PYTHON480_304PX_NOSUBSAMPLE
#define BERT_8MBPS
#define TEST_PRBS_BUFFER_ENABLE
#define FRAMERATE_1FPS
#define EWL_ENABLE
#define DMA_TO_SPI_ENABLE
#define EXLED_PWM_ENABLE
#define BATTERY_ENABLE
#define BATTERY_CB_DISABLE
#define WPT_ADC_ENABLE
#define PUSH_BUT_ENABLE
#define STATUS_LED_ENABLE
#define IR_UART_ENABLE
#define SPI_SERCOM5_ENABLE
#define SDO_32BIT_ENABLE
#define PREAMBLE_ENABLE
#define PRESET_HEADER_ENABLE
#define AUTOSTART_ENABLE
#define RESTART_KEY				200
#define BUFFER_BLOCK_LENGTH		10			// can be edited by user to optimize speed
#define NUM_BUFFERS				32			// Assumes 1024 KB program memory
#define ROI_XSTART_REG			76			// x shift
#define ROI_YSTART_REG			51			// y shift
#define EXCITATION_LED_VALUE	0			// Brightness value of excitation LED: from 0 to 100.
#define DUMMY_WORD_LENGTH		10
#define DUMMY_WORD				0xff00cccc
#define PREAMBLE_WORD			0x12345678
#define DEVICE_ID				0x00000005
#endif

#ifdef WLMS_USART_MODE
#define PYTHON480_ENABLE
//#define PYTHON480_304PX
#define PYTHON480_152PX
#define DMA_TO_USART_ENABLE
#define EXLED_PWM_ENABLE
#define BATTERY_ENABLE
//#define WPT_ADC_ENABLE
#define EWL_ENABLE
#define PUSH_BUT_ENABLE
#define STATUS_LED_ENABLE
#define IR_UART_ENABLE
#define USART_SERCOM5_ENABLE
#define SDO_8BIT_ENABLE
#define PREAMBLE_ENABLE
#define PRESET_HEADER_ENABLE
#define AUTOSTART_ENABLE
#define FRAMERATE_1FPS
#define DEBUGLED_ENABLE
#define BUFFER_BLOCK_LENGTH		40 // can be edited by user to optimize speed
#define NUM_BUFFERS				8 // Assumes 1024 KB program memory
#endif



#ifdef GS_MODE
#define NANEYE_ENABLE
// #define DMA_TO_SD_ENABLE
//#define EXLED_PWM_ENABLE // RE-ENABLE WITH PROPER PIN NAME
// #define BATTERY_ENABLE
// #define PUSH_BUT_ENABLE
//#define STATUS_LED_ENABLE // RE-ENABLE WITH PROPER PIN NAME
#define SPI_SERCOM5_ENABLE // LED
#define SERCOM4_ENABLE // Naneye
#define DMA_TO_SPI_ENABLE
 
#define STOP_ENABLE
//#define FRAMERATE_20FPS
#define PRESET_HEADER_ENABLE
#define DEBUGLED_ENABLE
#define SPI_LUT_ENABLE
#define SDO_32BIT_ENABLE
#define AUTOSTART_ENABLE
//#define RECORDTIME_DISABLE
#define BUFFER_BLOCK_LENGTH		40 // can be edited by user to optimize speed
#define NUM_BUFFERS				4 // Limited due to lack of memory on MS-US dev board's MCU
#endif



#ifdef WLMS_SD_MODE
#define PYTHON480_ENABLE
#define PYTHON480_304PX
//#define PYTHON480_152PX
#define DMA_TO_SD_ENABLE
#define EXLED_PWM_ENABLE
#define BATTERY_ENABLE
//#define WPT_ADC_ENABLE
#define EWL_ENABLE
#define PUSH_BUT_ENABLE
#define STATUS_LED_ENABLE
//#define IR_UART_ENABLE
//#define IR_TRIGGER_ENABLE
#define ADMA_ENABLE
#define STOP_ENABLE
//#define FRAMERATE_20FPS
//#define PRESET_HEADER_ENABLE
//#define DEBUGLED_ENABLE
#define AUTOSTART_ENABLE
//#define RECORDTIME_DISABLE
#define BUFFER_BLOCK_LENGTH		40 // can be edited by user to optimize speed
#define NUM_BUFFERS				8 // Assumes 1024 KB program memory
#endif

#ifdef DMA_TO_SPI_TESTMODE
#define DMA_TO_SPI_ENABLE
#define EXLED_PWM_ENABLE
#define BATTERY_ENABLE
#define WPT_ADC_ENABLE
#define EWL_ENABLE
#define PUSH_BUT_ENABLE
#define STATUS_LED_ENABLE
#define IR_UART_ENABLE
#define IR_TRIGGER_ENABLE
#define SPI_SERCOM0_ENABLE
#define ADMA_ENABLE
#define SPI_LUT_ENABLE
#define SDO_32BIT_ENABLE
#define PREAMBLE_ENABLE
#define PRESET_HEADER_ENABLE
#define BUFFER_BLOCK_LENGTH		40 // can be edited by user to optimize speed
#define NUM_BUFFERS				8 // Assumes 1024 KB program memory
#endif

#ifdef DMA_TO_SPI_METRO_TESTMODE
#define DMA_TO_SPI_ENABLE
#define HEADER_DISABLE
#define	TEST_PRBS_BUFFER_ENABLE
#define SPI_SERCOM0_ENABLE
#define BUFFER_BLOCK_LENGTH		40 // can be edited by user to optimize speed
#define NUM_BUFFERS				8 // Assumes 1024 KB program memory
#endif




#endif /* MS_CONFIG_H_ */