/*
 * LED.h
 *
 *  Created on: 12 avr. 2022
 *      Author: APrap
 */

#ifndef LIGHT_GESTION_H_
#define LIGHT_GESTION_H_

#define ALAMRM_SPEED	70

#define LED_ON		1
#define LED_OFF		0
#define RGB_RED		100, 0, 0
#define RGB_BLUE	0, 0, 100
#define RGB_GREEN	0, 100, 0
#define RGB_YELLOW	100, 100, 0
#define RGB_WHITE	100, 100, 100
#define RGB_OFF		0, 0, 0


void LED_start(void);

#endif /* LIGHT_GESTION_H_ */
