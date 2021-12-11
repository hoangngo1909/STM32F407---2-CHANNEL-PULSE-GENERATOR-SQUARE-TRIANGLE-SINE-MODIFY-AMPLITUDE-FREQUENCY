
#include "stm32f4xx.h"
#include "system_timetick.h"
#include "stdlib.h"
#include "math.h"
#include "stdio.h"

#define BUFF_SIZE_RX 30
#define BUFF_SIZE_TX 25
#define pi 3.141592654

uint16_t value1[1000],value2[1000];
volatile uint16_t adc_value[2]; // luu tru gia tri doc ve tu ADC1 va ADC2
int8_t rxbuff[BUFF_SIZE_RX];
int8_t txbuff[BUFF_SIZE_TX];
char str_ADC1[100], str_ADC2[100], receive_data[100], tmp_string[100]; // Chuoi du lieu chua gia tri ADC doc ve tu hai kenh
float parameter[100]; // luu gia tri thong so A, F, Mode nhan ve tu may tinh da duoc tach ra 
float *pointer;

uint8_t MODE1 = 0, MODE2=0;
float A1=3, F1=0.1, A2=3, F2=0.1;

// Prototypes

void Check_Condition_DAC1(void);
void Check_Condition_DAC2(void);
void CONFIG_ADC_Channel1(void);
void CONFIG_ADC_Channel2(void);
void CONFIG_DAC_Channel1 (void);
void CONFIG_DAC_Channel2 (void);
void CONFIG_Timer7(void);
void CONFIG_Timer6(void);
void CONFIG_UART4 (void);
void CONFIG_Timer3(void);
void Transmit_Data_FromADC(void);

void Sine_Wave(uint16_t *value, float A);
void Square_Wave(uint16_t *value, float A);
void Triangular_Wave(uint16_t *value, float A);
void Linear_Wave(uint16_t *value, float A);
void Zero_Wave(uint16_t *value);

int main(void)
{
	/* Enable SysTick at 10ms interrupt */
	SysTick_Config(SystemCoreClock/100);
	
	CONFIG_ADC_Channel1();
	CONFIG_ADC_Channel2();
	CONFIG_DAC_Channel1();
	CONFIG_DAC_Channel2();
	CONFIG_Timer7();
	CONFIG_Timer6();
	CONFIG_Timer3();
	CONFIG_UART4 ();
	// Kich hoat 2 bo ADC: ADC1 va ADC2
	ADC_SoftwareStartConv(ADC1);
	ADC_SoftwareStartConv(ADC2);
	
	Sine_Wave(value1, A1);
	Square_Wave(value1, A1);
	Triangular_Wave(value1, A1);
	Linear_Wave(value1, A1);
	Zero_Wave(value1);

	Sine_Wave(value2, A2);
	Square_Wave(value2, A2);
	Triangular_Wave(value2, A2);
	Linear_Wave(value2, A2);
	Zero_Wave(value2);
	
	Check_Condition_DAC1();
	Check_Condition_DAC2();
	
	while(1){
	 
	}
}

// KIEM TRA DIEU KIEN BIEN DO, TAN SO, MODE KHI XUAT DAC1
void Check_Condition_DAC1(void)
{
	if (A1<0)	A1 = 0;
	if (A1>3)	A1 = 3;
	if (F1<0.01)	F1=0.01;
	if (F1>10)	F1=10;
	
	switch(MODE1)
	{
		case 0: // Sine
			Sine_Wave(value1, A1);
			break;
		case 1: // Square
			Square_Wave(value1, A1);
			break;
		case 2:
			Triangular_Wave(value1, A1);
			break;
		case 3:	//Linear
			Linear_Wave(value1, A1);	
			break;
		default:
			Zero_Wave(value1);
			break;
	}
	
	// Tinh toan x, y --> Prescale, Counter Period nap vao cho Timer Trigger
	uint32_t x=1, y=0;
	for (x=10; x<=20; x++)
	{	
		y =(uint32_t)1000/(x*F1);
		if((10<= y) && (y<=5000)) break;
	}
	
	TIM7->CR1 = 0;		// stop Timer7
	TIM7->PSC = x*84-1;		// clk = SystemCoreClock /2/(PSC+1) = 1MHz
	TIM7->ARR = y-1;
	TIM7->CNT = 0;
	TIM7->EGR = 1;		// update registers;
	TIM7->SR  = 0;		// clear overflow flag
	TIM7->CR1 = 1;		// enable Timer7
}

