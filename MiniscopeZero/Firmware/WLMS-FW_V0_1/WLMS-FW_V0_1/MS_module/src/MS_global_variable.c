/**
@file
@brief Global variables
@author Takuya
*/
#include "MS_config.h"
#include "MS_definitions.h"

// ------------ GLOBAL VARIABLES --------
COMPILER_ALIGNED(16)
volatile uint32_t dataBuffer[NUM_BUFFERS][BUFFER_BLOCK_LENGTH * PCC_BLOCK_SIZE_IN_WORDS] = {((unsigned char)0)};; //Allocate memory for DMA image buffers

volatile uint8_t headerBlock[SD_BLOCK_SIZE] = {0}; // Will hold the 512 bytes from the header block of sd card
volatile uint8_t configBlock[SD_BLOCK_SIZE] = {0}; // Will hold the device config information to be written to the starting block

volatile uint32_t currentBlock = STARTING_BLOCK;
volatile uint32_t initBlocksRemaining = 0;

volatile uint32_t deviceState = DEVICE_STATE_IDLE;

//COMPILER_ALIGNED(16) // Taken from hpl_dmac.c but I think this could be '8' since descriptors need to be 64bit aligned from data sheet
//volatile DmacDescriptor linkedList[NUM_BUFFERS];

#ifdef SPI_SERCOM0_ENABLE
Sercom *sercom_sdo = SERCOM0;
#elif defined(SPI_SERCOM5_ENABLE)
Sercom *sercom_sdo = SERCOM5;
#elif defined(SPI_SERCOM7_ENABLE)
Sercom *sercom_sdo = SERCOM7;
#elif defined(USART_SERCOM5_ENABLE)
Sercom *sercom_sdo = SERCOM5;
#endif

#ifdef IR_UART_ENABLE
Sercom *sercom_ir = SERCOM1;
#endif

// Probably should turn this into a struct to be more easily understandable
volatile uint64_t SDTransferDescriptor; // I think we will only use 1 of these for now. Each descriptor is 64bits long 

volatile uint32_t frameNum = 0;
volatile uint32_t bufferCount = 0;
volatile uint32_t frameBufferCount = 0;

volatile uint32_t startTimeMS;
volatile uint32_t endTimeMS;
volatile uint32_t timeMS = 0;

volatile uint16_t battVolt;
volatile uint8_t wptVolt;

// used for tracking recording and inc. DMA buffers
volatile uint32_t writeFrameNum;
volatile uint32_t writeBufferCount;
volatile uint32_t droppedBufferCount;
volatile uint32_t droppedFrameCount;
volatile uint32_t framesToDrop;
volatile uint32_t *bufferToWrite;
volatile uint32_t numBlocks = BUFFER_BLOCK_LENGTH;
volatile uint32_t numBuffersPerFrame = 0; 
volatile uint32_t sdo_dma_cb_calls = 0;
volatile uint32_t pcc_dma_cb_calls = 0;

// Debugging and checking stuff
volatile uint16_t chip_id; // Reads the chip id from Python480 to make sure we can talk to it
volatile uint32_t ewlvalue;
volatile uint32_t batteryvalue;
volatile uint32_t ledvalue; 
volatile uint32_t frameratevalue;
volatile uint32_t delayvalue;
volatile uint32_t reclengthvalue;

volatile uint32_t ewlScanEnable;
volatile uint32_t ewlStart;
volatile uint32_t ewlStop;
volatile uint32_t ewlStep;
volatile uint32_t ewlStepTime;
volatile uint32_t ewlCount = 0;

volatile uint16_t regValue[2]; 
volatile uint32_t tempPCC[4];
volatile uint32_t tempHeader[100][4];
volatile uint32_t tempCount = 0;
volatile uint32_t tempTimestamp[100];
volatile uint8_t timerIndex = 0;

volatile uint16_t roi_x_shift = ROI_XSTART_REG;
volatile uint16_t roi_y_shift = ROI_YSTART_REG;
volatile uint8_t subsampleEnable;
volatile uint16_t image_width;
volatile uint16_t image_height;
volatile uint32_t num_pixels;
// --------------------------------------