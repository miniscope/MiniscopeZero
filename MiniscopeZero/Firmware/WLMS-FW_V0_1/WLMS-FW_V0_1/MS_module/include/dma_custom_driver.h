/**
@file
@brief Custom driver functions that are added to the ASF drivers.
@author Takuya
*/

#ifndef MS_CUSTOM_DRIVER_H_
#define MS_CUSTOM_DRIVER_H_

#include <utils.h>

/**
@brief Driver function for setting the BTCTRL register.
@param channel DMA channel number
@param src value of the BTCTRL register.
@return 0 (ERR_NONE)
@note Probably should move this to hpl_dmac.h at some point
*/
int32_t _dma_set_BTCTRL(const uint8_t channel, uint32_t src);


/**
@brief Driver function for setting the BTCTRL register.
@param channel DMA channel number
@return DESCADDR.reg value of the specified DMA channel
@note Probably should move this to hpl_dmac.h at some point
*/
uint32_t _dma_get_DESCADDR(const uint8_t channel);

#endif /* MS_CUSTOM_DRIVER_H_ */