// KIEM TRA DIEU KIEN BIEN DO, TAN SO, MODE KHI XUAT DAC2
void Check_Condition_DAC2(void)
{
	if (A2<0)	A2 = 0;
	if (A2>3)	A2 = 3;
	if (F2<0.01)	F2=0.01;
	if (F2>10)	F2=10;
	
	switch(MODE2)
	{
		case 0: // Sine
			Sine_Wave(value2, A2);
			break;
		case 1: // Square
			Square_Wave(value2, A2);
			break;
		case 2:
			Triangular_Wave(value2, A2);
			break;
		case 3:	//Linear
			Linear_Wave(value2, A2);	
			break;
		default:
			Zero_Wave(value2);
			break;
	}
	uint32_t x=1,y=0;
	for (x=10; x<=20; x++)
	{	
		y =(uint32_t)1000/(x*F2);
		if((10<= y) && (y<=5000)) break;
	}
	
	TIM6->CR1 = 0;		// stop Timer6
	
	TIM6->PSC = x*84-1;		// clk = SystemCoreClock /2/(PSC+1) = 1MHz
	TIM6->ARR = y-1;
	TIM6->CNT = 0;
	TIM6->EGR = 1;		// update registers;
	TIM6->SR  = 0;		// clear overflow flag
	TIM6->CR1 = 1;		// enable Timer6
}

// CAU HINH DAC1 DMA (PIN: PA4 - TRIGGER TIMER 7 - DMA1 STREAM 5)
void CONFIG_DAC_Channel1 (void)
{
	GPIO_InitTypeDef GPIO_InitStructure; 
	DAC_InitTypeDef 	DAC_InitStructure;
	DMA_InitTypeDef 	DMA_InitStructure;
	
	/* DMA1 clock enable */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1, ENABLE);   
	/* GPIOA clock enable (to be used with DAC) */ 
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);                          
	/* DAC Periph clock enable */ 
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_DAC, ENABLE);
	
	// Cấu hình chân ngõ ra PA4 - DAC Channel 1
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;  
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	// Cấu hình trigger từ Timer: 
	DAC_InitStructure.DAC_Trigger = DAC_Trigger_T7_TRGO;
	DAC_InitStructure.DAC_WaveGeneration = DAC_WaveGeneration_None;
	DAC_InitStructure.DAC_OutputBuffer = DAC_OutputBuffer_Enable;
	DAC_Init(DAC_Channel_1, &DAC_InitStructure);

	// Cấu hình DMA cho DAC: DAC1 sử dụng DMA1 Stream 5 Channel 7
	DMA_DeInit(DMA1_Stream5);
	DMA_InitStructure.DMA_Channel = DMA_Channel_7;  
	//DMA_InitStructure.DMA_PeripheralBaseAddr = DAC_DHR12R1_ADDRESS; // địa chỉ thanh ghi chứa giá trị điện áp
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&(DAC->DHR12R1);
	DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)&value1;
	DMA_InitStructure.DMA_BufferSize = 1000; // 1000 mẫu dữ liệu
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
	DMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToPeripheral;
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular; // Mode Circular
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;
	DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;         
	DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_HalfFull;
	DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
	DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
	DMA_Init(DMA1_Stream5, &DMA_InitStructure);
  
// Cấu hình cho phép hoạt động chức năng DMA, DAC, DAC DMA
/* Enable DMA1_Stream5 */
  DMA_Cmd(DMA1_Stream5, ENABLE);
  /* Enable DAC Channel1 */
  DAC_Cmd(DAC_Channel_1, ENABLE);
  /* Enable DMA for DAC Channel1 */
  DAC_DMACmd(DAC_Channel_1, ENABLE);
}

// CAU HINH TIMER 7 - TRIGGER
void CONFIG_Timer7(void)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	// Cho phép xung clock Timer 7
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM7, ENABLE);
	
	/* Configure TIM7 (1ms) clock */
	TIM_TimeBaseStructure.TIM_Prescaler =84*5 - 1;
	TIM_TimeBaseStructure.TIM_Period = 1000 - 1;/*so xung trong 1 chu ky*/ 
	TIM_TimeBaseStructure.TIM_ClockDivision = 0; 
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM7, &TIM_TimeBaseStructure); 
	
	TIM_SelectOutputTrigger(TIM7,TIM_TRGOSource_Update);

	TIM_Cmd(TIM7, ENABLE);
}

