/**
@file
@brief Functions to interface Python 480 (image sensor)
@author Daniel, Federico, Takuya
@sa https://www.onsemi.jp/download/data-sheet/pdf/noip1sn0480a-d.pdf
*/


#ifndef PYTHON480_H_
#define PYTHON480_H_

#include <driver_init.h>
#include <utils.h>

#define DISABLE_PLL

/**
@def ROI_XREG_MAX
@brief maximum value of ROI X-range register
*/
#define ROI_XREG_MAX 201 ///

/**
@def ROI_XREG_MIN
@brief minimum value of ROI X-range register
*/
#define ROI_XREG_MIN 0 ///

/**
@def ROI_YREG_MAX
@brief maximum value of ROI Y-range register
*/
#define ROI_YREG_MAX 151

/**
@def ROI_YREG_MIN
@brief minimum value of ROI Y-range register
*/
#define ROI_YREG_MIN 0

void python480Init(void);

/**
@brief Sets gain of image sensor
@param value Value of gain. 1: gain = 1; 2: gain = 2; 4: gain = 3.5; default: gain = 1
@note Adding validation might be nice though I'm not sure what's the best way to feedback to users
*/
void python480SetGain(uint32_t value);

/**
@brief Sets ROI of image sensor
@param image_width image width
@param xshift xshift from default
@param yshift yshift from default
@note Adding validation might be nice though I'm not sure what's the best way to feedback to users
*/
void setROI(uint16_t image_width, uint16_t xshift, uint16_t yshift);

/**
@brief Sets FPS of image sensor
@param value Value of FPS. For value = 1, 5, 10, 15, 20, FPS = value; for value = 0: FPS = 0.5; default: undefined
@note Need to check what's default value of the image sensor.
*/
void python480SetFPS(uint32_t value);

/**
@brief Write function of bit-banging SPI (configured for python 480)
@param[in] address Python 480 register address
@param[in] value register write value
@note Probably should change function name to explicitly show it's a transfer of address and value and not general SPI. Not urgent because the namespace is narrow enough.
*/
void spi_BB_Write(uint16_t address, uint16_t value);

/**
@brief Read function of bit-banging SPI (configured for python 480)
@param[in] address Python 480 register address
@return register read value
@note Probably should change function name to explicitly show it requires address and value and not general SPI. Not urgent because the namespace is narrow enough.
*/
uint16_t spi_BB_Read(uint16_t address);
void EnableClockMngmnt1(void);
void EnableClockMngmnt2(void);

/**
@brief Upload required registers for Python 480
@param[in] image_width
@note To do: Make ROI definition more flexible
*/
void RequiredUploads(uint16_t image_width);
void SoftPowerUp(void);
void NoTransfer(void);
void Transfer(void);
void Eblack(void);
void Egray(void);
void python480sequenceInit(void);
void DisableE(void);
void EnableSeq (void);
void DisableSeq (void);

#endif /* PYTHON480_H_ */