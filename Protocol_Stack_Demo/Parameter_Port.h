/**
 *  @file Parameter_Port.h
 *
 *  @date 2021-02-26
 *
 *  @author aron566
 *
 *  @brief 参数存储操作
 *  
 *  @version V1.0
 */
#ifndef PARAMETER_PORT_H
#define PARAMETER_PORT_H
#ifdef __cplusplus ///<use C compiler
extern "C" {
#endif
/** Includes -----------------------------------------------------------------*/
#include <stdint.h> /**< need definition of uint8_t */
#include <stddef.h> /**< need definition of NULL    */
#include <stdbool.h>/**< need definition of BOOL    */
#include <stdio.h>  /**< if need printf             */
#include <stdlib.h>
#include <string.h>
#include <limits.h> /**< need variable max value    */
/** Private includes ---------------------------------------------------------*/
/** Private defines ----------------------------------------------------------*/

/** Exported typedefines -----------------------------------------------------*/
/*通道设置*/
typedef enum
{
  LEFT_DEVICE_SEL = 0,          /**< 左通道参数*/
  RIGHT_DEVICE_SEL,             /**< 右通道参数*/
  BOTH_DEVICE_SEL,              /**< 双通道*/
}DEVICE_CHANNEL_SEL_Typedef_t;

/*寄存器参数存储区域索引号*/
typedef struct
{
  uint8_t index;          /**< 寄存器数值在100个字节内的偏移量*/
  uint8_t reg_val;        /**< 寄存器数值*/   
  uint16_t reg;           /**< 寄存器地址*/
}REG_PAR_Typedef_t;
/** Exported constants -------------------------------------------------------*/

/** Exported macros-----------------------------------------------------------*/
/** Exported variables -------------------------------------------------------*/

/** Exported functions prototypes --------------------------------------------*/

/*参数接口初始化*/
void Parameter_Port_Init(void);
/*更新参数*/
void Parameter_Port_Update(const uint8_t *Data);
/*设置参数*/
bool Parameter_Port_Set_Par(uint16_t Reg_Addr, uint8_t *Val);
/*获取参数*/
bool Parameter_Port_Get_Par(uint16_t Reg_Addr, uint8_t *Buf);
/*设置参数通道*/
void Parameter_Port_Update_Channel(DEVICE_CHANNEL_SEL_Typedef_t Sel);

#ifdef __cplusplus ///<end extern c
}
#endif
#endif
/******************************** End of file *********************************/
