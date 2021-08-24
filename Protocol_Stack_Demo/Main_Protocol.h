/**
 *  @file Main_Protocol.h
 *
 *  @date 2021-07-19
 *
 *  @author aron566
 *
 *  @brief 通讯协议报文解析
 *  
 *  @version V1.0
 */
#ifndef MAIN_PROTOCOL_H
#define MAIN_PROTOCOL_H
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
#include "UART_Port.h"
/** Private defines ----------------------------------------------------------*/
/** Exported typedefines -----------------------------------------------------*/
/*返回类型*/
typedef enum RETURN_TYPE
{
  RETURN_OK = 0,
  RETURN_UPGRADE_FRAME_REDIRECT,/**< 升级帧重定向*/
  RETURN_TIMEOUT,
  RETURN_ERROR,
}RETURN_TYPE_Typedef_t;

/*报文检测结果*/
typedef struct
{
	uint16_t DataLen;
	uint16_t DataOffset;
	uint8_t* frame_data;
}FRAME_CHECK_RESULT_Typedef_t;

/*帧检测状态*/
typedef enum
{
  LONG_FRAME_CHECK_OK = 0,        /**< 长帧检测正常*/
  LONG_FRAME_CRC_ERROR,           /**< 长帧CRC错误*/
  SHORT_FRAME_CHECK_OK,       
  SHORT_FRAME_CRC_ERROR,    
  FRAME_DATA_NOT_FULL,            /**< 帧不全*/
  UNKNOW_FRAME_ERROR,             /**< 未知帧*/
}FRAME_CHECK_STATUS_Typedef_t;

/*功能码类型*/
typedef enum
{
  SET_PAR_CMD         = 0x03,
  GET_PAR_CMD         = 0x04,
  FRIMWARE_UPGRADE_CMD= 0x10,
  FRIMWARE_UPLOAD_CMD = 0x11,
  FILE_UPGRADE_CMD    = 0x20,
  FILE_UPLOAD_CMD     = 0x21,
  UNKNOW_CMD,
}COMMAND_TYPE_Typedef_t;

/*回复ACK*/
typedef enum
{
  REPLY_ACK_SET_OK     = 1,     /**< 设置正常*/
  REPLY_ACK_GET_DATA_OK,        /**< 获取数据正常*/
  REPLY_ACK_END_OK,             /**< 结束帧OK*/
  REPLY_ACK_ERROR,              /**< 错误*/
}REPLY_ACK_Typedef_t;

/*回复报文*/
typedef struct
{
  uint16_t                  frame_head;
  COMMAND_TYPE_Typedef_t    cmd;
  uint16_t                  data_len;
  uint8_t                   data_buf[512];
  REPLY_ACK_Typedef_t       ack;
  uint16_t                  crc;
}REPLY_FRAME_Typedef_t;

/** Exported constants -------------------------------------------------------*/

/** Exported macros-----------------------------------------------------------*/
/** Exported variables -------------------------------------------------------*/

/** Exported functions prototypes --------------------------------------------*/

/*协议栈初始化*/
void Protocol_Stack_Init(void);
/*启动协议栈*/
void Protocol_Stack_Start(void);
/*获取寄存器数值*/
void Main_Protocol_Get_Reg_Value(uint16_t Reg_Addr, uint8_t *Buf);
/*设置寄存器数值*/
void Main_Protocol_Set_Reg_Value(uint16_t Reg_Addr, const uint8_t *Data);
/*获取寄存器数值长度*/
uint16_t Main_Protocol_Get_Reg_Value_Size(uint16_t Reg_Addr);

#ifdef __cplusplus ///<end extern c
}
#endif
#endif
/******************************** End of file *********************************/
