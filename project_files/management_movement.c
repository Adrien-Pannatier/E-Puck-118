/*
 * management_movement.c
 *
 *  Created on: 15 avr. 2022
 *      Author: Axel Praplan, Adrien Pannatier
 *
 *  Functions and defines to manage the state of the robot and the
 *  way it moves and interacts with its surroundings
 */

#include "ch.h"
#include "hal.h"
#include "management_movement.h"
#include "motors.h"
#include "management_proximity.h"
#include "management_transmissions.h"
#include "handle_fire.h"

//provisoir pour debug
#include "leds.h"
#include "sensors/proximity.h"
#include "sensors/VL53L0X/VL53L0X.h"
#include "chprintf.h"
#include "button.h"
#include "selector.h"


//Moving parameters
#define CERTAINTY						3												//minimum occurrence of a measure to have a good confidence
#define NULL_POS						0												//Position zero for motor counter
#define R_EPUCK							3.5												//[cm]
#define MAZE_WIDTH						12.0											//[cm]
#define NSTEP_ONE_TURN   				1000 											//number of step for 1 turn of the motor
#define WHEEL_PERIMETER     			13.0 											//[cm]
#define SPEED							4.0												//[cm/s]
#define SPEED_STEP						(SPEED*NSTEP_ONE_TURN/WHEEL_PERIMETER)			//[step/s]
#define ROTATIONAL_SPEED				280												//[step]
#define STEP_TO_REACH_THE_MIDDLE		320											 	//[step]
#define ZERO_SPEED						0												//[step]
#define HISTORY_SIZE					40												//Size of navigation history buffer (store history of movements)

//Angle related
#define MIN_ANGLE						0
#define MAX_ANGLE						360
#define STEP_DEG						(1316.0/360.0)									//[step]
#define RIGHT_360						360
#define RIGHT_180						180												//[step]
#define RIGHT_90						90												//[step]
#define LEFT_360						-360											//[step]
#define LEFT_180						-180											//[step]
#define LEFT_90							-90												//[step]


//PD parameters
#define ANTI_WINDUP						5000
#define KP								1.65
#define KD								0.06


//a changer
#define VL53L0X_OPENING					120												//distance considered for opening [mm]
#define VL53L0X_OBSTRUCTED				70												//distance considered for obstructed [mm]

//Private variables
static int16_t orientation_before_check = NORTH;
static bool fire_detected = false;
static bool opening_right = false, opening_left = false, opening_front = true;

//Public variables
static uint8_t movement_state = STOP;
static int16_t orientation = NORTH;

//Store history of navigation
//static int16_t buffer_navigation_history[HISTORY_SIZE];
//static int8_t ptr_buffer_nav = 0;

//DEV
//oooooooooooooooooooooooooooooooooooooooooooooooooo

static float Kd_tun = 1.65;
static float Kp_tun = 0.06;
const float incr = 0.05;

//ooooooooooooooooooooooooooooooooooooooooooooooooooo

//Private functions
bool opening_found(void);
bool dead_end_found(void);
bool corridor_found(void);
void stop_movement(void);
void followind_corridor(void);
void moving_in_intersection(void);
void update_orientation(int rotation_angle);
void rotate(int rotation_angle);
void analysing_intersection(void);
void join_corridor(void);
void turn_towards_path(void);
void fighting_fire(void);
void searching_for_fire(void);
void reseting_orientation(void);

//Thread of motion management

static THD_WORKING_AREA(waThdMovement, 512);
static THD_FUNCTION(Movement, arg) {

    chRegSetThreadName(__FUNCTION__);
    (void)arg;

//    systime_t time;

    while(1){

//    	time = chVTGetSystemTime();

    	//call of all motion-related functions based on the current state of the robot

    	switch(movement_state){


       	case STOP: 					stop_movement();
//									opening_right = false;
//									opening_left = false;
//									opening_front = true;
//									if(check_for_fire()) deploy_antifire_measures();
//									else stop_antifire_measures();
       								break;


    	case MOVING: 				followind_corridor(); break;


    	case REACHING_INTERSECTION: moving_in_intersection(); break;


    	case ROTATING: 				analysing_intersection(); break;

    	case LEAVING_INTERSECTION: 	join_corridor(); break;

    	case SEARCHING_FIRE:		searching_for_fire(); break;

    	case FIRE_FIGHTING: 		fighting_fire(); break;

    	default: 					movement_state = STOP; break;
    	}

//    	chprintf((BaseSequentialStream *)&SD3, "Thd time = %d\n\n\r", chVTGetSystemTime()-time);

    	chThdSleepMilliseconds(50);
    }
}

//DEV
//ooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooo

int32_t absolute_value_int32(int32_t value){
	if(value < 0) value = -value;
	return value;
}