// CAU HINH DAC2 DMA (PIN: PA5 - TRIGGER TIMER 6 - DMA1 STREAM 6)
void CONFIG_DAC_Channel2(void)
{
	GPIO_InitTypeDef 	GPIO_InitStructure;
	DAC_InitTypeDef 	DAC_InitStructure;
	DMA_InitTypeDef 	DMA_InitStructure;
	
	/* Enable the GPIOA peripheral */ 
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_DAC, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1,ENABLE);
	
  // Cấu hình chân ngõ ra PA5 - DAC Channel 2
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 ;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;  
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	// Cấu hình chức năng trigger từ Timer: chân PA5 dùng Timer 6
	DAC_InitStructure.DAC_Trigger = DAC_Trigger_T6_TRGO;
	DAC_InitStructure.DAC_WaveGeneration = DAC_WaveGeneration_None;
	DAC_InitStructure.DAC_OutputBuffer = DAC_OutputBuffer_Enable;
	DAC_Init(DAC_Channel_2, &DAC_InitStructure);
	
  // Cấu hình DMA cho DAC: DAC2 sử dụng DMA Stream 6 Channel 7
	DMA_DeInit(DMA1_Stream6);
	DMA_InitStructure.DMA_Channel = DMA_Channel_7;  
	//DMA_InitStructure.DMA_PeripheralBaseAddr = DAC_DHR12R2_ADDRESS;
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&(DAC->DHR12R2);
	DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)&value2;
	DMA_InitStructure.DMA_BufferSize = 1000;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
	DMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToPeripheral;
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;
	DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;         
	DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_HalfFull;
	DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
	DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
	DMA_Init(DMA1_Stream6, &DMA_InitStructure);    
	
 /* Enable DMA1_Stream5 */
  DMA_Cmd(DMA1_Stream6, ENABLE);
  
  /* Enable DAC Channel1 */
  DAC_Cmd(DAC_Channel_2, ENABLE);

  /* Enable DMA for DAC Channel1 */
  DAC_DMACmd(DAC_Channel_2, ENABLE);
}

// CAU HINH TIMER 6 _ TRIGGER
void CONFIG_Timer6(void)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	/* Enable the TIM7 clock */ 
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM6, ENABLE);
	
	/* Configure TIM7 (1ms) clock */
	TIM_TimeBaseStructure.TIM_Prescaler =84*5 - 1;
	TIM_TimeBaseStructure.TIM_Period = 1000 - 1;/*so xung trong 1 chu ky*/ 
	TIM_TimeBaseStructure.TIM_ClockDivision = 0; 
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM6, &TIM_TimeBaseStructure); 
	
	TIM_SelectOutputTrigger(TIM6,TIM_TRGOSource_Update);

	TIM_Cmd(TIM6, ENABLE);
}
 
// CAU HINH ADC_CHANNEL 1: PA6 - DMA2, CHANNEL 0, STREAM 0
void CONFIG_ADC_Channel1(void)
{
	ADC_InitTypeDef       ADC_InitStructure;
	ADC_CommonInitTypeDef ADC_CommonInitStructure;
	DMA_InitTypeDef       DMA_InitStructure;
	GPIO_InitTypeDef      GPIO_InitStructure;
	
	// Cấu hình xung clock cho ADC, GPIO, DMA
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE); 
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2, ENABLE);
	
	// Cấu hình chân PA6 ADC1
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;  
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	  /* ADC Common Init **********************************************************/  
	ADC_CommonInitStructure.ADC_Mode = ADC_Mode_Independent;
	ADC_CommonInitStructure.ADC_Prescaler = ADC_Prescaler_Div4;
	ADC_CommonInitStructure.ADC_DMAAccessMode = ADC_DMAAccessMode_Disabled;
	ADC_CommonInitStructure.ADC_TwoSamplingDelay = ADC_TwoSamplingDelay_5Cycles;
	ADC_CommonInit(&ADC_CommonInitStructure);
	
/* ADC1 Init ****************************************************************/   
	ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;  
	ADC_InitStructure.ADC_ScanConvMode = DISABLE;
	ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;   
	ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None; 
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T1_CC1;   
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right; 
	ADC_InitStructure.ADC_NbrOfConversion = 1;
	ADC_Init(ADC1, &ADC_InitStructure);
	
