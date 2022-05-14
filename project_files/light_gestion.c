/*
 * light_gestion.c
 *
 *  Created on: 12 avr. 2022
 *     Authors: Axel Praplan, Adrien Pannatier
 *
 *  Thread declaration and defines for LED responses depending on
 *  the state of the robot
 */

#include "light_gestion.h"
#include "ch.h"
#include "hal.h"
#include "leds.h"
#include "management_movement.h"

static THD_WORKING_AREA(waThdLED, 128);
static THD_FUNCTION(ThdLED, arg) {

    chRegSetThreadName(__FUNCTION__);
    (void)arg;

    while (1)
    {
    	if(get_movement_state() == STOP)
    	{
    		//LEDs OFF
    		clear_leds();
    		chThdSleepMilliseconds(ALAMRM_SPEED);
    	}
    	else if(get_movement_state() == FIRE_FIGHTING)
       	{
    		//pattern for FIRE_FIGHTING state
    		clear_leds();
    		set_led(LED1, LED_ON);
    		chThdSleepMilliseconds(ALAMRM_SPEED);
    		set_led(LED1, LED_OFF);
    		chThdSleepMilliseconds(ALAMRM_SPEED);
    		set_led(LED3, LED_ON);
    		chThdSleepMilliseconds(ALAMRM_SPEED);
    		set_led(LED3, LED_OFF);
    		chThdSleepMilliseconds(ALAMRM_SPEED);
    		set_led(LED5, LED_ON);
    		chThdSleepMilliseconds(ALAMRM_SPEED);
    		set_led(LED5, LED_OFF);
    		chThdSleepMilliseconds(ALAMRM_SPEED);
    		set_led(LED7, LED_ON);
    		chThdSleepMilliseconds(ALAMRM_SPEED);
    		set_led(LED7, LED_OFF);
    		chThdSleepMilliseconds(ALAMRM_SPEED);
       	}
    	else if(get_movement_state() == SEARCHING_FIRE)
       	{
    		//pattern for SEARCHING FIRE state
    		clear_leds();
    		set_led(LED1, LED_ON);
    		set_led(LED5, LED_ON);
    		chThdSleepMilliseconds(ALAMRM_SPEED);
    		set_led(LED1, LED_OFF);
    		set_led(LED5, LED_OFF);
    		chThdSleepMilliseconds(ALAMRM_SPEED);
       		set_led(LED3, LED_ON);
    		set_led(LED7, LED_ON);
    		chThdSleepMilliseconds(ALAMRM_SPEED);
    		set_led(LED3, LED_OFF);
    		set_led(LED7, LED_OFF);
    		chThdSleepMilliseconds(ALAMRM_SPEED);
       	}
    	else if(get_movement_state() == END_OF_MAZE)
    	{
    		//pattern for END OF MAZE state
    		set_body_led(1);
    	    chThdSleepMilliseconds(ALAMRM_SPEED);
    	    set_body_led(0);
    	    chThdSleepMilliseconds(ALAMRM_SPEED);
    	}
    }
}

/**************************PUBLIC FUNCTIONS***********************************/

void LED_start(void){
	chThdCreateStatic(waThdLED, sizeof(waThdLED), NORMALPRIO -1, ThdLED, NULL);
}

/**************************END PUBLIC FUNCTIONS***********************************/
