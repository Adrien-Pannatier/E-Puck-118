#include "stdio.h"
#include "stdlib.h"
#include "string.h"

#include "main.h"
#include "ch.h"
#include "hal.h"
#include "selector.h"
#include "memory_protection.h"

#include "spi_comm.h"
//#include "leds.h"
#include "light_gestion.h"


#include "audio/microphone.h"
#include "audio/audio_thread.h"
#include "audio/play_melody.h"
#include "added_melodies.h"

#include "process_image.h"
#include "camera/po8030.h"

#include "chmtx.h"
#include "msgbus/messagebus.h"
#include "sensors/proximity.h"
#include "management_proximity.h"

#include "usbcfg.h"
#include "management_transmissions.h"

#include "motors.h"
#include "management_movement.h"

messagebus_t bus;
MUTEX_DECL(bus_lock);
CONDVAR_DECL(bus_condvar);

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

int main(void)
{
	//System
    halInit();
    chSysInit();
    mpu_init();

    //LED
    //spi_comm_start();
    LED_start();

    //Music
    dac_start();
    playMelodyStart();
    playAddedAlarmStart();

    //Camera
    dcmi_start();
    po8030_start();
    process_image_start();

    //IR
    messagebus_init(&bus, &bus_lock, &bus_condvar);
    proximity_start();
    management_proximity_start();

    //Transmission
    usb_start();
    serial_start();
    management_transmissions_start();

    //Movement
    motors_init();
    management_movement_start();

    /* Infinite loop. */
    while (1) {

    	//Selector control
    	switch (get_selector())
    	{
			case 0:  set_movement_state(STOP);	break;

			case 1: if(get_movement_state() == STOP) set_movement_state(LEAVING_INTERSECTION); break;

			default: set_movement_state(STOP);	break;
    	}

    	chThdSleepMilliseconds(50);
    }
}

#define STACK_CHK_GUARD 0xe2dee396
uintptr_t __stack_chk_guard = STACK_CHK_GUARD;

void __stack_chk_fail(void)
{
    chSysHalt("Stack smashing detected");
}