// Cau hinh DMA2, CHANNEL 0, STREAM 0
	DMA_InitStructure.DMA_Channel = DMA_Channel_0;  
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&(ADC1->DR);
	DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)&adc_value[0];
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;
	DMA_InitStructure.DMA_BufferSize = 1;
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Disable;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;
	DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;         
	DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_HalfFull;
	DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
	DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
	DMA_Init(DMA2_Stream0, &DMA_InitStructure);
	DMA_Cmd(DMA2_Stream0, ENABLE);
	
	ADC_RegularChannelConfig(ADC1,ADC_Channel_6,1,ADC_SampleTime_15Cycles);
	/* Start ADC1 */
	ADC_Cmd(ADC1,ENABLE);
	
	/* Enable DMA request after last transfer (Single-ADC mode) */
  ADC_DMARequestAfterLastTransferCmd(ADC1, ENABLE);
	/* Enable ADC1 DMA */
  ADC_DMACmd(ADC1, ENABLE);
}

// CAU HINH ADC_CHANNEL 2: PA7 - DMA2, CHANNEL 1, STREAM 2
void CONFIG_ADC_Channel2(void)
{
	GPIO_InitTypeDef 				GPIO_InitStructure;
	ADC_CommonInitTypeDef 	ADC_CommonInitStructure;
	ADC_InitTypeDef 				ADC_InitStructure;
	DMA_InitTypeDef       	DMA_InitStructure;
	/* Enable the GPIOA peripheral */ 
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
	/* Enable the ADC2 interface clock*/
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC2, ENABLE); 
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2, ENABLE);
	
  /* Cấu hình chân GPIO PA7 ADC2 */
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;  
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	/* Configure ADC_CommonInit */
	ADC_CommonInitStructure.ADC_DMAAccessMode = ADC_DMAAccessMode_Disabled;
	ADC_CommonInitStructure.ADC_Mode = ADC_Mode_Independent;
	ADC_CommonInitStructure.ADC_Prescaler = ADC_Prescaler_Div4;
	ADC_CommonInitStructure.ADC_TwoSamplingDelay = ADC_TwoSamplingDelay_5Cycles;
	ADC_CommonInit(&ADC_CommonInitStructure);
	
	/* Configure ADC2 */
	ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T1_CC1;
	ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None;
	ADC_InitStructure.ADC_NbrOfConversion = 1;
	ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;
	ADC_InitStructure.ADC_ScanConvMode = DISABLE;
	ADC_Init(ADC2,&ADC_InitStructure);
	
	/* DMA2 Stream2 channel1 configuration **************************************/
	DMA_InitStructure.DMA_Channel = DMA_Channel_1;  
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&(ADC2->DR);
	DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)&adc_value[1];
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;
	DMA_InitStructure.DMA_BufferSize = 1;
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Disable;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;
	DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;         
	DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_HalfFull;
	DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
	DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
	DMA_Init(DMA2_Stream2, &DMA_InitStructure);
	DMA_Cmd(DMA2_Stream2, ENABLE);
	
	ADC_RegularChannelConfig(ADC2,ADC_Channel_7,1,ADC_SampleTime_15Cycles);
	/* Start ADC1 */
	ADC_Cmd(ADC2,ENABLE);
	/* Enable DMA request after last transfer (Single-ADC mode) */
  ADC_DMARequestAfterLastTransferCmd(ADC2, ENABLE);
	/* Enable ADC1 DMA */
  ADC_DMACmd(ADC2, ENABLE);
}

