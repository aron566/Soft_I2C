/**
 *  @file i2c_soft_driver.c
 *
 *  @date 2021-08-15
 *
 *  @author aron566
 *
 *  @copyright Copyright (c) 2021 aron566 <aron566@163.com>.
 *
 *  @brief 软件I2C通讯
 *
 *  @details None
 *
 *  @version V1.1
 */
#ifdef __cplusplus ///<use C compiler
extern "C" {
#endif
/** Includes -----------------------------------------------------------------*/
#include "i2c_soft_driver.h"  
/* Private includes ----------------------------------------------------------*/
/** Private typedef ----------------------------------------------------------*/

/** Private macros -----------------------------------------------------------*/
/**
 * @name CPU频率
 * @{
 */
#define CPU_FREQUENCY_MHZ 8
/** @}*/
/**
 * @name 波特率
 * @{
 */
#define I2C_BAUDRATE_DELAY_US      20/**< 延时us*/
/** @}*/  
/**
 * @name 模拟I2C引脚定义，及配置功能
 * @{
 */
#define MYI2C_SCL_PIN		GPIO_PIN_6
#define MYI2C_SCL_PORT	GPIOB
#define MYI2C_SDA_PIN		GPIO_PIN_7
#define MYI2C_SDA_PORT	GPIOB
/** @}*/
 
#define Delay_us(xx)  delay_xus(xx)

#define SDA_Dout_LOW()          HAL_GPIO_WritePin(MYI2C_SDA_PORT, MYI2C_SDA_PIN, GPIO_PIN_RESET) 
#define SDA_Dout_HIGH()         HAL_GPIO_WritePin(MYI2C_SDA_PORT, MYI2C_SDA_PIN, GPIO_PIN_SET)
#define SDA_Data_IN()           HAL_GPIO_ReadPin(MYI2C_SDA_PORT, MYI2C_SDA_PIN)
#define SCL_Dout_LOW()          HAL_GPIO_WritePin(MYI2C_SCL_PORT, MYI2C_SCL_PIN, GPIO_PIN_RESET) 
#define SCL_Dout_HIGH()         HAL_GPIO_WritePin(MYI2C_SCL_PORT, MYI2C_SCL_PIN, GPIO_PIN_SET)
#define SCL_Data_IN()           HAL_GPIO_ReadPin(MYI2C_SCL_PORT, MYI2C_SCL_PIN)
#define SDA_Write(XX)           HAL_GPIO_WritePin(MYI2C_SDA_PORT, MYI2C_SDA_PIN, (XX?GPIO_PIN_SET:GPIO_PIN_RESET))
/** Private constants --------------------------------------------------------*/
/** Public variables ---------------------------------------------------------*/

/** Private variables --------------------------------------------------------*/
static bool Soft_Free_State = true;
/** Private function prototypes ----------------------------------------------*/
static void delay_xus(__IO uint32_t nTime);
static void SDA_Output(void);
static void SDA_Input(void);
static void SCL_Output(void);
static void SCL_Input(void);
static void I2C_Init(void);
static void I2C_Start(void);
static void I2C_Stop(void);
static bool I2C_Wait_Ack(void);
static void I2C_Ack(void);
static void I2C_NAck(void);
static void I2C_Send_Byte(uint8_t txd);
static uint8_t I2C_Read_Byte(uint8_t ack);

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
	* @brief   模拟I2C us级延时
	* @param   [in]nTime 延时us数
	* @retval  None
	* @author  aron566
	* @version V1.0
	* @date    2021-07-23
	******************************************************************
	*/
static void delay_xus(__IO uint32_t nTime)
{
    int old_val,new_val,val;
 
    if(nTime > 900)
    {
        for(old_val = 0; old_val < nTime/900; old_val++)
        {
            delay_xus(900);
        }
        nTime = nTime%900;
    }
 
    old_val = SysTick->VAL;
    new_val = old_val - CPU_FREQUENCY_MHZ*nTime;
    if(new_val >= 0)
    {
        do
        {
            val = SysTick->VAL;
        }
        while((val < old_val)&&(val >= new_val));
    }
    else
    {
        new_val +=CPU_FREQUENCY_MHZ*1000;
        do
        {
            val = SysTick->VAL;
        }
        while((val <= old_val)||(val > new_val));
    }
}
 
 /**
	******************************************************************
	* @brief   模拟I2C 数据脚配置为输出模式
	* @param   [in]None
	* @retval  None
	* @author  aron566
	* @version V1.0
	* @date    2021-07-23
	******************************************************************
	*/
static void SDA_Output(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.Pin = MYI2C_SDA_PIN;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(MYI2C_SDA_PORT,&GPIO_InitStruct);
}
 
 /**
	******************************************************************
	* @brief   模拟I2C 数据脚配置为输入模式
	* @param   [in]None
	* @retval  None
	* @author  aron566
	* @version V1.0
	* @date    2021-07-23
	******************************************************************
	*/
static void SDA_Input(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.Pin = MYI2C_SDA_PIN;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;/*没有上下拉*/
	GPIO_InitStruct.Speed =GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(MYI2C_SDA_PORT,&GPIO_InitStruct);
}
 
 /**
	******************************************************************
	* @brief   模拟I2C 时钟脚配置为输出模式
	* @param   [in]None
	* @retval  None
	* @author  aron566
	* @version V1.0
	* @date    2021-07-23
	******************************************************************
	*/
static void SCL_Output(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.Pin = MYI2C_SCL_PIN;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Speed =GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(MYI2C_SCL_PORT,&GPIO_InitStruct);
}
 
 /**
	******************************************************************
	* @brief   模拟I2C 时钟脚配置为输入模式
	* @param   [in]None
	* @retval  None
	* @author  aron566
	* @version V1.0
	* @date    2021-07-23
	******************************************************************
	*/
static void SCL_Input(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.Pin = MYI2C_SCL_PIN;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;/*没有上下拉*/
	GPIO_InitStruct.Speed =GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(MYI2C_SCL_PORT,&GPIO_InitStruct);
}

/**
  ******************************************************************
  * @brief   模拟I2C初始化
  * @param   [in]None
  * @retval  None
  * @author  aron566
  * @version V1.0
  * @date    2021-07-23
  ******************************************************************
  */
static void I2C_Init(void)
{	
	SCL_Dout_HIGH();
	SDA_Dout_HIGH();
	SCL_Output();
	SDA_Output();
}
 
/**
  ******************************************************************
  * @brief   模拟I2C发出起始信号
  * @param   [in]None
  * @retval  None
  * @author  aron566
  * @version V1.0
  * @date    2021-07-23
  ******************************************************************
  */
static void I2C_Start(void)
{
  SDA_Dout_HIGH();
	SDA_Output();
	
	SCL_Dout_HIGH();
	Delay_us(I2C_BAUDRATE_DELAY_US);
	SDA_Dout_LOW();
	Delay_us(I2C_BAUDRATE_DELAY_US);
	SCL_Dout_LOW();
}
 
/**
  ******************************************************************
  * @brief   模拟I2C发出停止信号
  * @param   [in]None
  * @retval  None
  * @author  aron566
  * @version V1.0
  * @date    2021-07-23
  ******************************************************************
  */
static void I2C_Stop(void)
{
  SDA_Dout_LOW();
	SDA_Output();
  
	SCL_Dout_LOW();
	Delay_us(I2C_BAUDRATE_DELAY_US);
	SCL_Dout_HIGH();
  
	SDA_Dout_HIGH();
	Delay_us(I2C_BAUDRATE_DELAY_US);						   	
}
 
/**
  ******************************************************************
  * @brief   模拟I2C等待应答
  * @param   [in]None
  * @retval  true 应答成功（SDA脚为低电平）
  * @author  aron566
  * @version V1.0
  * @date    2021-07-23
  ******************************************************************
  */
static bool I2C_Wait_Ack(void)
{
	uint8_t ucErrTime = 0;
	
  /*配置为输入*/
  //SDA_Input();
  
  Delay_us(I2C_BAUDRATE_DELAY_US);
  
	//SDA_Dout_HIGH();Delay_us(1);	   
	SCL_Dout_HIGH();Delay_us(1);
  
	while(SDA_Data_IN())
	{
    /*检测超时*/
		ucErrTime++;
		if(ucErrTime > (I2C_BAUDRATE_DELAY_US))
		{
			I2C_Stop();
			return false;
		}
    Delay_us(1);
	}
  Delay_us(I2C_BAUDRATE_DELAY_US-ucErrTime);
	SCL_Dout_LOW();//时钟输出0 	   
	return true; 
}
 
/**
  ******************************************************************
  * @brief   模拟I2C应答
  * @param   [in]None
  * @retval  None
  * @author  aron566
  * @version V1.0
  * @date    2021-07-23
  ******************************************************************
  */
static void I2C_Ack(void)
{
	SCL_Dout_LOW();

  SDA_Dout_LOW();
	SDA_Output();

	Delay_us(I2C_BAUDRATE_DELAY_US);
	SCL_Dout_HIGH();
	Delay_us(I2C_BAUDRATE_DELAY_US);
	SCL_Dout_LOW();
}
 
/**
  ******************************************************************
  * @brief   模拟I2C不应答
  * @param   [in]None
  * @retval  None
  * @author  aron566
  * @version V1.0
  * @date    2021-07-23
  ******************************************************************
  */
static void I2C_NAck(void)
{
	SCL_Dout_LOW();
  
  SDA_Dout_HIGH();
	SDA_Output();
	
	Delay_us(I2C_BAUDRATE_DELAY_US);
	SCL_Dout_HIGH();
	Delay_us(I2C_BAUDRATE_DELAY_US);
	SCL_Dout_LOW();
}					 				     

/**
  ******************************************************************
  * @brief   模拟I2C发送一个字节
  * @param   [in]txd 发送的字节
  * @retval  None
  * @author  aron566
  * @version V1.0
  * @date    2021-07-23
  ******************************************************************
  */
static void I2C_Send_Byte(uint8_t txd) 
{                        
	uint8_t t;      
	/*拉低时钟开始数据传输，设置数据线默认值*/
  SCL_Dout_LOW();
  
  SDA_Dout_HIGH();
	SDA_Output();
  
	for(t = 0; t < 8; t++)
	{  
		SDA_Write((txd&0x80)>>7);		   
		txd<<=1; 	  
		Delay_us(I2C_BAUDRATE_DELAY_US);
		SCL_Dout_HIGH();
		Delay_us(I2C_BAUDRATE_DELAY_US); 	
		SCL_Dout_LOW();
  }	 
} 	    

/**
  ******************************************************************
  * @brief   模拟I2C读取一个字节 
  * @param   [in]ack ack=1时，发送ACK，ack=0，发送nACK 
  * @retval  读取到的字节
  * @author  aron566
  * @version V1.0
  * @date    2021-07-23
  ******************************************************************
  */
static uint8_t I2C_Read_Byte(uint8_t ack)
{
	uint8_t i, receive = 0;
	//SDA设置为输入
	SDA_Input();
  for(i = 0; i < 8; i++)
	{
		SCL_Dout_LOW();
		Delay_us(I2C_BAUDRATE_DELAY_US);
		SCL_Dout_HIGH();
		receive<<=1;
		if(SDA_Data_IN())receive++;   
		Delay_us(I2C_BAUDRATE_DELAY_US);
  }					 
  if(!ack)I2C_NAck();/**< 发送nACK*/
  else  I2C_Ack(); 	 /**< 发送ACK*/   
  
	return receive;
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
  * @brief   主机发送数据
  * @param   [in]x 无作用
  * @param   [in]Saddr 从机地址需包含读写位
  * @param   [in]Data 发送的数据
  * @param   [in]Size 发送的数据的字节数
  * @param   [in]阻塞时间
  * @return  HAL_OK 成功
  * @author  aron566
  * @version v1.0
  * @date    2021/8/15
  ******************************************************************
  */
HAL_StatusTypeDef HAL_I2C_Master_Transmitx(void *x, uint8_t Saddr, const uint8_t *Data, uint16_t Size, uint32_t Block_Time)
{
  (void)(Block_Time);
  Soft_Free_State = false;
  I2C_Start();
  
  /*发送从机地址*/
  I2C_Send_Byte(Saddr);
  if(I2C_Wait_Ack() == false)
  {
    Soft_Free_State = true;
    return HAL_ERROR;
  }
  
  for(uint16_t i = 0; i < Size; i++)
  {
    I2C_Send_Byte(Data[i]);
    if(I2C_Wait_Ack() == false)
    {
      Soft_Free_State = true;
      return HAL_ERROR;
    }
  }
  I2C_Stop();
  Soft_Free_State = true;
  return HAL_OK;
}

/**
  ******************************************************************
  * @brief   主机接收数据
  * @param   [in]x 无作用
  * @param   [in]Saddr 从机地址需包含读写位
  * @param   [in]Buf 数据接收缓冲区
  * @param   [in]Size 接收数据的字节数
  * @param   [in]阻塞时间
  * @return  HAL_OK 成功
  * @author  aron566
  * @version v1.0
  * @date    2021/8/15
  ******************************************************************
  */
HAL_StatusTypeDef HAL_I2C_Master_Receivex(void *x, uint8_t Saddr, uint8_t *Buf, uint16_t Size, uint32_t Block_Time)
{
  (void)(Block_Time);
  Soft_Free_State = false;
  I2C_Start();
  
  /*发送从机地址*/
  I2C_Send_Byte(Saddr);
  if(I2C_Wait_Ack() == false)
  {
    Soft_Free_State = true;
    return HAL_ERROR;
  }
  
  for(uint16_t i = 0; i < Size; i++)
  {
    Buf[i] = I2C_Read_Byte(((i+1) == Size)?0:1);
  }
  I2C_Stop();
  Soft_Free_State = true;
  return HAL_OK;
}

/**
  ******************************************************************
  * @brief   获取软件I2C空闲状态
  * @param   [in]None.
  * @return  true 空闲.
  * @author  aron566
  * @version v1.0
  * @date    2021/8/24
  ******************************************************************
  */
bool Soft_I2C_Is_Free(void)
{
  return Soft_Free_State;
}

/**
  ******************************************************************
  * @brief   软件I2C初始化
  * @param   [in]None.
  * @return  None.
  * @author  aron566
  * @version v1.0
  * @date    2021/8/15
  ******************************************************************
  */
void Soft_I2C_Init(void)
{
  I2C_Init();
}

#ifdef __cplusplus ///<end extern c
}
#endif
