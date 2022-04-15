/*
 * management_movement.h
 *
 *  Created on: 15 avr. 2022
 *      Author: APrap
 */

#ifndef MANAGEMENT_MOVEMENT_H_
#define MANAGEMENT_MOVEMENT_H_

#define DISPLACEMENT_SPEED					320		//[step/s]
#define DISTANCE_TO_REACH_THE_MIDDLE		320		//[step]
#define SPEED_NUL							0
#define ANTI_WINDUP							5000
#define KP									1.1
#define KI									0.001
#define KD									10.105

#define VL53L0X_OPPENING					100		//distance considered for oppening [mm]

void management_movement_start(void);
uint32_t absolute_value_int32(int32_t value);
void check_for_oppenings(void);

#endif /* MANAGEMENT_MOVEMENT_H_ */
