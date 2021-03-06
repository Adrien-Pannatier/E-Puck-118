/*
 * light_gestion.h
 *
 *  Created on: 12 avr. 2022
 *     Authors: Axel Praplan, Adrien Pannatier
 *
 *  Thread declaration and defines for LED responses depending on
 *  the state of the robot
 */

#ifndef LIGHT_GESTION_H_
#define LIGHT_GESTION_H_

#define ALAMRM_SPEED	70

//RGB states
#define LED_ON		1
#define LED_OFF		0
#define RGB_RED		100, 0, 0
#define RGB_BLUE	0, 0, 100
#define RGB_GREEN	0, 100, 0
#define RGB_YELLOW	100, 100, 0
#define RGB_WHITE	100, 100, 100
#define RGB_OFF		0, 0, 0

/**
 * @brief Activates the LED thread
 *
 */
void LED_start(void);

#endif /* LIGHT_GESTION_H_ */