int infinity_selector(void){
	static int last_pos = 0;
	int pos = get_selector();

	if(pos == last_pos) return(0);
	if(pos == 0 && last_pos == 16){
		last_pos = pos;
		return(1);
	}
	if(pos == 16 && last_pos == 0){
		last_pos = pos;
		return(-1);
	}
	if(pos > last_pos){
		last_pos = pos;
		return(1);
	}
	if(pos < last_pos){
		last_pos = pos;
		return(-1);
	}
	return(0);
}
int16_t absolute_value_int16(int16_t value){
	if(value < 0) value = -value;
	return value;
}

void PID_tuning(void){
	static int state = 0;
	int max_state = 3;

	if(button_get_state() == 1)
	{
		if(state <= max_state) state++;
		else state = 0;
	}

	switch(state)
	{
	case 0: break;
	case 1: set_led(LED1, 1); Kp_tun += incr * infinity_selector();
	chprintf((BaseSequentialStream *)&SD3, "KP = %.2f \n\n\r", Kp_tun);break;
	case 2: set_led(LED3, 1); Kd_tun += incr * infinity_selector();
	chprintf((BaseSequentialStream *)&SD3, "KD = %.2f \n\n\r", Kd_tun);break;
	case 3: set_led(LED5, 1);
	break;
	}

}

//oooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooo


/***************************INTERNAL FUNCTIONS************************************/

/**
 * @brief 			stop the robot moving
 *
 */
void stop_movement(void){

	//Set sero speed
	right_motor_set_speed(ZERO_SPEED);
	left_motor_set_speed(ZERO_SPEED);
}

/**
 * @brief 			Check for a side opening
 *
 * @retval true		if there is an opening on at least one side.
 * @retval false	if there is no opening on both sides.
 */
bool opening_found(void){

	//if opening detected return true
	if(get_calibrated_prox(IR3) < NOISE_IR || get_calibrated_prox(IR6) < NOISE_IR)
	{
		return true;
	}
	return false;
}

/**
 * @brief 			Check for a dead end
 *
 * @retval true		if the forward path is obstructed.
 * @retval false	if there is no obstruction in front.
 */
bool dead_end_found(void){

	static uint8_t certainty_counter = 0;

	//if front is obstructed return true

	if(VL53L0X_get_dist_mm() <= VL53L0X_OBSTRUCTED)
	{
		if(certainty_counter >= CERTAINTY)
		{
			certainty_counter = 0;
			return true;
		}

		//Counter to avoid noise
		certainty_counter++;
	}
	else if(VL53L0X_get_dist_mm() >= VL53L0X_OPENING) certainty_counter = 0;

	return false;
}

/**
 * @brief 			Use a PD regulator to follow a corridor and check for opening or dead end
 *
 */
void followind_corridor(void){

	//Variables PD
	int32_t error = 0;
	int32_t past_error = 0;
	int16_t correction = 0;

	//Reset counter left motor for mapping
	left_motor_set_pos(NULL_POS);

	while(movement_state != STOP){

		//PD
		error = get_calibrated_prox(IR3) - get_calibrated_prox(IR6);

		//Calculation of trajectory correction
		correction = Kp_tun * error + Kd_tun * (error - past_error);
		past_error = error;

		//Smoother to limit  the speed of rotations
		if(correction > ROTATIONAL_SPEED) correction = ROTATIONAL_SPEED;
		else if(correction < -ROTATIONAL_SPEED) correction = -ROTATIONAL_SPEED;

		//set motor speed
		right_motor_set_speed(SPEED_STEP + correction);
		left_motor_set_speed(SPEED_STEP - correction);

		//Send movement for transmission every 1cm
		if(left_motor_get_pos() >= (NSTEP_ONE_TURN / WHEEL_PERIMETER)){
			send_corridor();
			left_motor_set_pos(NULL_POS);
		}

//		//Save history of movements for trajectory correction
//		buffer_navigation_history[ptr_buffer_nav] = correction;
//		ptr_buffer_nav++;
//		if(ptr_buffer_nav == HISTORY_SIZE) ptr_buffer_nav = 0;

		//Check for dead end or opening
		if(opening_found()){
			movement_state = REACHING_INTERSECTION;
			break;
		}
		if(dead_end_found()){
			movement_state = ROTATING;
			break;
		}

		chThdSleepMilliseconds(50);
	}
}

/**
 * @brief 			Check for a corridor
 *
 * @retval true		if a corridor is found around.
 * @retval false	if there is no corridor detected.
 */
