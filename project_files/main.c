#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ch.h"
#include "hal.h"
#include "memory_protection.h"
#include <main.h>
#include <motors.h>
#include <audio/microphone.h>
#include <leds.h>
#include <msgbus/messagebus.h>
#include <audio/audio_thread.h>
#include <audio/play_melody.h>
#include <sensors/proximity.h>
#include <added_melodies.h>
#include "spi_comm.h"
#include "management_proximity.h"
#include "management_movement.h"
#include "management_transmissions.h"

messagebus_t bus;
MUTEX_DECL(bus_lock);
CONDVAR_DECL(bus_condvar);

int main(void)
{
    halInit();
    chSysInit();
    mpu_init();

    spi_comm_start();
    playMelodyStart();

    //start movement related thread
    management_transmissions_start();
    management_proximity_start();
    management_movement_start();

    messagebus_init(&bus, &bus_lock, &bus_condvar);

    /* Infinite loop. */
    while (1) {

    	chThdSleepMilliseconds(500);

    }
}

#define STACK_CHK_GUARD 0xe2dee396
uintptr_t __stack_chk_guard = STACK_CHK_GUARD;

void __stack_chk_fail(void)
{
    chSysHalt("Stack smashing detected");
}
