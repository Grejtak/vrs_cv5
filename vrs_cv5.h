/*
 * vrs_cv5.h
 *
 *  Created on: Oct 19, 2016
 *      Author: Jany
 */

#ifndef VRS_CV5A_H_
#define VRS_CV5A_H_

// zmena frekvencie blikania pomocou AD prevodnika
void blikaj(uint16_t prevod);
// funkcia na inicializaciu LED
void LED_init(void);
// inicializacia pre NVIC
void NVIC_init(void);
// funkcia na inicializaciu ADC prevodnika
void ADC_init(void);
// funkcia prerusenia z ADC prevodnika
void ADC1_IRQHandler(void);


#endif /* VRS_CV5A_H_ */

