#ifndef VRS_CV5_H_
#define VRS_CV5_H_

// funkcia na inicializaciu LED
void LED_init(void);
// zmena frekvencie blikania pomocou AD prevodnika
void blikaj(uint16_t prevod);
// inicializacia pre NVIC
void NVIC_init(void);
// funkcia na inicializaciu ADC prevodnika
void ADC_init(void);
// funkcia na inicializaciu USART
void USART_init(void);
// posli string po seriovej zbernici USART2
void SendUSART2(char *s);

#endif /* VRS_CV5_H_ */

