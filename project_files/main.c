#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ch.h"
#include "hal.h"
#include "memory_protection.h"
#include <usbcfg.h>
#include <main.h>
#include <chprintf.h>
#include <motors.h>
#include <audio/microphone.h>
#include <leds.h>
#include <msgbus/messagebus.h>
#include <audio/audio_thread.h>
#include <audio/play_melody.h>
#include <added_melodies.h>

messagebus_t bus;
MUTEX_DECL(bus_lock);
CONDVAR_DECL(bus_condvar);

//static THD_WORKING_AREA(waThdFrontLed, 1024);
//static THD_FUNCTION(ThdFrontLed, arg) {
//
//    chRegSetThreadName(__FUNCTION__);
//    (void)arg;
//
//    systime_t time;
//
//    while(1){
//    	playMelody(MARIO,ML_WAIT_AND_CHANGE,0);
//    }
//}

int main(void)
{
    halInit();
    chSysInit();
    mpu_init();
    messagebus_init(&bus, &bus_lock, &bus_condvar);
    proximity_start();
    spi_comm_start();
    dac_start();
    playMelodyStart();

    // chThdCreateStatic(waThdFrontLed, sizeof(waThdFrontLed), NORMALPRIO, ThdFrontLed, NULL);

    /* Infinite loop. */
    while (1) {

    	switch(get_selector())
    	{
    	case 0: playAddedMelody(ROCKY, 0,2);break;
    	case 1: stopCurrentMelody();break;
    	default: break;
    	}
//    	playMelody(ML_SIMPLE_PLAY,ML_WAIT_AND_CHANGE,NULL);
//    	waitMelodyHasFinished();


    }
}

#define STACK_CHK_GUARD 0xe2dee396
uintptr_t __stack_chk_guard = STACK_CHK_GUARD;

void __stack_chk_fail(void)
{
    chSysHalt("Stack smashing detected");
}
