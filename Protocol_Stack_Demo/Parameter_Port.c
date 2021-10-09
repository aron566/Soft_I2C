/**
 *  @file Parameter_Port.c
 *
 *  @date 2021-02-26
 *
 *  @author aron566
 *
 *  @copyright 爱谛科技研究院.
 *
 *  @brief 参数存储操作
 *
 *  @details 1、
 *
 *  @version V1.0
 */
#ifdef __cplusplus ///<use C compiler
extern "C" {
#endif
/** Includes -----------------------------------------------------------------*/
/* Private includes ----------------------------------------------------------*/
#include "Parameter_Port.h"
#include "main.h"
/** Private typedef ----------------------------------------------------------*/

/** Private macros -----------------------------------------------------------*/
#define REG_NUM_SIZE        40U /**< 寄存器参数个数*/

/*I2C设备地址*/
#define LEFT_SLAVE_ADDR     0x06/**< 左设7Bit地址*/
#define RIGHT_SLAVE_ADDR    0x05/**< 右设7Bit地址*/

#define LEFT_SLAVE_ADDR_W   (LEFT_SLAVE_ADDR<<1)
#define LEFT_SLAVE_ADDR_R   ((LEFT_SLAVE_ADDR<<1)|0x01)

#define RIGHT_SLAVE_ADDR_W  (RIGHT_SLAVE_ADDR<<1)
#define RIGHT_SLAVE_ADDR_R  ((RIGHT_SLAVE_ADDR<<1)|0x01)  
  
/*单次I2C数据大小*/
#define I2C_FRAME_SET_SIZE  3U		/**< 16bit寄存器地*/
#define I2C_FRAME_READ_SIZE	100U	/**< 读取长度*/
#define READ_PAR_REG        0x0F00/**< 读取参数*/
/** Private constants --------------------------------------------------------*/
/** Public variables ---------------------------------------------------------*/
extern I2C_HandleTypeDef hi2c1;
/** Private variables --------------------------------------------------------*/
/*缓冲区*/
static uint8_t data[I2C_FRAME_READ_SIZE+I2C_FRAME_SET_SIZE] = {0}; 

/*参数表*/
static REG_PAR_Typedef_t LEFT_Par_Index_Info[ ] = 
{
  {
    .index = 0,
    .reg = 0x0055,
  },
  {
    .index = 2,
    .reg = 0x0155,
  },
  {
    .index = 24,
    .reg = 0x0C55,
  },
  {
    .index = 26,
    .reg = 0x0D55,
  },
  {
    .index = 28,
    .reg = 0x0E55,
  },
  {
    .index = 30,
    .reg = 0x0F55,
  },
  {
    .index = 32,
    .reg = 0x1055,
  },
  {
    .index = 34,
    .reg = 0x1155,
  },
  {
    .index = 36,
    .reg = 0x1255,
  },
  {
    .index = 38,
    .reg = 0x1355,
  },
  {
    .index = 40,
    .reg = 0x1455,
  },
  {
    .index = 42,
    .reg = 0x1555,
  },
  {
    .index = 44,
    .reg = 0x1655,
  },
  {
    .index = 46,
    .reg = 0x1755,
  },
  {
    .index = 48,
    .reg = 0x1855,
  },
  {
    .index = 51,
    .reg = 0x1955,
  },
  {
    .index = 53,
    .reg = 0x1A55,
  },
  {
    .index = 55,
    .reg = 0x1B55,
  },
  {
    .index = 57,
    .reg = 0x1C55,
  },
  {
    .index = 59,
    .reg = 0x1D55,
  },
  {
    .index = 61,
    .reg = 0x1E55,
  },
  {
    .index = 63,
    .reg = 0x1F55,
  },  
  {
    .index = 65,
    .reg = 0x2055,
  },
  {
    .index = 67,
    .reg = 0x2155,
  },
  {
    .index = 80,
    .reg = 0x2655,
  },
  {
    .index = 81,
    .reg = 0x2755,
  },
  {
    .index = 82,
    .reg = 0x2855,
  },
  {
    .index = 83,
    .reg = 0x2955,
  },
  {
    .index = 84,
    .reg = 0x3055,
  },
  {
    .index = 85,
    .reg = 0x3155,
  },
  {
    .index = 86,
    .reg = 0x3255,
  },
  {
    .index = 87,
    .reg = 0x3355,
  },
  {
    .index = 88,
    .reg = 0x3455,
  },
  {
    .index = 89,
    .reg = 0x3555,
  },
  {
    .index = 90,
    .reg = 0x3655,
  },
  {
    .index = 91,
    .reg = 0x3755,
  },
  {
    .index = 92,
    .reg = 0x3855,
  },
  {
    .index = 93,
    .reg = 0x3955,
  },
  {
    .index = 94,
    .reg = 0x4055,
  },
  {
    .index = 95,
    .reg = 0x4155,
  },
  {
    .index = 0,
    .reg = 0x4255,  /**< 静音使能*/
  },
  {
    .index = 0,
    .reg = 0x4355,  /**< 恢复声音*/
  },
  {
    .index = 0,
    .reg = 0x0F00,  /**< 场景模式*/
  }, 
  {
    .index = 0,
    .reg = 0x4455,  /**< 一次性写入参数*/
  },   
  {
    .index = 0,
    .reg = 0x0001,  /**< 通道选择*/
  },    
};

size_t Par_Num = sizeof(LEFT_Par_Index_Info)/sizeof(LEFT_Par_Index_Info[0]);

static REG_PAR_Typedef_t RIGHT_Par_Index_Info[REG_NUM_SIZE];

static REG_PAR_Typedef_t *pCurrent_Channel_Reg_Par = LEFT_Par_Index_Info;
static DEVICE_CHANNEL_SEL_Typedef_t Current_Channel_Sel = LEFT_DEVICE_SEL;

/** Private function prototypes ----------------------------------------------*/
extern void Soft_I2C_Init(void);
extern HAL_StatusTypeDef HAL_I2C_Master_Receivex(void *x, uint8_t Saddr, uint8_t *Buf, uint16_t Size, uint32_t Block_Time);
extern HAL_StatusTypeDef HAL_I2C_Master_Transmitx(void *x, uint8_t Saddr, const uint8_t *Data, uint16_t Size, uint32_t Block_Time);
extern bool Soft_I2C_Is_Free(void);
/** Private user code --------------------------------------------------------*/

/** Private application code -------------------------------------------------*/
/*******************************************************************************
*
*       Static code
*
********************************************************************************
*/

static bool Read_Par(DEVICE_CHANNEL_SEL_Typedef_t Channel, uint8_t Scene_Mode)
{
  uint8_t SAddrW = 0, SAddrR = 0;
  REG_PAR_Typedef_t *Opt_Ptr = NULL;
  /*设备选择*/
  if(Channel == LEFT_DEVICE_SEL)
  {
    SAddrW = LEFT_SLAVE_ADDR_W;
    SAddrR = LEFT_SLAVE_ADDR_R;
    Opt_Ptr = LEFT_Par_Index_Info;
  }
  else if(Channel == RIGHT_DEVICE_SEL)
  {
    SAddrW = RIGHT_SLAVE_ADDR_W;
    SAddrR = RIGHT_SLAVE_ADDR_R;
    Opt_Ptr = RIGHT_Par_Index_Info;  
  }
  else
  {
    return false;
  }
  
  /*读取*/
  /*step 1 设置操作的寄存器地址*/
//  data[0] = (uint8_t)(READ_PAR_REG >> 8)&0xFF;
//  data[1] = (uint8_t)READ_PAR_REG&0xFF;
//  /*step 2 发出命令包设置读 x场景模式参数*/
//  data[2] = Scene_Mode;
//  //while(HAL_I2C_GetState(&hi2c1) != HAL_I2C_STATE_READY);//等待总线空闲
//  while(Soft_I2C_Is_Free() == false);
//  if(HAL_I2C_Master_Transmitx(&hi2c1, SAddrW, data, I2C_FRAME_SET_SIZE, 5000) != HAL_OK)
//  {
//    return false;
//  }

  /*step 3 再读*/
  //while(HAL_I2C_GetState(&hi2c1) != HAL_I2C_STATE_READY);//等待总线空闲
  while(Soft_I2C_Is_Free() == false);
  if(HAL_I2C_Master_Receivex(&hi2c1, SAddrR, data, I2C_FRAME_READ_SIZE, 5000) != HAL_OK)
  {
    return false;
  }
  
  for(size_t i = 0; i < REG_NUM_SIZE; i++)
  {
    Opt_Ptr[i].reg_val = data[Opt_Ptr[i].index];
  }
  
  return true;
}

static bool Set_Par(DEVICE_CHANNEL_SEL_Typedef_t Channel, uint16_t RegAddr, uint8_t *Val)
{
  uint8_t SAddrW = 0;
  /*设备选择*/
  if(Channel == LEFT_DEVICE_SEL)
  {
    SAddrW = LEFT_SLAVE_ADDR_W;
  }
  else if(Channel == RIGHT_DEVICE_SEL)
  {
    SAddrW = RIGHT_SLAVE_ADDR_W;
  }
  /*step 1 设置操作的寄存器地址*/
  data[0] = (uint8_t)(RegAddr >> 8)&0xFF;
  data[1] = (uint8_t)RegAddr&0xFF;
  
  uint16_t Data_Size = RegAddr == 0x4455?38:1;
  /*step 2 设置数值*/
  memmove(data+2, Val, Data_Size);

  //while(HAL_I2C_GetState(&hi2c1) != HAL_I2C_STATE_READY);//等待总线空闲
  while(Soft_I2C_Is_Free() == false);
  if(HAL_I2C_Master_Transmitx(&hi2c1, SAddrW, data, Data_Size + 2, 5000) != HAL_OK)
  {
    return false;
  }
  
  /*当寄存器为一次性写入参数，触发读取，更新本地参数*/
  if(RegAddr == 0x4455)
  {
    Read_Par(Channel, 0x0A);
  }
  return true;
}

static void dbug_print(void)
{
  REG_PAR_Typedef_t *Current_Channel_Reg_Par_Ptr = NULL;
  printf("Current channel sel is :");
  if(Current_Channel_Sel == LEFT_DEVICE_SEL)
  {
    Current_Channel_Reg_Par_Ptr = LEFT_Par_Index_Info; 
    printf(" LEFT CHANNEL\r\n");
  }
  else if(Current_Channel_Sel == RIGHT_DEVICE_SEL)
  {
    Current_Channel_Reg_Par_Ptr = RIGHT_Par_Index_Info; 
    printf(" RIGHT CHANNEL\r\n");
  }
  else
  {
    printf(" BOTH CHANNEL Error, must be select one channel\r\n");
    return;
  }
  
  printf("Current channel par show :\r\n");
  uint8_t index = 0;
  printf("%-*.*s\t%hhu\r\n", 20, 20, "音量:", Current_Channel_Reg_Par_Ptr[index++].reg_val);
  printf("%-*.*s\t%hhu\r\n", 20, 20, "降噪:", Current_Channel_Reg_Par_Ptr[index++].reg_val);
  printf("%-*.*s\t%hhu\r\n", 20, 20, "通道一静止区到放大区:", Current_Channel_Reg_Par_Ptr[index++].reg_val);
  printf("%-*.*s\t%hhu\r\n", 20, 20, "通道一放大区到压缩区:", Current_Channel_Reg_Par_Ptr[index++].reg_val);
  printf("%-*.*s\t%hhu\r\n", 20, 20, "通道一压缩区到限制区:", Current_Channel_Reg_Par_Ptr[index++].reg_val);
  printf("%-*.*s\t%hhu\r\n", 20, 20, "通道二静止区到放大区:", Current_Channel_Reg_Par_Ptr[index++].reg_val);
  printf("%-*.*s\t%hhu\r\n", 20, 20, "通道二放大区到压缩区:", Current_Channel_Reg_Par_Ptr[index++].reg_val);
  printf("%-*.*s\t%hhu\r\n", 20, 20, "通道二压缩区到限制区:", Current_Channel_Reg_Par_Ptr[index++].reg_val);
  printf("%-*.*s\t%hhu\r\n", 20, 20, "通道三静止区到放大区:", Current_Channel_Reg_Par_Ptr[index++].reg_val);
  printf("%-*.*s\t%hhu\r\n", 20, 20, "通道三放大区到压缩区:", Current_Channel_Reg_Par_Ptr[index++].reg_val);
  printf("%-*.*s\t%hhu\r\n", 20, 20, "通道三压缩区到限制区:", Current_Channel_Reg_Par_Ptr[index++].reg_val);
  printf("%-*.*s\t%hhu\r\n", 20, 20, "通道四静止区到放大区:", Current_Channel_Reg_Par_Ptr[index++].reg_val);
  printf("%-*.*s\t%hhu\r\n", 20, 20, "通道四放大区到压缩区:", Current_Channel_Reg_Par_Ptr[index++].reg_val);
  printf("%-*.*s\t%hhu\r\n", 20, 20, "通道四压缩区到限制区:", Current_Channel_Reg_Par_Ptr[index++].reg_val);
  printf("%-*.*s\t0x%02X\r\n", 20, 20, "通道一放大系数:", Current_Channel_Reg_Par_Ptr[index++].reg_val);
  printf("%-*.*s\t0x%02X\r\n", 20, 20, "通道一压缩系数:", Current_Channel_Reg_Par_Ptr[index++].reg_val);
  printf("%-*.*s\t0x%02X\r\n", 20, 20, "通道二放大系数:", Current_Channel_Reg_Par_Ptr[index++].reg_val);
  printf("%-*.*s\t0x%02X\r\n", 20, 20, "通道二压缩系数:", Current_Channel_Reg_Par_Ptr[index++].reg_val);
  printf("%-*.*s\t0x%02X\r\n", 20, 20, "通道三放大系数:", Current_Channel_Reg_Par_Ptr[index++].reg_val);
  printf("%-*.*s\t0x%02X\r\n", 20, 20, "通道三压缩系数:", Current_Channel_Reg_Par_Ptr[index++].reg_val);
  printf("%-*.*s\t0x%02X\r\n", 20, 20, "通道四放大系数:", Current_Channel_Reg_Par_Ptr[index++].reg_val);
  printf("%-*.*s\t0x%02X\r\n", 20, 20, "通道四压缩系数:", Current_Channel_Reg_Par_Ptr[index++].reg_val);
  printf("%-*.*s\t%02X\r\n", 20, 20, "场景参数:", Current_Channel_Reg_Par_Ptr[index++].reg_val);
  printf("%-*.*s\t%02X\r\n", 20, 20, "保存参数:", Current_Channel_Reg_Par_Ptr[index++].reg_val); 
  printf("%-*.*s\t%hhu\r\n", 20, 20, "EQ_CH0:", Current_Channel_Reg_Par_Ptr[index++].reg_val);
  printf("%-*.*s\t%hhu\r\n", 20, 20, "EQ_CH1:", Current_Channel_Reg_Par_Ptr[index++].reg_val);
  printf("%-*.*s\t%hhu\r\n", 20, 20, "EQ_CH2:", Current_Channel_Reg_Par_Ptr[index++].reg_val);
  printf("%-*.*s\t%hhu\r\n", 20, 20, "EQ_CH3:", Current_Channel_Reg_Par_Ptr[index++].reg_val);
  printf("%-*.*s\t%hhu\r\n", 20, 20, "EQ_CH4:", Current_Channel_Reg_Par_Ptr[index++].reg_val);
  printf("%-*.*s\t%hhu\r\n", 20, 20, "EQ_CH5:", Current_Channel_Reg_Par_Ptr[index++].reg_val);
  printf("%-*.*s\t%hhu\r\n", 20, 20, "EQ_CH6:", Current_Channel_Reg_Par_Ptr[index++].reg_val);
  printf("%-*.*s\t%hhu\r\n", 20, 20, "EQ_CH7:", Current_Channel_Reg_Par_Ptr[index++].reg_val);
  printf("%-*.*s\t%hhu\r\n", 20, 20, "EQ_CH8:", Current_Channel_Reg_Par_Ptr[index++].reg_val);
  printf("%-*.*s\t%hhu\r\n", 20, 20, "EQ_CH9:", Current_Channel_Reg_Par_Ptr[index++].reg_val);
  printf("%-*.*s\t%hhu\r\n", 20, 20, "EQ_CH10:", Current_Channel_Reg_Par_Ptr[index++].reg_val);
  printf("%-*.*s\t%hhu\r\n", 20, 20, "EQ_CH11:", Current_Channel_Reg_Par_Ptr[index++].reg_val);
  printf("%-*.*s\t%hhu\r\n", 20, 20, "EQ_CH12:", Current_Channel_Reg_Par_Ptr[index++].reg_val);
  printf("%-*.*s\t%hhu\r\n", 20, 20, "EQ_CH13:", Current_Channel_Reg_Par_Ptr[index++].reg_val);
  printf("%-*.*s\t%hhu\r\n", 20, 20, "EQ_CH14:", Current_Channel_Reg_Par_Ptr[index++].reg_val);
  printf("%-*.*s\t%hhu\r\n", 20, 20, "EQ_CH15:", Current_Channel_Reg_Par_Ptr[index++].reg_val);
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
  * @brief   更新参数
  * @param   [in]Data 100Bytes参数数据
  * @return  None.
  * @author  aron566
  * @version V1.0
  * @date    2021-08-04
  ******************************************************************
  */
void Parameter_Port_Update(const uint8_t *Data)
{
  if(Current_Channel_Sel == LEFT_DEVICE_SEL)
  {
    pCurrent_Channel_Reg_Par = LEFT_Par_Index_Info;
  }
  else if(Current_Channel_Sel == RIGHT_DEVICE_SEL)
  {
    pCurrent_Channel_Reg_Par = RIGHT_Par_Index_Info;  
  } 

  for(size_t i = 0; i < REG_NUM_SIZE; i++)
  {
    pCurrent_Channel_Reg_Par[i].reg_val = Data[pCurrent_Channel_Reg_Par[i].index];
  }
}

/**
  ******************************************************************
  * @brief   设置参数
  * @param   [in]Reg_Addr 寄存器地址
  * @param   [in]Val 数值地址
  * @return  true 正确.
  * @author  aron566
  * @version V1.0
  * @date    2021-08-04
  ******************************************************************
  */
bool Parameter_Port_Set_Par(uint16_t Reg_Addr, uint8_t *Val)
{
  size_t index = 0;
  /*获得寄存器所在索引*/
  for(size_t i = 0; i < Par_Num; i++)
  {
    if(pCurrent_Channel_Reg_Par[i].reg == Reg_Addr)
    {
      index = i;
      break;
    }
  }
  if(index >= Par_Num)
  {
    return false;
  }
  
  /*通道选择设置*/
  if(Reg_Addr == 0x0001)
  {
    Parameter_Port_Update_Channel((DEVICE_CHANNEL_SEL_Typedef_t)(*Val));
    /*切换至双通道，直接返回，不更新参数*/
    if((DEVICE_CHANNEL_SEL_Typedef_t)(*Val) == BOTH_DEVICE_SEL)
    {
      return true;
    }
    /*从新读取 更新参数 -> 获得当前场景模式*/
    size_t Scene_Index = 0;
    for(; Scene_Index < Par_Num; Scene_Index++)
    {
      if(pCurrent_Channel_Reg_Par[Scene_Index].reg == 0x2055)
      {
        break;
      }
    }
    /*读取该通道场景模式下的参数*/
    return Read_Par(Current_Channel_Sel, pCurrent_Channel_Reg_Par[Scene_Index].reg_val);
  }
  
  /*双通道*/
  if(Current_Channel_Sel != LEFT_DEVICE_SEL && Current_Channel_Sel != RIGHT_DEVICE_SEL)
  {
      LEFT_Par_Index_Info[index].reg_val = (*Val);
      RIGHT_Par_Index_Info[index].reg_val = (*Val);
      /*TODO:发送至I2C左右从机*/
      if(Reg_Addr == 0x0F00)
      {
        if(Read_Par(LEFT_DEVICE_SEL, (*Val)) != true)
        {
          return false;
        }
        if(Read_Par(RIGHT_DEVICE_SEL, (*Val)) != true)
        {
          return false;
        }
      }
      else
      {
        if(Set_Par(LEFT_DEVICE_SEL, Reg_Addr, Val) != true)
        {
          return false;
        }
        if(Set_Par(RIGHT_DEVICE_SEL, Reg_Addr, Val) != true)
        {
          return false;
        }
      }
      return true;
  }

  pCurrent_Channel_Reg_Par[index].reg_val = (*Val);
  /*TODO:发送至I2C从机,当操作寄存器为读取寄存器时，会触发读取操作进行更新参数,
         在读此寄存器时会直接返回缓冲区数据*/
  if(Reg_Addr == 0x0F00)
  {
    if(Read_Par(Current_Channel_Sel, (*Val)) != true)
    {
      return false;
    }
  }
  else
  {
    if(Set_Par(Current_Channel_Sel, Reg_Addr, Val) != true)
    {
      return false;
    }
  }
  return true;
}

/**
  ******************************************************************
  * @brief   获取参数
  * @param   [in]Reg_Addr 寄存器地址
  * @param   [in]Buf 数值存储区
  * @return  true 成功.
  * @author  aron566
  * @version V1.0
  * @date    2021-08-04
  ******************************************************************
  */
bool Parameter_Port_Get_Par(uint16_t Reg_Addr, uint8_t *Buf)
{
 if(Current_Channel_Sel == LEFT_DEVICE_SEL)
  {
    pCurrent_Channel_Reg_Par = LEFT_Par_Index_Info;
  }
  else if(Current_Channel_Sel == RIGHT_DEVICE_SEL)
  {
    pCurrent_Channel_Reg_Par = RIGHT_Par_Index_Info;  
  }
  else
  {
    return false;
  }

  /*判断是否是读取全部参数*/
  if(Reg_Addr == 0x0F00)
  {
    /*全部返回给蓝牙主机*/
    for(size_t i = 0; i < REG_NUM_SIZE; i++)
    {
      Buf[i] = pCurrent_Channel_Reg_Par[i].reg_val;
    }
    dbug_print();
    return true;
  }

  /*返回给蓝牙主机*/
  for(size_t i = 0; i < Par_Num; i++)
  {
    if(pCurrent_Channel_Reg_Par[i].reg == Reg_Addr)
    {
      *Buf = pCurrent_Channel_Reg_Par[i].reg_val;
      return true;
    }
  }
  return false;
}

/**
  ******************************************************************
  * @brief   更新通道选择
  * @param   [in]Sel 通道选择
  * @return  None.
  * @author  aron566
  * @version V1.0
  * @date    2021-08-04
  ******************************************************************
  */
void Parameter_Port_Update_Channel(DEVICE_CHANNEL_SEL_Typedef_t Sel)
{
  Current_Channel_Sel = Sel;
  if(Current_Channel_Sel == LEFT_DEVICE_SEL)
  {
    pCurrent_Channel_Reg_Par = LEFT_Par_Index_Info;
  }
  else if(Current_Channel_Sel == RIGHT_DEVICE_SEL)
  {
    pCurrent_Channel_Reg_Par = RIGHT_Par_Index_Info;  
  }  
}

/**
  ******************************************************************
  * @brief   参数接口初始化
  * @param   [in]None
  * @return  None.
  * @author  aron566
  * @version V1.0
  * @date    2021-08-04
  ******************************************************************
  */
void Parameter_Port_Init(void)
{
  memmove(RIGHT_Par_Index_Info, LEFT_Par_Index_Info, sizeof(LEFT_Par_Index_Info));
  
  /*软件I2C初始化*/
  Soft_I2C_Init();
  
  Read_Par(LEFT_DEVICE_SEL, 0x0A);
  Read_Par(RIGHT_DEVICE_SEL, 0x0A);
}

#ifdef __cplusplus ///<end extern c
}
#endif
/******************************** End of file *********************************/
