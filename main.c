/*
******************************************************************************
File:     main.c
Info:     Generated by Atollic TrueSTUDIO(R) 6.0.0   2016-10-11
The MIT License (MIT)
Copyright (c) 2009-2016 Atollic AB
Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
******************************************************************************
*/

#include <stddef.h>
#include <stdio.h>
#include <math.h>
#include "stm32l1xx.h"
#include "vrs_cv5.h"

// premenna na ulozenie hodnoty AD prevodu
volatile int ADC1_prevod = 0;
// vlajka na prepinanie medzi formatom vypisu
volatile unsigned char Format_FLAG = 0;
// vlajka na pociatocne odoslanie znaku cez USART
volatile unsigned char Zaciatok_FLAG = 1;

// odosielaci kruhovy buffer
volatile char USART_KRUH_BUFFER[USART_KRUH_BUFFER_SIZE];
//smernik na zaciatocnu a koncovu poziciu
volatile uint8_t USART_ZACIATOK = 0;
volatile uint8_t USART_KONIEC = 0;

int main(void) {

	  /**
	  *  IMPORTANT NOTE!
	  *  See the <system_*.c> file and how/if the SystemInit() function updates
	  *  SCB->VTOR register. Sometimes the symbol VECT_TAB_SRAM needs to be defined
	  *  when building the project if code has been located to RAM and interrupts
	  *  are used. Otherwise the interrupt table located in flash will be used.
	  *  E.g.  SCB->VTOR = 0x20000000;
	  */

	  /**
	  *  At this stage the microcontroller clock setting is already configured,
	  *  this is done through SystemInit() function which is called from startup
	  *  file (startup_stm32l1xx_hd.s) before to branch to application main.
	  *  To reconfigure the default setting of SystemInit() function, refer to
	  *  system_stm32l1xx.c file
	  */

	// zavolame inicializacne funkcie
	LED_init();
	NVIC_init();
	USART_init();
	ADC_init();

	// premenna na uchovanie prevedeneho cisla
	char strADCNum[5];	// max 4095 + '\0'
	char strADCVol[5];	// X.XX + '\0'
	char pom_buff[8];

	// hlavna slucka programu
	while (1) {
		// blikaj LED frekvenciou na zaklade hodnoty z AD prevodnika
		blikaj(ADC1_prevod);
		// podla vlajky menime format vypisu
		if(Format_FLAG){
			// nutny prepocet na vhodny format pre odoslanie
			float napatie = 3.3*(ADC1_prevod/4095.0);
			int Ccast = napatie;
			float zvysok = napatie - Ccast;
			int Dcast = trunc(zvysok*100);	// pre dve desatinne cisla
			// konverzia napatia na string
			sprintf(strADCVol, "%d.%d", Ccast, Dcast);
			//skopirujeme do pomocneho bufferu
			sprintf(pom_buff, strADCVol);
			// spojime stringy
			strcat(pom_buff, "V\n\r");
			// nastavime konecny index
			USART_KONIEC = 7;
			// odosli po seriovej linke
			sprintf(USART_KRUH_BUFFER, pom_buff);
			if (Zaciatok_FLAG){
				USART_ITConfig(USART2, USART_IT_TXE, ENABLE);
				USART_SendData(USART2, USART_KRUH_BUFFER[USART_ZACIATOK]);
				Zaciatok_FLAG = 0;
			}
		}
		else{
			// konverzia cisla na string
			sprintf(strADCNum, "%d", ADC1_prevod);
			//skopirujeme do pomocneho bufferu
			sprintf(pom_buff, strADCNum);
			// spojime stringy
			strcat(pom_buff, "\n\r");
			// nastavime konecny index
			USART_KONIEC = 6;
			sprintf(USART_KRUH_BUFFER, pom_buff);
			if (Zaciatok_FLAG){
				USART_ITConfig(USART2, USART_IT_TXE, ENABLE);
				// odosli po seriovej linke
				USART_SendData(USART2, USART_KRUH_BUFFER[USART_ZACIATOK]);
				Zaciatok_FLAG = 0;
			}
		}
	}
	return 0;
}

// funkcia prerusenia z ADC prevodnika
void ADC1_IRQHandler(void) {
	if (ADC1->SR & ADC_SR_EOC) {
		ADC1_prevod = ADC1->DR;
	}
}

// funkcia prerusenia USART2
void USART2_IRQHandler(void) {
	uint8_t pom = 0;

	// ak ak sme prijali data
	if (USART_GetITStatus(USART2, USART_IT_RXNE) != RESET) {
		// vynulujeme vlajku prerusenia
		USART_ClearITPendingBit(USART2, USART_IT_RXNE);
		// prijate data
		pom = USART_ReceiveData(USART2);
		// ak je prijaty znak 'm'
		if(pom == 'm'){
			// zmenime vlajku formatu vypisu na seriovej linke
			if(Format_FLAG==1){
				Format_FLAG = 0;
			}
			else{
				Format_FLAG = 1;
			}
		}

	}

	// ak sme odoslali data
	if (USART_GetITStatus(USART2, USART_IT_TXE) != RESET) {
		// posunieme smernik
		USART_ZACIATOK++;
		if (USART_ZACIATOK == USART_KONIEC+1){
			USART_ZACIATOK = 0;
		}
		// posleme dalsi znak
		// vynulujeme vlajku prerusenia
		USART_ClearITPendingBit(USART2, USART_IT_TXE);
		USART_SendData(USART2, USART_KRUH_BUFFER[USART_ZACIATOK]);
	}

}

#ifdef  USE_FULL_ASSERT

/**
 * @brief  Reports the name of the source file and the source line number
 *   where the assert_param error has occurred.
 * @param  file: pointer to the source file name
 * @param  line: assert_param error line source number
 * @retval None
 */
void assert_failed(uint8_t* file, uint32_t line)
{
	/* User can add his own implementation to report the file name and line number,
	 ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

	/* Infinite loop */
	while (1)
	{
	}
}
#endif

/*
 * Minimal __assert_func used by the assert() macro
 * */
void __assert_func(const char *file, int line, const char *func,
		const char *failedexpr) {
	while (1) {
	}
}

/*
 * Minimal __assert() uses __assert__func()
 * */
void __assert(const char *file, int line, const char *failedexpr) {
	__assert_func(file, line, NULL, failedexpr);
}
