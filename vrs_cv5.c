/*
 * vrs_cv5.c
 *
 *  Created on: Oct 19, 2016
 *      Author: Jany
 */
#include <stddef.h>
#include <vrs_cv5a.h>
#include "stm32l1xx.h"

// zmena frekvencie blikania pomocou AD prevodnika
void blikaj(uint16_t prevod){
	// zmena stavu LED na pine 5
	GPIO_ToggleBits(GPIOA, GPIO_Pin_5);
	for(int i = 0; i < 100*prevod; i++);

}


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

// inicializacia pre NVIC
void NVIC_init(void)
{
	// inicializacna struktura pre NVIC
	NVIC_InitTypeDef NVIC_InitStructure;

	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
	NVIC_InitStructure.NVIC_IRQChannel = ADC1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}

// funkcia na inicializaciu ADC prevodnika
void ADC_init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	ADC_InitTypeDef ADC_InitStructure;
	/* Enable GPIO clock */
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 ;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL ;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	/* Enable the HSI oscillator */
	RCC_HSICmd(ENABLE);
	/* Check that HSI oscillator is ready */
	while(RCC_GetFlagStatus(RCC_FLAG_HSIRDY) == RESET);
	/* Enable ADC clock */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
	/* Initialize ADC structure */
	ADC_StructInit(&ADC_InitStructure);
	/* ADC1 configuration */
	ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;
	ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;
	ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None;
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
	ADC_InitStructure.ADC_NbrOfConversion = 1;
	ADC_Init(ADC1, &ADC_InitStructure);
	/* ADCx regular channel8 configuration */
	ADC_RegularChannelConfig(ADC1, ADC_Channel_0, 1, ADC_SampleTime_16Cycles);
	/* Enable the ADC */


	//povolili sme prerusenie po skonceni konverzie
	ADC_ITConfig(ADC1, ADC_IT_EOC, ENABLE);
	ADC_ITConfig(ADC1, ADC_IT_OVR, ENABLE);
	ADC_Cmd(ADC1, ENABLE);

	/* Wait until the ADC1 is ready */
	while(ADC_GetFlagStatus(ADC1, ADC_FLAG_ADONS) == RESET)
		{
		}
		/* Start ADC Software Conversion */

	ADC_SoftwareStartConv(ADC1);
}


// funkcia prerusenia z ADC prevodnika
void ADC1_IRQHandler(void){

	//if(!ADC_GetITStatus(ADC1, ADC_IT_EOC) != RESET){
		// nacitame si hodnotu z prevodnika
		blikaj(ADC_GetConversionValue(ADC1));
		ADC_ClearITPendingBit(ADC1, ADC_IT_EOC);
	//}

}





