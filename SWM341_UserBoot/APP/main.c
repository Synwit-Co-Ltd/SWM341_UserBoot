#include "SWM341.h"


void SerialInit(void);
void JumpToApp(uint32_t addr);

int main(void)
{
 	SystemInit();
	
	SerialInit();
	
	GPIO_Init(GPIOA, PIN0, 0, 1, 0, 0);		// ���룬������
	GPIO_Init(GPIOA, PIN1, 1, 0, 0, 0);		// ���
	
	if(GPIO_GetBit(GPIOA, PIN0) == 0)		// ��⵽�ض��źţ����߼�⵽APP����������
	{
		for(int i = 0; i < 10; i++)			// ģ��UserBoot����APP����Ĺ���
		{
			GPIO_InvBit(GPIOA, PIN1);
			
			printf("Hi from UserBoot\n");
			
			for(int j = 0; j < SystemCoreClock/50; j++) __NOP();
		}
	}
	
	/* ��ת��APPǰ��Ҫ��UserBootʹ�õ�����ص�����λ��*/
	__disable_irq();
	
	SYS->PRSTEN = 0x55;
	SYS->PRSTR0 = 0xFFFFFFFF & (~SYS_PRSTR0_ANAC_Msk) & (~SYS_PRSTR0_GPIOA_Msk);
	SYS->PRSTR1 = 0xFFFFFFFF;
	for(int i = 0; i < CyclesPerUs; i++) __NOP();
	SYS->PRSTR0 = 0;
	SYS->PRSTR1 = 0;
	SYS->PRSTEN = 0;
	
	SysTick->CTRL = 0;	//�ر�SysTick
	
	NVIC->ICER[0] = 0xFFFFFFFF;
	NVIC->ICER[1] = 0xFFFFFFFF;
	NVIC->ICER[2] = 0xFFFFFFFF;
	
	JumpToApp(0x20000);
	
 	while(1==1)
 	{
 	}
}


/* ע�⣺����ʹ�� O0 �Ż��ȼ����� JumpToApp() ����
 * ��Ϊ O0 �Ż��ȼ��£������� AC6 ���ɵ�ָ����Ƚ� pc ��ֵѹ��ջ�У���ִ�� ResetHandler() ʱ
 * ����ʹ�ã����ڴ�֮ǰ __set_MSP(sp) �Ѿ��޸���ջָ�룬������תĿ�ĵ�ַ����
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
	
	PORT_Init(PORTM, PIN0, PORTM_PIN0_UART0_RX, 1);	//GPIOM.0����ΪUART0��������
	PORT_Init(PORTM, PIN1, PORTM_PIN1_UART0_TX, 0);	//GPIOM.1����ΪUART0�������
 	
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
* ��������: fputc()
* ����˵��: printf()ʹ�ô˺������ʵ�ʵĴ��ڴ�ӡ����
* ��    ��: int ch		Ҫ��ӡ���ַ�
*			FILE *f		�ļ����
* ��    ��: ��
* ע������: ��
******************************************************************************************************************************************/
int fputc(int ch, FILE *f)
{
	UART_WriteByte(UART0, ch);
	
	while(UART_IsTXBusy(UART0));
 	
	return ch;
}
