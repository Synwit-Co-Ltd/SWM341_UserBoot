#include "SWM341.h"


void SerialInit(void);

int main(void)
{
 	SystemInit();
	
	SerialInit();
	
	GPIO_Init(GPIOA, PIN0, 0, 1, 0, 0);		// 输入，开上拉
	GPIO_Init(GPIOA, PIN1, 1, 0, 0, 0);		// 输出
	GPIO_SetBit(GPIOA, PIN1);
	
	TIMR_Init(TIMR0, TIMR_MODE_TIMER, CyclesPerUs, 1500000, 1);	// 1.5s 定时中断
	TIMR_Start(TIMR0);
	
	__enable_irq();
	
	while(1==1)
 	{
		printf("Hi from App\n");
		
		for(int i = 0; i < SystemCoreClock/4; i++) __NOP();
		
		
		if(GPIO_GetBit(GPIOA, PIN0) == 0)	// 检测到按键按下，跳到UserBoot
		{
			__disable_irq();
			
			WDT_Init(WDT, 0, 5);			// 通过触发WDT复位跳转到UserBoot
			WDT_Start(WDT);
			while(1) __NOP();
		}
 	}
}


void TIMR0_Handler(void)
{
	TIMR_INTClr(TIMR0);
	
	GPIO_InvBit(GPIOA, PIN1);
}


void SerialInit(void)
{
	UART_InitStructure UART_initStruct;
	
	PORT_Init(PORTM, PIN0, PORTM_PIN0_UART0_RX, 1);	//GPIOM.0配置为UART0输入引脚
	PORT_Init(PORTM, PIN1, PORTM_PIN1_UART0_TX, 0);	//GPIOM.1配置为UART0输出引脚
 	
 	UART_initStruct.Baudrate = 57600;
	UART_initStruct.DataBits = UART_DATA_8BIT;
	UART_initStruct.Parity = UART_PARITY_NONE;
	UART_initStruct.StopBits = UART_STOP_1BIT;
	UART_initStruct.RXThresholdIEn = 0;
	UART_initStruct.TXThresholdIEn = 0;
	UART_initStruct.TimeoutIEn = 0;
 	UART_Init(UART0, &UART_initStruct);
	UART_Open(UART0);
}

/****************************************************************************************************************************************** 
* 函数名称: fputc()
* 功能说明: printf()使用此函数完成实际的串口打印动作
* 输    入: int ch		要打印的字符
*			FILE *f		文件句柄
* 输    出: 无
* 注意事项: 无
******************************************************************************************************************************************/
int fputc(int ch, FILE *f)
{
	UART_WriteByte(UART0, ch);
	
	while(UART_IsTXBusy(UART0));
 	
	return ch;
}