bool corridor_found(void){

	//Counter to avoid noise
	static uint8_t certainty_counter = 0;

	//If wall found on both sides
	if(get_calibrated_prox(IR3) >= NOISE_IR && get_calibrated_prox(IR6) >= NOISE_IR )
//			 && get_calibrated_prox(IR2) >= NOISE_IR && get_calibrated_prox(IR7) >= NOISE_IR)
		{
			if(certainty_counter >= CERTAINTY)
			{
				certainty_counter = 0;
				return true;
			}
			else certainty_counter++;
		}
	else if(get_calibrated_prox(IR3) <= NOISE_IR || get_calibrated_prox(IR6) <= NOISE_IR) certainty_counter = 0;

	return false;
}

/**
 * @brief 			Performs the reverse of the last movements made by the robot in case of incorrect movement
 *
 */
//bool trajectory_correction(void){
//
//	static uint8_t counter = 0;
//
//	//Rewind back some movements after a wrong correction
//	if(ptr_buffer_nav == 0) ptr_buffer_nav = HISTORY_SIZE;
//	ptr_buffer_nav--;
//
//	right_motor_set_speed(SPEED_STEP - buffer_navigation_history[ptr_buffer_nav]);
//	left_motor_set_speed(SPEED_STEP + buffer_navigation_history[ptr_buffer_nav]);
//
//	counter++;
//	if(counter == HISTORY_SIZE){
//		counter = 0;
//		return COMPLETE;
//	}
//
//	return NOT_COMPLETE;
//}

/**
 * @brief 			Moves towards to the middle of the intersection
 *
 */
void moving_in_intersection(void){

	//Reset the right motor counter for position
	right_motor_set_pos(0);

	//Reset counter left motor for mapping
	left_motor_set_pos(NULL_POS);

	//Correction of trajectory
//	trajectory_correction();

	//Start moving forward in the crossing
	right_motor_set_speed(SPEED_STEP);
	left_motor_set_speed(SPEED_STEP);

	while(movement_state != STOP){

		//Send movement for transmission every 1cm
		if(left_motor_get_pos() >= (NSTEP_ONE_TURN / WHEEL_PERIMETER)){
			send_moving_in_intersection();
			left_motor_set_pos(NULL_POS);
		}

		//Check if a corridor has been found again
		if(corridor_found()){
			movement_state = MOVING;
			break;
		}

		//Stop when the middle is reached
		if(right_motor_get_pos() >= STEP_TO_REACH_THE_MIDDLE)
		{
			stop_movement();
			movement_state = ROTATING;
			break;
		}

		chThdSleepMilliseconds(20);
	}
}

/**
 * @brief			Update the orientation in memory and send the communication for the drawing
 *
 * @param rotation_angle     angle of rotation in deg.
 */
void update_orientation(int rotation_angle){

	orientation = orientation + rotation_angle;

	//Modulo 360°
	if(orientation >= MAX_ANGLE) orientation -= MAX_ANGLE;
	else if(orientation < MIN_ANGLE) orientation += MAX_ANGLE;

	//Send new orientation for mapping
	send_orientation(orientation);

}

/**
 * @brief			Rotate the robot and stop the movement when the desired angle is reached
 *
 * @param rotation_angle     angle of rotation in deg.
 */
void rotate(int rotation_angle){

	int32_t right_motor_pos;

	//Reset the counter
	right_motor_set_pos(NULL_POS);

	//adjust rotation direction
	if(rotation_angle > 0)
	{
		right_motor_set_speed(-ROTATIONAL_SPEED);
		left_motor_set_speed(ROTATIONAL_SPEED);
	}
	else
	{
		right_motor_set_speed(ROTATIONAL_SPEED);
		left_motor_set_speed(-ROTATIONAL_SPEED);
	}

	while(1){

		//stop when desired angle is reached
		right_motor_pos = right_motor_get_pos();
		if(absolute_value_int32(right_motor_pos) >= absolute_value_int32(rotation_angle * STEP_DEG))
		{
			stop_movement();
			break;
		}

		chThdSleepMilliseconds(20);
	}

	update_orientation(rotation_angle);
}

/**
 * @brief 			analysis of the intersection and the different openings
 *
 */
void analysing_intersection(void){

	//Store orientation before moving
	orientation_before_check = orientation;

	//Check for opening
	if(VL53L0X_get_dist_mm() >= VL53L0X_OPENING){
		opening_front = true;
//		set_led(LED1, 1);
	}
	if(get_calibrated_prox(IR3) <= NOISE_IR){
		opening_right = true;
//		set_led(LED3, 1);
	}
	if(get_calibrated_prox(IR6) <= NOISE_IR){
		opening_left = true;
//		set_led(LED7, 1);
	}

	//Send crossing for mapping
	send_crossing(opening_right, opening_front, opening_left);

	//Changing movement state
	movement_state = SEARCHING_FIRE;
}

/**
 * @brief 			Leave the intersection, chose the right path and go straight ahead until a corridor is detected
 *
 */
