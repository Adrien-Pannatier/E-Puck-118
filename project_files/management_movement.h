/*
 * management_movement.h
 *
 *  Created on: 15 avr. 2022
 *      Author: APrap
 */

#ifndef MANAGEMENT_MOVEMENT_H_
#define MANAGEMENT_MOVEMENT_H_

//Moving state
#define STOP								0
#define MOVING								1
#define REACHING_INTERSECTION				2
#define ROTATING							3
#define LEAVING_INTERSECTION				4
#define FIRE_FIGHTING						5

//Moving parameters
#define DISPLACEMENT_SPEED					320		//[step/s]
#define DISTANCE_TO_REACH_THE_MIDDLE		320		//[step] //a changer en fonction de la largeur du labyrinthe
#define SPEED_NUL							0

//PD parameters
#define ANTI_WINDUP							5000
#define KP									1.1
#define KD									10.105


//a changer
#define VL53L0X_OPPENING					100		//distance considered for oppening [mm]

void management_movement_start(void);
bool get_fire_detected(void);

#endif /* MANAGEMENT_MOVEMENT_H_ */
