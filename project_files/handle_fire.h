/*
 * handle_fire.h
 *
 *  Created on: 21 avr. 2022
 *      Author: AdrienPannatier
 *
 * Functions to control the alarm and the air blower module
 */

#ifndef HANDLE_FIRE_H_
#define HANDLE_FIRE_H_

 /**
 * @brief   activates the camera to check if a fire is present
 *
 * @return  True if fire / false if no fire
 */
_Bool check_for_fire(void);

/**
 * @brief Activate the following events:
 * 										- Start the air blower
 * 										- Activate the siren
 * 										- wiggle to better reach the flame
 */
void deploy_antifire_measures(void);

/**
 * @brief Stops the following events:
 * 										- Stop the air blower
 * 										- deactivate the siren
 * 										- stop moving
 */
void stop_antifire_measures(void);

#endif /* ADDED_MELODIES_H_ */