void join_corridor(void){

	//Take the right path
	turn_towards_path();

	//reset opening booleans
	opening_front = false;
	opening_left = false;
	opening_right = false;

	//go forward until a corridor is reached
	right_motor_set_speed(SPEED_STEP);
	left_motor_set_speed(SPEED_STEP);

	//Reset counter for mapping
	left_motor_set_pos(NULL_POS);

	while(movement_state != STOP){

		//Send movement for transmission every 1cm
		if(left_motor_get_pos() >= (NSTEP_ONE_TURN / WHEEL_PERIMETER)){
			send_moving_in_intersection();
			left_motor_set_pos(NULL_POS);
		}

		//Check for corridor
		if(corridor_found()){

			movement_state = MOVING;

			break;
		}

		//Check for dead end
		if(dead_end_found()){

			movement_state = ROTATING;

			break;
		}
	}
//			clear_leds();
}

/**
 * @brief 			Turn to follow the right wall
 *
 */
void turn_towards_path(void){
	if(opening_right)			rotate(RIGHT_90);
	else if (opening_front)		return;
	else if (opening_left) 		rotate(LEFT_90);
	else if (!opening_front && !opening_right && !opening_left) rotate(LEFT_180);
}

/**
 * @brief 			Rotates to all openings and performs fire detection
 *
 */
void searching_for_fire(void){

	//Checking front
	if(check_for_fire()){

		//Fire procedure
		opening_front = false;
		movement_state = FIRE_FIGHTING;
		return;
	}

	//Checking left
	if(opening_left){
		rotate(LEFT_90);
		if(check_for_fire()){

			//Fire procedure
			opening_left = false;
			movement_state = FIRE_FIGHTING;
			return;
		}
		//Checking right (after left)
		if(opening_right){
			rotate(RIGHT_180);
			if(check_for_fire()){

				//Fire procedure
				opening_right = false;
				movement_state = FIRE_FIGHTING;
				return;
			}
		}
	}
	//Checking right
	else if(opening_right){
		rotate(RIGHT_90);
		if(check_for_fire()){

			//Fire procedure
			opening_right = false;
			movement_state = FIRE_FIGHTING;
			return;
		}
	}

	//Orientation reset after firecheck
	reseting_orientation();

	movement_state = LEAVING_INTERSECTION;
}

/**
 * @brief 			Takes measures against the fire and checks whether the fire has been brought under control
 *
 */
void fighting_fire(void){

	//Fight against fire
	deploy_antifire_measures();
	chThdSleepMilliseconds(1000);
	stop_antifire_measures();

	if(check_for_fire() == false) movement_state = SEARCHING_FIRE;

}

/**
 * @brief 			Restoring the previously saved orientation
 *
 */
void reseting_orientation(void){

	//NORTH = 0, EAST = 90, SOUTH = 180, WEST = 270
	switch(orientation){
		case NORTH: 	switch(orientation_before_check){
							case	NORTH: 	break;
							case	EAST:	rotate(RIGHT_90); break;
							case	SOUTH:	rotate(LEFT_180); break;
							case	WEST: 	rotate(LEFT_90); break;
							default		:	break;
		}break;
		case EAST: 		switch(orientation_before_check){
							case	NORTH: 	rotate(LEFT_90); break;
							case	EAST:	break;
							case	SOUTH:	rotate(RIGHT_90); break;
							case	WEST: 	rotate(RIGHT_180); break;
							default		:	break;
		}break;
		case SOUTH: 	switch(orientation_before_check){
							case	NORTH: 	rotate(LEFT_180); break;
							case	EAST:	rotate(LEFT_90); break;
							case	SOUTH:	break;
							case	WEST: 	rotate(RIGHT_90); break;
							default		:	break;
		}break;
		case WEST: 		switch(orientation_before_check){
							case	NORTH: 	rotate(RIGHT_90); break;
							case	EAST:	rotate(RIGHT_180); break;
							case	SOUTH:	rotate(LEFT_90); break;
							case	WEST: 	break;
							default		:	break;
		}break;
		default: 		break;
	}
	orientation_before_check = NULL_POS;
}

/*************************END INTERNAL FUNCTIONS**********************************/


/****************************PUBLIC FUNCTIONS*************************************/

bool get_fire_detected(void){
	return fire_detected;
}

uint8_t get_movement_state(void){
	return movement_state;
}

uint8_t get_orientation(void){
	return orientation;
}

void set_movement_state(uint8_t state_to_set){
	movement_state = state_to_set;
}

void management_movement_start(void){
	chThdCreateStatic(waThdMovement, sizeof(waThdMovement), NORMALPRIO, Movement, NULL);
}

/**************************END PUBLIC FUNCTIONS***********************************/
