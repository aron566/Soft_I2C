/**
 *  @file Main_Protocol.c
 *
 *  @date 2021-08-04
 *
 *  @author aron566
 *
 *  @copyright 爱谛科技研究院.
 *
 *  @brief 通讯协议报文解析
 *
 *  @details 1、
 *
 *  @version V2.0
 */
#ifdef __cplusplus ///<use C compiler
extern "C" {
#endif
/** Includes -----------------------------------------------------------------*/
/* Private includes ----------------------------------------------------------*/
#include "Main_Protocol.h"
#include "utilities_crc.h"
#include "Timer_Port.h"
#include "Parameter_Port.h"
/** Private macros -----------------------------------------------------------*/
/*分包发送配置*/
#define ENABLE_DEBUG_PROTOCL          0       /**< 启动调试打印报文*/
#define ENABLE_SEND_DELAY             0       /**< 为1开启分包发送*/
#define ENABLE_SEND_DELAY_MS          100U    /**< 分包发送间隔ms >30ms*/
#define ENABLE_SEND_DELAY_LIMIT_SIZE  100U    /**< >100Bytes时开启发送*/
#define SEND_ONE_PACKET_SIZE_MAX      100U    /**< 每包发送大小*/

/*协议解析*/
#define NOT_FULL_TIMEOUT_SEC_MAX      3U      /**< 帧不满超时时间*/

/*软件版本*/
#define SOFTWARE_VER_MAJOR            0U
#define SOFTWARE_VER_MINOR            1U
#define SOFTWARE_VER_REVISION         1U
#define SOFTWARE_V                    ((SOFTWARE_VER_MAJOR << 8) | (SOFTWARE_VER_MINOR << 4) | SOFTWARE_VER_REVISION)

#define GET_FRAME_SIZE_MAX            512U
#define REPLY_FRAME_SIZE_MAX          512U
#define FRAME_SIZE_MIN                7       /**< 最小帧大小*/
#define IS_LESS_MIN_FRAME_SIZE(size)  ((size < FRAME_SIZE_MIN)?1:0)
#define SET_FRAME_PACKAGE_LEN(data_len)   (data_len + 9)
#define READ_FRAME_PACKAGE_LEN       FRAME_SIZE_MIN

/*寄存器地址*/
#define REG_SOFT_VER                  0xFF00  /**< 软件版本*/
#define REG_READ_ALL_PAR              0x0055  /**< 音量参数*/
/** Private typedef ----------------------------------------------------------*/
/*协议栈句柄*/
typedef struct
{
  Uart_Dev_Handle_t *Uart_Opt_Handle;         /**< 串口操作句柄*/
  uint32_t Update_Time;                       /**< 更新时间*/
  uint32_t NotFull_LastTime;                  /**< 上次帧不全允许超时时间*/
}PROTOCOL_STACK_HANDLE_Typedef_t;

/*分包分送句柄*/
typedef struct
{
  PROTOCOL_STACK_HANDLE_Typedef_t *Stack_Opt_Handle_Ptr; 
  uint32_t Last_Send_Time_ms;
  uint32_t Wait_Send_Size;
  uint32_t Current_Send_Index;
  uint32_t Data_Total_Size;
  uint8_t *Buf_Ptr;
}SEND_TASK_LIST_Typedef_t;

/*寄存器处理映射*/
typedef RETURN_TYPE_Typedef_t (*REG_CALLBACK_FUNC_t)(uint16_t reg_addr, void *data, REPLY_FRAME_Typedef_t *reply_data);
typedef struct
{
  uint16_t reg_addr;
  uint16_t reg_size;
  REG_CALLBACK_FUNC_t func;
}REG_PROCESS_MAP_Typedef_t;

/** Public variables ---------------------------------------------------------*/

/** Private variables --------------------------------------------------------*/
static PROTOCOL_STACK_HANDLE_Typedef_t *Stack_Opt_Handle = NULL;/**< 协议栈操作句柄*/

static PROTOCOL_STACK_HANDLE_Typedef_t UART_StackHandle;

static SEND_TASK_LIST_Typedef_t Send_Task_Handle;
/** Private function prototypes ----------------------------------------------*/
/*回复处理*/
static RETURN_TYPE_Typedef_t get_soft_version(uint16_t reg_addr, void* data, REPLY_FRAME_Typedef_t *reply_data);
static RETURN_TYPE_Typedef_t get_set_all_par(uint16_t reg_addr, void* data, REPLY_FRAME_Typedef_t *reply_data);

/*解析处理*/
static RETURN_TYPE_Typedef_t Decode_Frame_Data(REPLY_FRAME_Typedef_t *reply_data);
static FRAME_CHECK_STATUS_Typedef_t Frame_Check_Parse(CQ_handleTypeDef *cb, FRAME_CHECK_RESULT_Typedef_t *result);
static void Reply_Data_Frame(REPLY_FRAME_Typedef_t *reply_data);
static int16_t Get_Reg_Index(uint16_t reg_addr);
static void Check_Frame_Not_Full(PROTOCOL_STACK_HANDLE_Typedef_t *pStack_Opt_Handle);
/** Private constants --------------------------------------------------------*/
/*寄存器地址与处理解析映射表*/
static const REG_PROCESS_MAP_Typedef_t reg_process_map_list[] = 
{
  {REG_SOFT_VER,  2, get_soft_version},
  {0xFFFF, 1, get_set_all_par},
};

static const uint16_t reg_num_max = sizeof(reg_process_map_list)/sizeof( reg_process_map_list[0]);
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
  * @brief   待回复任务检测
  * @param   [in]force true强制发送.
  * @return  true 存在待回复任务.
  * @author  aron566
  * @version V1.0
  * @date    2021-08-04
  ******************************************************************
  */
static bool Check_Wait_Send_Task(bool force)
{
  if(Send_Task_Handle.Wait_Send_Size == 0)
  {
    return false;
  }
  uint32_t Elapsed_Time_ms = Timer_Port_Get_Current_Time(TIMER_MS) - Send_Task_Handle.Last_Send_Time_ms;
  if(Elapsed_Time_ms < ENABLE_SEND_DELAY_MS && force == false)
  {
    return true;
  }
  Send_Task_Handle.Last_Send_Time_ms = Timer_Port_Get_Current_Time(TIMER_MS);
  uint16_t Can_Send_Size = (Send_Task_Handle.Data_Total_Size - Send_Task_Handle.Current_Send_Index)>SEND_ONE_PACKET_SIZE_MAX?SEND_ONE_PACKET_SIZE_MAX:(Send_Task_Handle.Data_Total_Size - Send_Task_Handle.Current_Send_Index);
  Uart_Port_Transmit_Data(Send_Task_Handle.Stack_Opt_Handle_Ptr->Uart_Opt_Handle, Send_Task_Handle.Buf_Ptr+Send_Task_Handle.Current_Send_Index, Can_Send_Size, 0);
  Send_Task_Handle.Current_Send_Index = (Send_Task_Handle.Current_Send_Index + SEND_ONE_PACKET_SIZE_MAX) > Send_Task_Handle.Data_Total_Size?Send_Task_Handle.Current_Send_Index:(Send_Task_Handle.Current_Send_Index + SEND_ONE_PACKET_SIZE_MAX);
  Send_Task_Handle.Wait_Send_Size -= Can_Send_Size;
  return true;
}

/**
  ******************************************************************
  * @brief   帧未满超时检测
  * @param   [in]pStack_Opt_Handle 协议栈句柄.
  * @return  None.
  * @author  aron566
  * @version V1.0
  * @date    2021-08-04
  ******************************************************************
  */
static inline void Check_Frame_Not_Full(PROTOCOL_STACK_HANDLE_Typedef_t *pStack_Opt_Handle)
{
  uint32_t Time_Sec = Timer_Port_Get_Current_Time(TIMER_SEC);
  if(Stack_Opt_Handle->NotFull_LastTime > 0)
  {
    if(Time_Sec != Stack_Opt_Handle->Update_Time)
    {
      Stack_Opt_Handle->Update_Time = Time_Sec;
      Stack_Opt_Handle->NotFull_LastTime--;
    }
  }
  else
  {
    CQ_ManualOffsetInc(pStack_Opt_Handle->Uart_Opt_Handle->cb, 1);
    Stack_Opt_Handle->NotFull_LastTime = NOT_FULL_TIMEOUT_SEC_MAX;
  }
}

/**
  ******************************************************************
  * @brief   协议解析入口
  * @param   [out]Reply_Data 发送数据信息.
  * @return  RETURN_TYPE_Typedef_t
  * @author  aron66
  * @version V1.0
  * @date    2021-08-04
  ******************************************************************
  */
static RETURN_TYPE_Typedef_t Decode_Frame_Data(REPLY_FRAME_Typedef_t *Reply_Data)
{ 
  FRAME_CHECK_RESULT_Typedef_t result;
  RETURN_TYPE_Typedef_t ret = RETURN_OK;
  
  
  /*串口操作句柄为空时直接返回*/
  if(Stack_Opt_Handle->Uart_Opt_Handle == NULL)
  {
    return RETURN_ERROR;
  }
  
  /*报文检测*/
  FRAME_CHECK_STATUS_Typedef_t check_state = Frame_Check_Parse(Stack_Opt_Handle->Uart_Opt_Handle->cb, &result);
  if(check_state != LONG_FRAME_CHECK_OK && check_state != SHORT_FRAME_CHECK_OK)
  {
    return RETURN_ERROR;
  }

  /*获得功能码*/
  uint8_t cmd = result.frame_data[2];
  uint16_t reg_addr = result.frame_data[4] << 8;
  reg_addr |= result.frame_data[3];
  uint8_t *data = result.frame_data + result.DataOffset;
  
  /*调用功能*/
  int16_t idx = Get_Reg_Index(reg_addr);
  if(idx == -1)
  {
    return RETURN_ERROR;
  }

  Reply_Data->cmd = (COMMAND_TYPE_Typedef_t)cmd;
  ret = reg_process_map_list[idx].func(reg_addr, data, Reply_Data);
  if(ret != RETURN_OK)
  {
    return RETURN_ERROR;
  }
  
  /*发送回复数据*/
  Reply_Data_Frame(Reply_Data);
  
  return RETURN_OK;
}

/**
  ******************************************************************
  * @brief   报文检查
  * @param   [in]cb 缓冲区句柄
  * @param   [out]result 报文检测结果
  * @return  FRAME_CHECK_STATUS_Typedef_t 报文检测状态
  * @author  aron566
  * @version V1.0
  * @date    2021-08-04
  ******************************************************************
  */
static FRAME_CHECK_STATUS_Typedef_t Frame_Check_Parse(CQ_handleTypeDef *cb, FRAME_CHECK_RESULT_Typedef_t *result)
{
  static uint8_t databuf[GET_FRAME_SIZE_MAX];
  
  uint32_t rxnb = CQ_getLength(cb);
  if(IS_LESS_MIN_FRAME_SIZE(rxnb))
  {
    return FRAME_DATA_NOT_FULL;
  }
  
  /*判断帧头,跳过无效帧头*/
  if(CQ_ManualGet_Offset_Data(cb, 0) != 0x7A || CQ_ManualGet_Offset_Data(cb, 1) != 0x55)
  {
    CQ_ManualOffsetInc(cb, 1);
    if(CQ_skipInvaildU8Header(cb, 0x7A) == 0)
    {
      return FRAME_DATA_NOT_FULL;
    }
  }
  
  /*判断可读数据是否小于最小帧长*/
  rxnb = CQ_getLength(cb);
  if(IS_LESS_MIN_FRAME_SIZE(rxnb))
  {
    return FRAME_DATA_NOT_FULL;
  }

  /*判断帧类型*/
  uint8_t cmd = CQ_ManualGet_Offset_Data(cb, 2);
  uint16_t length = 0;
  uint32_t package_len = 0;
  switch((COMMAND_TYPE_Typedef_t)cmd)
  {
    /*写入命令*/
    case SET_PAR_CMD:
      length = CQ_ManualGet_Offset_Data(cb, 6) << 8;
      length += CQ_ManualGet_Offset_Data(cb, 5);
      package_len = SET_FRAME_PACKAGE_LEN(length);
      package_len = (package_len > GET_FRAME_SIZE_MAX)?GET_FRAME_SIZE_MAX:package_len;
      if(rxnb < package_len)
      {
        Check_Frame_Not_Full(Stack_Opt_Handle);
        return FRAME_DATA_NOT_FULL;
      }
      
      CQ_ManualGetData(cb, databuf, package_len);
      if(modbus_get_crc_result(databuf, package_len-2) == true)
      {
        result->frame_data = databuf;
        result->DataLen = length;
        result->DataOffset = 7;
        CQ_ManualOffsetInc(cb, package_len);
        return LONG_FRAME_CHECK_OK;
      }
      break;
    /*读取命令*/
    case GET_PAR_CMD:
      package_len = READ_FRAME_PACKAGE_LEN;
      package_len = (package_len > GET_FRAME_SIZE_MAX)?GET_FRAME_SIZE_MAX:package_len;
       
      if(rxnb < package_len)
      {
        Check_Frame_Not_Full(Stack_Opt_Handle);
        return FRAME_DATA_NOT_FULL;
      }
      
      CQ_ManualGetData(cb, databuf, package_len);
      if(modbus_get_crc_result(databuf, package_len-2) == true)
      {
        result->frame_data = databuf;
        CQ_ManualOffsetInc(cb, package_len);
        return SHORT_FRAME_CHECK_OK;
      }
      break;
    default:
      /*TODO*/
      CQ_ManualOffsetInc(cb, 1);
      return UNKNOW_FRAME_ERROR;
  }
  
  CQ_ManualOffsetInc(cb, 1);
  return SHORT_FRAME_CRC_ERROR;
  
}

/**
  ******************************************************************
  * @brief   回复数据
  * @param   [out]reply_data 发送数据信息.
  * @return  None.
  * @author  aron566
  * @version V1.0
  * @date    2021-08-04
  ******************************************************************
  */
static void Reply_Data_Frame(REPLY_FRAME_Typedef_t *reply_data)
{
  static uint8_t reply_buf[REPLY_FRAME_SIZE_MAX] = {0x75, 0xAA};
  uint16_t index = 2;
  reply_buf[index++] = (uint8_t)reply_data->cmd;

  if((COMMAND_TYPE_Typedef_t)reply_data->cmd == GET_PAR_CMD)
  {
    reply_buf[index++] = reply_data->data_len&0x00FF;
    reply_buf[index++] = reply_data->data_len>>8;
    for(uint16_t data_index = 0; data_index < reply_data->data_len; data_index++)
    {
      reply_buf[index++] = reply_data->data_buf[data_index];
    }
    uint16_t crc_val = modbus_crc_return_with_table(reply_buf, index);
    reply_buf[index++] = (uint8_t)(crc_val&0x00FF);
    reply_buf[index++] = (uint8_t)((crc_val>>8)&0xFF);
  }
  else if((COMMAND_TYPE_Typedef_t)reply_data->cmd == SET_PAR_CMD)
  {
    reply_buf[index++] = (uint8_t)reply_data->ack;
  }
  
  /*检测发送大小是否需要分包发送*/
#if ENABLE_SEND_DELAY
  if(index > ENABLE_SEND_DELAY_LIMIT_SIZE && Stack_Opt_Handle == &UART_StackHandle)
  {
    Send_Task_Handle.Current_Send_Index = 0;
    Send_Task_Handle.Data_Total_Size = index;
    Send_Task_Handle.Wait_Send_Size = index;
    Send_Task_Handle.Stack_Opt_Handle_Ptr = Stack_Opt_Handle;
    Send_Task_Handle.Buf_Ptr = reply_buf;
    Send_Task_Handle.Last_Send_Time_ms = Timer_Port_Get_Current_Time(TIMER_MS);
    Check_Wait_Send_Task(true);
    return;
  }
#endif 
  
  /*直接发送*/
  Uart_Port_Transmit_Data(Stack_Opt_Handle->Uart_Opt_Handle, reply_buf, index, 0);
#if ENABLE_DEBUG_PROTOCL  
  for (uint32_t i = 0; i < index; i++)
  printf("%02X ",
      reply_buf[i]);
  printf("\n");
#endif  
}

/**
  ******************************************************************
  * @brief   获取功能码索引号
  * @param   [in]reg_addr 寄存器地址
  * @return  AnalyzRslt
  * @author  aron566
  * @version V1.0
  * @date    2021-08-04
  ******************************************************************
  */
static int16_t Get_Reg_Index(uint16_t Reg_Addr)
{
  for(int16_t index = 0; index < reg_num_max; index++)
  {
    if(Reg_Addr == reg_process_map_list[index].reg_addr)
    {
      return index;					
    }
  }
  return reg_num_max-1;
}

/**
  ******************************************************************
  * @brief   功能-获取软件版本
  * @param   [in]reg_addr 寄存器地址
  * @param   [in]data 数据
  * @param   [out]reply_data 回复数据存储区
  * @return  RETURN_OK 正常
  * @author  aron566
  * @version V1.0
  * @date    2021-08-04
  ******************************************************************
  */
static RETURN_TYPE_Typedef_t get_soft_version(uint16_t reg_addr, void *data, REPLY_FRAME_Typedef_t *reply_data)
{
  if((COMMAND_TYPE_Typedef_t)reply_data->cmd == GET_PAR_CMD)
  {
    reply_data->data_len = 2;
    reply_data->data_buf[0] = (uint8_t)(((uint16_t)(SOFTWARE_V))>>8);
    reply_data->data_buf[1] = (uint8_t)(((uint16_t)(SOFTWARE_V))&0x00FF);
    return RETURN_OK;
  }
  return RETURN_ERROR;
}

/**
  ******************************************************************
  * @brief   读写参数
  * @param   [in]reg_addr 寄存器地址
  * @param   [in]data 数据
  * @param   [out]reply_data 回复数据存储区
  * @return  None
  * @author  aron566
  * @version V1.0
  * @date    2021-08-04
  ******************************************************************
  */
static RETURN_TYPE_Typedef_t get_set_all_par(uint16_t reg_addr, void* data, REPLY_FRAME_Typedef_t *reply_data)
{
  if((COMMAND_TYPE_Typedef_t)reply_data->cmd == SET_PAR_CMD)
  {
    if(Parameter_Port_Set_Par(reg_addr, (uint8_t *)data) == false)
    {
      return RETURN_ERROR;
    }
    reply_data->ack = REPLY_ACK_SET_OK;
  }
  else if((COMMAND_TYPE_Typedef_t)reply_data->cmd == GET_PAR_CMD)
  {
    /*判断是否是读取全部参数寄存器*/
    if(reg_addr == 0x0F00)
    {
      reply_data->data_len = 40;
    }
    else
    {
      reply_data->data_len = 1;
    }
    if(Parameter_Port_Get_Par(reg_addr, reply_data->data_buf) == false)
    {
      return RETURN_ERROR;
    }
  }
  return RETURN_OK;
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
  * @brief   获取寄存器数值长度
  * @param   [in]Reg_Addr 寄存器地址.
  * @return  字节数
  * @author  aron566
  * @version V1.0
  * @date    2021-07-23
  ******************************************************************
  */
uint16_t Main_Protocol_Get_Reg_Value_Size(uint16_t Reg_Addr)
{
  for(int16_t index = 0; index < reg_num_max; index++)
  {
    if(Reg_Addr == reg_process_map_list[index].reg_addr)
    {
      return reg_process_map_list[index].reg_size;					
    }
  }
  return 0;
}

/**
  ******************************************************************
  * @brief   设置寄存器数值
  * @param   [in]Reg_Addr 寄存器地址.
  * @param   [in]Data 寄存器数值.
  * @return  None
  * @author  aron566
  * @version V1.0
  * @date    2021-07-23
  ******************************************************************
  */
void Main_Protocol_Set_Reg_Value(uint16_t Reg_Addr, const uint8_t *Data)
{
  REPLY_FRAME_Typedef_t reply_i2c_data;
  
  /*调用功能*/
  int16_t idx = Get_Reg_Index(Reg_Addr);
  if(idx == -1)
  {
    return;
  }

  reply_i2c_data.cmd = (COMMAND_TYPE_Typedef_t)SET_PAR_CMD;
  reg_process_map_list[idx].func(Reg_Addr, (void *)Data, &reply_i2c_data);
}

/**
  ******************************************************************
  * @brief   获取寄存器数值
  * @param   [in]Reg_Addr 寄存器地址.
  * @param   [in]Buf 寄存器数值存储区.
  * @return  None
  * @author  aron566
  * @version V1.0
  * @date    2021-07-23
  ******************************************************************
  */
void Main_Protocol_Get_Reg_Value(uint16_t Reg_Addr, uint8_t *Buf)
{
  REPLY_FRAME_Typedef_t reply_i2c_data;
  RETURN_TYPE_Typedef_t ret = RETURN_OK;
  
  /*调用功能*/
  int16_t idx = Get_Reg_Index(Reg_Addr);
  if(idx == -1)
  {
    return;
  }

  reply_i2c_data.cmd = (COMMAND_TYPE_Typedef_t)GET_PAR_CMD;
  ret = reg_process_map_list[idx].func(Reg_Addr, NULL, &reply_i2c_data);
  if(ret != RETURN_OK)
  {
    return;
  }
  
  memmove(Buf, reply_i2c_data.data_buf, reply_i2c_data.data_len);
}

/**
  ******************************************************************
  * @brief   启动协议栈
  * @param   [in]None.
  * @return  None
  * @author  aron566
  * @version V1.0
  * @date    2021-08-04
  ******************************************************************
  */
void Protocol_Stack_Start(void)
{
  static REPLY_FRAME_Typedef_t reply_data = {0};
  
  /*分包回复任务检测*/
  if(Check_Wait_Send_Task(false) == true)
  {
    return;
  }
  
  /*串口数据解析*/
  Stack_Opt_Handle = &UART_StackHandle;
  Decode_Frame_Data(&reply_data);
}

/**
  ******************************************************************
  * @brief   协议栈初始化
  * @param   [in]None.
  * @return  None
  * @author  aron566
  * @version V1.0
  * @date    2021-08-04
  ******************************************************************
  */
void Protocol_Stack_Init(void)
{
  /*获取串口操作句柄*/
  memset(&UART_StackHandle, 0, sizeof(PROTOCOL_STACK_HANDLE_Typedef_t));
  UART_StackHandle.NotFull_LastTime = NOT_FULL_TIMEOUT_SEC_MAX;
  UART_StackHandle.Uart_Opt_Handle = Uart_Port_Get_Handle(UART_NUM_1);
  if(UART_StackHandle.Uart_Opt_Handle == NULL)
  {
    printf("get uart opt ble handle faild.\n");
    return;
  }
}

#ifdef __cplusplus ///<end extern c
}
#endif
/******************************** End of file *********************************/
