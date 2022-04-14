/*
 * Proximity.c
 *
 *  Created on: 8 avr. 2022
 *      Author: APrap
 */

#include <proximity.h>
#include <msgbus/messagebus.h>

static THD_WORKING_AREA(waThdProximity, 1024);
static THD_FUNCTION(ThdProximity, arg) {

    chRegSetThreadName(__FUNCTION__);
    (void)arg;

    //inititalisation proximity sensors
    proximity_start();
    messagebus_init(&bus, &bus_lock, &bus_condvar);

    systime_t time;

    while(1){
    	chThdSleepMilliseconds(500);
    }
}


