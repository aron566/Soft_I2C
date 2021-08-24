/**
 *  @file Timer_Port.h
 *
 *  @date 2021-01-13
 *
 *  @author aron566
 *
 *  @brief 
 *  
 *  @version V1.0
 */
#ifndef TIMER_PORT_H
#define TIMER_PORT_H
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
/*定时器时间单位*/
typedef enum
{
  TIMER_SEC = 0,
  TIMER_MS
}TIMER_TIME_UNIT_Typedef_t;
/** Exported constants -------------------------------------------------------*/

/** Exported macros-----------------------------------------------------------*/
/** Exported variables -------------------------------------------------------*/
/** Exported functions prototypes --------------------------------------------*/

/*定时器接口初始化*/
void Timer_Port_Init(void);
/*获取运行时间*/
uint32_t Timer_Port_Get_Current_Time(TIMER_TIME_UNIT_Typedef_t time_unit);
/*延时指定ms*/
void Timer_Port_Delay(uint32_t ms);
/*定时器周期回调接口*/
void HAL_TIM_PeriodElapsedCallback(void *TimerX);

#ifdef __cplusplus ///<end extern c
}
#endif
#endif
/******************************** End of file *********************************/
