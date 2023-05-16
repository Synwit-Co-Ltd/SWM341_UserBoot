#include "SWM341.h"


void SerialInit(void);
void JumpToApp(uint32_t addr);

int main(void)
{
 	SystemInit();
	
	SerialInit();
	
	GPIO_Init(GPIOA, PIN0, 0, 1, 0, 0);		// 输入，开上拉
	GPIO_Init(GPIOA, PIN1, 1, 0, 0, 0);		// 输出
	
	if(GPIO_GetBit(GPIOA, PIN0) == 0)		// 检测到特定信号（或者检测到APP程序不完整）
	{
		for(int i = 0; i < 10; i++)			// 模拟UserBoot更新APP程序的过程
		{
			GPIO_InvBit(GPIOA, PIN1);
			
			printf("Hi from UserBoot\n");
			
			for(int j = 0; j < SystemCoreClock/50; j++) __NOP();
		}
	}
	
	/* 跳转到APP前需要将UserBoot使用的外设关掉（复位）*/
	__disable_irq();
	
	SYS->PRSTEN = 0x55;
	SYS->PRSTR0 = 0xFFFFFFFF & (~SYS_PRSTR0_ANAC_Msk) & (~SYS_PRSTR0_GPIOA_Msk);
	SYS->PRSTR1 = 0xFFFFFFFF;
	for(int i = 0; i < CyclesPerUs; i++) __NOP();
	SYS->PRSTR0 = 0;
	SYS->PRSTR1 = 0;
	SYS->PRSTEN = 0;
	
	SysTick->CTRL = 0;	//关闭SysTick
	
	NVIC->ICER[0] = 0xFFFFFFFF;
	NVIC->ICER[1] = 0xFFFFFFFF;
	NVIC->ICER[2] = 0xFFFFFFFF;
	
	JumpToApp(0x20000);
	
 	while(1==1)
 	{
 	}
}


/* 注意：不能使用 O0 优化等级编译 JumpToApp() 函数
 * 因为 O0 优化等级下，编译器 AC6 生成的指令会先将 pc 的值压入栈中，在执行 ResetHandler() 时
 * 弹出使用，但在此之前 __set_MSP(sp) 已经修改了栈指针，导致跳转目的地址错误
*/
void JumpToApp(uint32_t addr)
{
	uint32_t sp = *((volatile uint32_t *)(addr));
	uint32_t pc = *((volatile uint32_t *)(addr + 4));
	
	typedef void (*Func_void_void)(void);
	Func_void_void ResetHandler = (Func_void_void)pc;
	
	SCB->VTOR = addr;
	
	__set_MSP(sp);
	
	ResetHandler();
	
	while(1) __NOP();
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
