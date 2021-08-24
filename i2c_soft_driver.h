/**
 *  @file i2c_soft_driver.h
 *
 *  @date 2021/8/15
 *
 *  @author Copyright (c) 2021 aron566 <aron566@163.com>.
 *
 *  @brief None
 *  
 *  @version v1.0
 */
#ifndef I2C_SOFT_DRIVER_H
#define I2C_SOFT_DRIVER_H
#ifdef __cplusplus ///<use C compiler
extern "C" {
#endif
/** Includes -----------------------------------------------------------------*/
#include <stdint.h> /*need definition of uint8_t*/
#include <stddef.h> /*need definition of NULL*/
#include <stdbool.h>/*need definition of BOOL*/
#include <stdio.h>  /*if need printf*/
#include <stdlib.h>
#include <string.h>
#include <limits.h> /**< if need INT_MAX*/
/** Private includes ---------------------------------------------------------*/
#include "main.h"  
/** Private defines ----------------------------------------------------------*/

/** Exported typedefines -----------------------------------------------------*/

/** Exported constants -------------------------------------------------------*/
/** Exported macros-----------------------------------------------------------*/
/** Exported variables -------------------------------------------------------*/
/** Exported functions prototypes --------------------------------------------*/

/*软件I2C初始化*/
void Soft_I2C_Init(void);
/*软件I2C接收数据*/
HAL_StatusTypeDef HAL_I2C_Master_Receivex(void *x, uint8_t Saddr, uint8_t *Buf, uint16_t Size, uint32_t Block_Time);
/*软件I2C发送数据*/
HAL_StatusTypeDef HAL_I2C_Master_Transmitx(void *x, uint8_t Saddr, const uint8_t *Data, uint16_t Size, uint32_t Block_Time);
/*获取软件I2C空闲状态*/
bool Soft_I2C_Is_Free(void);

#ifdef __cplusplus ///<end extern c
}
#endif
#endif
/******************************** End of file *********************************/
