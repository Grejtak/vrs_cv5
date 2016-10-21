#include <stddef.h>
#include "stm32l1xx.h"
#include "vrs_cv5.h"

// funkcia na inicializaciu LED
void LED_init(void)
{	// inicializacna struktura pre periferiu GPIOA
	GPIO_InitTypeDef gpioaInitStr;

	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA,ENABLE);
	gpioaInitStr.GPIO_Pin = GPIO_Pin_5;
	gpioaInitStr.GPIO_Mode = GPIO_Mode_OUT;
	gpioaInitStr.GPIO_OType = GPIO_OType_PP;
	gpioaInitStr.GPIO_PuPd = GPIO_PuPd_UP;
	gpioaInitStr.GPIO_Speed = GPIO_Speed_40MHz;
	GPIO_Init(GPIOA,&gpioaInitStr);
}

// zmena frekvencie blikania pomocou AD prevodnika
void blikaj(uint16_t prevod){
	// zmena stavu LED na pine 5
	GPIO_ToggleBits(GPIOA, GPIO_Pin_5);
	for(int i = 0; i < 100*prevod; i++);
}

// inicializacia pre USART
void USART_init(void){
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA,ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);

	// nakonfigurujeme piny pre USART
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource2, GPIO_AF_USART2);
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource3, GPIO_AF_USART2);

	// inicializacna struktura pre RX
	GPIO_InitTypeDef RXInitStr;

	RXInitStr.GPIO_Pin = GPIO_Pin_3;
	RXInitStr.GPIO_Mode = GPIO_Mode_AF;
	RXInitStr.GPIO_OType = GPIO_OType_PP;
	RXInitStr.GPIO_PuPd = GPIO_PuPd_NOPULL;
	RXInitStr.GPIO_Speed = GPIO_Speed_40MHz;
	GPIO_Init(GPIOA,&RXInitStr);

	// inicializacna struktura pre TX
	GPIO_InitTypeDef TXInitStr;

	TXInitStr.GPIO_Pin = GPIO_Pin_2;
	TXInitStr.GPIO_Mode = GPIO_Mode_AF;
	TXInitStr.GPIO_OType = GPIO_OType_PP;
	TXInitStr.GPIO_PuPd = GPIO_PuPd_NOPULL;
	TXInitStr.GPIO_Speed = GPIO_Speed_40MHz;
	GPIO_Init(GPIOA,&TXInitStr);

	// inicializacna struktura pre USART
	USART_InitTypeDef USART_InitStructure;
	// konfiguracia USART
	USART_InitStructure.USART_BaudRate = 2*9600;	// 2x kvoli kniznici
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(USART2, &USART_InitStructure);
	USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);
	USART_Cmd(USART2, ENABLE);
}


// inicializacia pre NVIC
void NVIC_init(void){
	// inicializacna struktura pre NVIC
	NVIC_InitTypeDef NVIC_InitStructure;
	// ADC ma vyssiu prioritu ako USART2
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
	NVIC_InitStructure.NVIC_IRQChannel = ADC1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
	NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}

// funkcia na inicializaciu ADC prevodnika
void ADC_init(void){
	GPIO_InitTypeDef GPIO_InitStructure;
	ADC_InitTypeDef ADC_InitStructure;
	// povolime hodine pre periferiu GPIOA
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 ;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL ;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	// zapneme HSI oscilator
	RCC_HSICmd(ENABLE);
	// skontrolujeme HSI oscilator
	while(RCC_GetFlagStatus(RCC_FLAG_HSIRDY) == RESET);
	// povolime hodiny pre preriferiu ADC1
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
	// inicializacna struktura
	ADC_StructInit(&ADC_InitStructure);
	// konfiguracia
	ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;
	ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;
	ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None;
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
	ADC_InitStructure.ADC_NbrOfConversion = 1;
	ADC_Init(ADC1, &ADC_InitStructure);
	ADC_RegularChannelConfig(ADC1, ADC_Channel_0, 1, ADC_SampleTime_16Cycles);
	//povolili sme prerusenie po skonceni konverzie
	ADC_ITConfig(ADC1, ADC_IT_EOC, ENABLE);
	ADC_ITConfig(ADC1, ADC_IT_OVR, ENABLE);
	ADC_Cmd(ADC1, ENABLE);
	// pockaj kym je AD prevodnik pripaveny
	while(ADC_GetFlagStatus(ADC1, ADC_FLAG_ADONS) == RESET){}
	// zacni prevod
	ADC_SoftwareStartConv(ADC1);
}

// posli string po seriovej zbernici USART2
void SendUSART2(char *s){
  // kym sme neposlali vsetky znaky
  while(*s){
	// pockaj kym mozes vysielat
    while(USART_GetFlagStatus(USART2, USART_FLAG_TXE) == RESET);
    // posli znak na zbernicu a posun smernik na dalsi znak
    USART_SendData(USART2, *s++);
  }
}
