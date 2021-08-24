/**
 *  @file Timer_Port.c
 *
 *  @date 2021-01-01
 *
 *  @author aron566
 *
 *  @copyright 爱谛科技研究院.
 *
 *  @brief 定时器接口
 *
 *  @details 1、对外提供运行时间计数
 *           2、1ms间隔调用HAL_TIM_PeriodElapsedCallback
 *
 *  @version V1.0
 */
#ifdef __cplusplus ///<use C compiler
extern "C" {
#endif
/** Includes -----------------------------------------------------------------*/
/* Private includes ----------------------------------------------------------*/
#include "Timer_Port.h"
  
/** Private typedef ----------------------------------------------------------*/

/** Private macros -----------------------------------------------------------*/
/** Private constants --------------------------------------------------------*/
/** Public variables ---------------------------------------------------------*/
/** Private variables --------------------------------------------------------*/
static uint32_t Timer_Port_TimeMS  = 0;
static uint32_t Timer_Port_TimeSec = 0;

/** Private function prototypes ----------------------------------------------*/
static inline void Timer_Port_IRQHandler(void);
/** Private user code --------------------------------------------------------*/

/** Private application code -------------------------------------------------*/
/*******************************************************************************
*
*       Static code
*
********************************************************************************
*/
/**
  ******************************************************************
  * @brief   定时器中断回调
  * @param   [in]None
  * @return  None.
  * @author  aron566
  * @version V1.0
  * @date    2021-01-13
  ******************************************************************
  */
static inline void Timer_Port_IRQHandler(void)
{
  Timer_Port_TimeMS++;
  if(Timer_Port_TimeMS == 1000)
  {
    Timer_Port_TimeMS = 0;
    Timer_Port_TimeSec++;
  }
}

/** Public application code --------------------------------------------------*/
/*******************************************************************************
*
*       Public code
*
********************************************************************************
*/
/**
  ******************************************************************
  * @brief   定时器周期时间到回调
  * @param   [in]TimerX 定时器句柄
  * @return  None.
  * @author  aron566
  * @version V1.0
  * @date    2021-02-25
  ******************************************************************
  */
void HAL_TIM_PeriodElapsedCallback(void *TimerX)
{
  (void)(TimerX);
  Timer_Port_IRQHandler();
}

/**
  ******************************************************************
  * @brief   获取当前运行累计时间
  * @param   [in]time_base 单位
  * @return  None.
  * @author  aron566
  * @version V1.0
  * @date    2021-01-13
  ******************************************************************
  */
uint32_t Timer_Port_Get_Current_Time(TIMER_TIME_UNIT_Typedef_t time_unit)
{
  return (time_unit == TIMER_MS)?Timer_Port_TimeMS:Timer_Port_TimeSec;
}

/**
  ******************************************************************
  * @brief   延时指定毫秒
  * @param   [in]ms 单位
  * @return  None.
  * @author  aron566
  * @version V1.0
  * @date    2021-07-13
  ******************************************************************
  */
void Timer_Port_Delay(uint32_t ms)
{
  uint32_t time_ms = Timer_Port_TimeMS;
  while((Timer_Port_TimeMS - time_ms) < ms);
}

/**
  ******************************************************************
  * @brief   定时器初始化
  * @param   [in]None
  * @return  None.
  * @author  aron566
  * @version V1.0
  * @date    2021-04-19
  ******************************************************************
  */
void Timer_Port_Init(void)
{
  /*初始化硬件定时器驱动*/
  
}

/******************************** End of file *********************************/