// CAU HINH UART
void CONFIG_UART4 (void)
{
	GPIO_InitTypeDef 	GPIO_InitStructure; 
	USART_InitTypeDef USART_InitStructure;  
	DMA_InitTypeDef   DMA_InitStructure;
	NVIC_InitTypeDef  NVIC_InitStructure;	
	
	  /* Enable GPIO clock */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
  /* Enable UART clock */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART4, ENABLE);
	/* Enable DMA1 clock */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1, ENABLE);
	
	  /* Connect UART4 pins to AF2 */  
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource0, GPIO_AF_UART4);
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource1, GPIO_AF_UART4);
	
	  /* GPIO Configuration for UART4 Tx */
	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_0;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

  /* GPIO Configuration for USART Rx */
	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_1;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	USART_InitStructure.USART_BaudRate = 115200;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(UART4, &USART_InitStructure);

	  /* Enable USART */
	USART_Cmd(UART4, ENABLE);
	
	/* Enable UART4 DMA  Rx & Tx */
	USART_DMACmd(UART4, USART_DMAReq_Rx, ENABLE);
	USART_DMACmd(UART4,USART_DMAReq_Tx,ENABLE);
	
	// Cau hinh DMA UART4 Rx: DMA1, Channel4, Stream 2
	DMA_InitStructure.DMA_Channel = DMA_Channel_4;  
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&UART4->DR;
	DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)rxbuff;
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;
	DMA_InitStructure.DMA_BufferSize = BUFF_SIZE_RX;
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;//DMA_Mode_Circular;
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;
	DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;         
	DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_HalfFull;
	DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
	DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
	DMA_Init(DMA1_Stream2, &DMA_InitStructure);
	DMA_Cmd(DMA1_Stream2, ENABLE);

	DMA_DeInit(DMA1_Stream4);
	// Cau hinh DMA UART4 Tx: DMA1, Channel4, Stream 4	
	DMA_InitStructure.DMA_Channel = DMA_Channel_4;  
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&(UART4->DR);
	DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)txbuff;
	DMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToPeripheral;
	DMA_InitStructure.DMA_BufferSize = BUFF_SIZE_TX;
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;
	DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;         
	DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_HalfFull;
	DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
	DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
	DMA_Init(DMA1_Stream4, &DMA_InitStructure);
	DMA_Cmd(DMA1_Stream4, ENABLE);
	
	/* Enable DMA Interrupt to the highest priority */
	NVIC_InitStructure.NVIC_IRQChannel = DMA1_Stream2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	// Bật cờ DMA_IT_TC để cho phép ngắt DMA UART RX ở kênh IRQn
	DMA_ITConfig(DMA1_Stream2, DMA_IT_TC, ENABLE);
}


void Sine_Wave(uint16_t *value, float A)
{
	for(int i=0;i<1000;i++)
		value[i] = (A*sin(2*pi*i/1000)+A)*4095/3/2;
}
void Square_Wave(uint16_t *value, float A)
{
	for(int i=0;i<1000;i++)
		if(i<500) value[i] = 0;
		else value[i] = A*4095/3;
}
void Triangular_Wave(uint16_t *value, float A)
{
	for(int i=0;i<1000;i++)
		if(i<500) value[i] = (A*i/500)*4095/3;
		else value[i] = (2*A-A*i/500)*4095/3;
}
void Linear_Wave(uint16_t *value, float A)
{
	for(int i=0;i<1000;i++)
	 value[i] = (A*i/1000)*4095/3;
}
void Zero_Wave(uint16_t *value)
{
	for(int i=0;i<1000;i++)
		value[i] = 0;
}


void CONFIG_Timer3(void)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	NVIC_InitTypeDef NVIC_InitStructure; // Ngat Timer
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
	
	TIM_TimeBaseStructure.TIM_Prescaler =8400 - 1;
	TIM_TimeBaseStructure.TIM_Period = 400  - 1;/*so xung trong 1 chu ky*/      //100  10ms   100 mau/s
	TIM_TimeBaseStructure.TIM_ClockDivision = 0; 
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure); 
	TIM_ITConfig(TIM3,TIM_IT_Update,ENABLE);
	TIM_Cmd(TIM3, ENABLE);
	
	NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn; 
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0; 
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; 
	NVIC_Init(&NVIC_InitStructure);

	TIM_Cmd(TIM3, ENABLE);
}

