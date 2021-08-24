# 从机协议栈

源码文件位于==Protocol_Stack_Demo==目录下

## 文件说明

| 文件名           | 功能               | 修改说明                                                     |
| ---------------- | ------------------ | ------------------------------------------------------------ |
| CircularQueue.c  | 环形缓冲区接口文件 | 无需修改                                                     |
| utilities_crc.c  | CRC校验功能        | 无需修改                                                     |
| Main_Protocol.c  | 协议栈接口文件     | 内有主机读取版本寄存器，设置音量寄存器，读取音量寄存器，从机响应示例代码，**需完善其他寄存器的读写操作** |
| Parameter_Port.c | 算法参数接口文件   | 需完善I2C的操作接口                                          |
| Timer_Port.c     | 定时器接口文件     | 需将HAL_TIM_PeriodElapsedCallback放入1ms定时中断中           |
| UART_Port.c      | 串口通讯接口文件   | 需将BLE主机发过来的数据接入到此，并且提供BLE发送数据的实现到此 |

## 移植说明

1、各文件都有各自的`.h`头文件，提供对外的接口及说明。

2、文件中如有`xxx_Init(xxx)`，应先进行初始化调用。

## 示例

```c
/*定时器1ms中断*/
void TIMER1_ISR(void)
{
  /*调用定时器接口*/
  HAL_TIM_PeriodElapsedCallback(NULL);
}

int main(void)
{
  /*初始化定时器*/
  Timer_Port_Init();
  
  /*初始化通讯*/
  Uart_Port_Init();
  
  /*初始化参数*/
  Parameter_Port_Init();
  
  /*初始化协议栈*/
  Protocol_Stack_Init();
  
  /*启动协议栈*/
  for(;;)
  {
    Protocol_Stack_Start();
  }
}
```

