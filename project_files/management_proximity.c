/*
 * Proximity.c
 *
 *  Created on: 8 avr. 2022
 *      Author: APrap
 */

#include <management_proximity.h>
#include <msgbus/messagebus.h>
#include "ch.h"
#include "hal.h"
#include "leds.h"
#include <stm32f4xx.h>
#include "sensors/proximity.h"
#include <usbcfg.h>
#include "chprintf.h"

//messagebus_t bus;
//static messagebus_t bus;
//static MUTEX_DECL(IR_lock);
//static CONDVAR_DECL(IR_condvar);

static THD_WORKING_AREA(waThdManagementProximity, 512);
static THD_FUNCTION(ThdManagementProximity, arg) {

    chRegSetThreadName("ManagementProximityThd");
    (void)arg;

    //Sensor clock init

    RCC->AHB1ENR    |= RCC_AHB1ENR_GPIOBEN;
    RCC->AHB1ENR    |= RCC_AHB1ENR_GPIOCEN;

    //Start proximity sensors
    proximity_start();
    calibrate_ir();

    //Realterm
    usb_start();
    serial_start();

	//Sensor value
    uint16_t tab_prox[PROXIMITY_NB_CHANNELS] = {0};

    while(1){

    	for(int i = 0; i < PROXIMITY_NB_CHANNELS; i++){
    		tab_prox[i] = get_calibrated_prox(i);
    	}

    	chprintf((BaseSequentialStream *)&SD3, "%u \n\n\r", tab_prox[0]);

    	chThdSleepMilliseconds(50);

    }
}

void management_proximity_start(void){
	chThdCreateStatic(waThdManagementProximity, sizeof(waThdManagementProximity), NORMALPRIO, ThdManagementProximity, NULL);
}


// COM

static void serial_start(void)
{
	static SerialConfig ser_cfg = {
	    115200,
	    0,
	    0,
	    0,
	};

	sdStart(&SD3, &ser_cfg); // UART3.
}