void Transmit_Data_FromADC(void)
{
	// Gán giá trị ADC về 
	int16_t ReceiveADC1 = adc_value[0];
	int16_t ReceiveADC2 = adc_value[1];
	int8_t indexADC1, indexADC2;
	// Kiểm tra độ lớn của giá trị ADC để cấp bộ nhớ cho phù hợp
	if (ReceiveADC1<10) // Gia tri ADC1 doc ve nho hon 10
		indexADC1 =1; // de bit 0 chua ADC1=a, bit 1 chua "/"
	else if (ReceiveADC1<100)
		indexADC1 = 2; // de bit index=0 1 chua gia tri ADC1=a b, index=2 chua "/"
	else if (ReceiveADC1<1000)
		indexADC1 = 3; // de bit index = 0 1 2 chua gia tri ADC1 = a b c, index = 3 chua "/"
	else
		indexADC1 = 4; // de bit index = 0 1 2 3 chua gia tri ADC1 = a b c d, index = 4 chua "/"
	
	if (ReceiveADC2<10) 
		indexADC2 =1; 
	else if (ReceiveADC2<100)
		indexADC2 = 2; 
	else if (ReceiveADC2<1000)
		indexADC2 = 3; 
	else
		indexADC2 = 4; 
	
	// Gan gia tri ADC doc ve thanh 1 chuoi su dung ham snprintf()
	snprintf(str_ADC1, 10, "%i", ReceiveADC1);
	str_ADC1[indexADC1]='|';
	snprintf(str_ADC2, 10, "%i", ReceiveADC2);
	str_ADC2[indexADC2]='|';
	
	// Dong goi du lieu vao txbuff
	txbuff[0] = 's';
	txbuff[1] = 't';
	txbuff[2] = 'a';
	txbuff[3] = 'r';
	txbuff[4] = 't';
	txbuff[5] = '|';
	for(int j=6; j<= indexADC1 + 6; j++)	txbuff[j] = str_ADC1[j-6];
	for(int j=indexADC1 + 7; j<= indexADC1 + 7 + indexADC2 ; j++)	txbuff[j] = str_ADC2[j-indexADC1-7];
	txbuff[indexADC1 + 7 + indexADC2 + 1] = 'e';
	txbuff[indexADC1 + 7 + indexADC2 + 2] = 'n';
	txbuff[indexADC1 + 7 + indexADC2 + 3] = 'd';
	txbuff[indexADC1 + 7 + indexADC2 + 4] = '|';
	
	DMA_ClearFlag(DMA1_Stream4, DMA_FLAG_TCIF4);
	DMA1_Stream4->NDTR = BUFF_SIZE_TX;
	DMA_Cmd(DMA1_Stream4, ENABLE);
}

// HAM NGAT UART RX NHAN VE GIA TRI TAN SO, BIEN DO
void DMA1_Stream2_IRQHandler(void)
{
	/* Clear the DMA1_Stream2 TCIF2 pending bit */
	DMA_ClearITPendingBit(DMA1_Stream2, DMA_IT_TCIF2);
	for(uint8_t i=0; i<BUFF_SIZE_RX; i++)	receive_data[i]=rxbuff[i];
	
	// Sau khi đọc về, tiến hành tách chuỗi dữ liệu
	uint8_t m = 0; 
	uint8_t n=0, p=0, q=0;
	for (m = 0; m<6; m++) // Tổng cộng có 6 phần tử cần đọc về là A1, F1, Mode1, A2, F2, Mode2
	{
		for(n=0; n<BUFF_SIZE_RX; n++)
		{
			if (receive_data[n] != 124) // khac dau | (ASCII cua | = 124)
				tmp_string[n] = receive_data[n];
			else
			{
				tmp_string[n]='\0'; // ky tu \0 ket thuc chuoi
				pointer = &parameter[m];
				*pointer = atof(tmp_string); // Chuyen doi chuoi thanh gia tri so thuc double
				break;
			}
		}
		// Tien hanh cat bo thong so vua duoc lay o tren trong mang receive_data
		p = 0;
			for (q = n+1; q<BUFF_SIZE_RX; q ++)
			{
				receive_data[p] = receive_data[q];
				if (p<BUFF_SIZE_RX) p++;
				else
				{
					p = 0;
					break;
				}
			}
	}
	
	// Gán các giá trị biên độ, tần số và mode khi đã có được chuỗi parameter
	A1 = parameter[0];
	F1 = parameter[1];
	MODE1 = parameter[2];
	A2 = parameter[3];
	F2 = parameter[4];
	MODE2 = parameter[5];
	// Xet dieu kien cua Bien do va Tan so
	Check_Condition_DAC1();
	Check_Condition_DAC2();
	DMA_Cmd(DMA1_Stream2, ENABLE);
}

// KHI TIMER 3 TRAN SE NHAY VAO NGAT THUC HIEN TRUYEN DU LIEU
void TIM3_IRQHandler(void)
{
	
	if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET)
	{
		Transmit_Data_FromADC();
		TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
	}
}