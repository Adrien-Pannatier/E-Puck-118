/*
 * added_melodies.c
 *
 *  Created on: 8 avr. 2022
 *      Author: AdrienPannatier
 */

#include "added_melodies.h"
#include "leds.h"

//conditional variable
static MUTEX_DECL(play_add_alarm_lock);
static CONDVAR_DECL(play_add_alarm_condvar);

//reference
static thread_reference_t play_add_alarm_ref = NULL;

//variable to stop the playing if necessary
static bool addplay = true;

//ROCKY melody
static const uint16_t rocky_melody[] = {
	NOTE_C4,

	NOTE_C4, NOTE_C4, NOTE_C4, NOTE_C4,
	NOTE_C4, NOTE_C4, NOTE_E4, NOTE_C4,
	NOTE_C4, NOTE_C4,NOTE_E4,

	NOTE_E4, NOTE_E4, NOTE_E4, NOTE_E4,
	NOTE_E4, NOTE_E4, NOTE_G4, NOTE_E4,
	NOTE_E4, NOTE_E4,

	NOTE_D4, NOTE_D4, NOTE_D4, NOTE_D4,
	NOTE_D4, NOTE_D4, NOTE_D4, NOTE_D4,
	NOTE_D4, 0, NOTE_D4,

	NOTE_D4, NOTE_D4, NOTE_D4, NOTE_D4,
	NOTE_D4, NOTE_D4, NOTE_D4, NOTE_E4,
	NOTE_G4,

	//REFRAIN
	NOTE_A4, NOTE_A4, NOTE_B4,

	NOTE_E4, NOTE_E4, NOTE_G4,

	NOTE_A4, NOTE_A4, NOTE_B4,

	NOTE_E4,


	NOTE_C4, NOTE_D4, NOTE_C4, NOTE_D4,
	NOTE_C4, NOTE_D4, NOTE_E4,

	NOTE_A3, NOTE_C4, NOTE_C4, NOTE_B3,
	NOTE_B3, NOTE_A3, NOTE_B3, NOTE_B3,
	NOTE_F4,

	NOTE_E4, NOTE_E4, NOTE_E4, NOTE_E4,
	NOTE_E4, NOTE_E4,

	NOTE_E4, NOTE_E4, NOTE_E4, NOTE_E4,
	NOTE_E4, NOTE_E4, NOTE_E4, NOTE_E4,
	NOTE_E4, NOTE_G4,

	//REFRAIN
	NOTE_A4, NOTE_A4, NOTE_B4,

	NOTE_E4, NOTE_E4, NOTE_G4,

	NOTE_A4, NOTE_A4, NOTE_B4,

	NOTE_E4,


	NOTE_C4, NOTE_D4, NOTE_C4, NOTE_D4,
	NOTE_C4, NOTE_D4, NOTE_E4,

	NOTE_E4, NOTE_C4, NOTE_C4, NOTE_B4,
	NOTE_B4, NOTE_A4, NOTE_A4, NOTE_A4,
	NOTE_G4, NOTE_F4,

	NOTE_E4, NOTE_E4, NOTE_E4, NOTE_E4,
	NOTE_E4, NOTE_E4, NOTE_E4,

	NOTE_E4, NOTE_E4, NOTE_E4, NOTE_E4,
	NOTE_E4, NOTE_E4, NOTE_G4,

	NOTE_A4, NOTE_G4, NOTE_A4, NOTE_C4,

	NOTE_A4,0
};

//PINPON melody
static const uint16_t pinpon_melody[] = {
	NOTE_A4, NOTE_B4,NOTE_A4, NOTE_B4,
	NOTE_A4, NOTE_B4,
};

//ROCKY tempo
static const float rocky_tempo[] = {
	8,

	8, 16, 16, 8,
	16, 16, 8, 16,
	16, 8, 8,

	8, 16, 16, 8,
	16, 16, 8, 16,
	16, 4,

	8, 16, 16, 8,
	16, 16, 16, 16,
	8, 8, 8,

	8, 16, 16, 16,
	8, 16, 4, 16,
	16/3,

	1.3, 16, 16/3,

	1.3, 16, 16/3,

	1.3, 16, 16/3,

	1,

	8, 16, 16, 16/3,
	16, 16, 16/7,

	8, 16, 16, 8,
	16, 8, 16, 4,
	8,

	4, 4, 8, 8,
	8, 8,

	16, 8, 16, 8,
	16, 8, 16, 8,
	16, 16/3,

	1.3, 16, 16/3,

	1.3, 16, 16/3,

	1.3, 16, 16/3,

	1,

	8, 16, 16, 16/3,
	16, 16, 16/7,

	8, 16, 16, 8,
	16, 16, 16, 16,
	4, 8,

	4, 8, 8, 8,
	8, 8, 8,

	8, 8, 8, 8,
	4, 16, 16/3,

	4, 16, 16/3, 4,

	4/5,1

};

static const float pinpon_tempo[] = {
	1,1,1,
	1,1,1
};

static const melody_t added_melodies[NB_ADDED_SONGS] = {
  //ROCKY
  {
    .notes = rocky_melody,
    .tempo = rocky_tempo,
    .length = sizeof(rocky_melody)/sizeof(uint16_t),
  },
};

static const melody_t added_alarms[NB_ADDED_ALARMS] = {
  //PINPON
  {
    .notes = pinpon_melody,
    .tempo = pinpon_tempo,
    .length = sizeof(pinpon_melody)/sizeof(uint16_t),
  },
};

void playAddedMelody(added_song_selection_t choice, play_melody_option_t option){
	//if(sizeof(rocky_melody)/sizeof(uint16_t)==sizeof(rocky_tempo)/sizeof(float))set_led(LED1,1);
	playMelody(EXTERNAL_SONG, option, &added_melodies[choice]);
}

static THD_WORKING_AREA(waPlayAddAlarmThd, 128);
static THD_FUNCTION(PlayAddAlarmThd, arg) {

  chRegSetThreadName("PlayAddAlarm Thd");

	(void)arg;

	static melody_t* alarm = NULL;

	while(1){
		//this thread is waiting until it receives a message
		chSysLock();
		alarm = (melody_t*) chThdSuspendS(&play_add_alarm_ref);
		chSysUnlock();

		for (int thisNote = 0; thisNote < alarm->length; thisNote++) {

      if(!addplay){
        break;
      }

			// to calculate the note duration, take one second
			// divided by the note type.
			//e.g. quarter note = 1000 / 4, eighth note = 1000/8, etc.
			uint16_t noteDuration = (uint16_t)(1000 / alarm->tempo[thisNote]);

			playNote(alarm->notes[thisNote], noteDuration);

			// to distinguish the notes, set a minimum time between them.
			// the note's duration + 30% seems to work well:
//			uint16_t pauseBetweenNotes = (uint16_t)(noteDuration * 1.30);
//			chThdSleepMilliseconds(pauseBetweenNotes);

		}
    addplay = false;
    //signals to the threads waiting that the melody has finished
    chCondBroadcast(&play_add_alarm_condvar);
	}
}

void playAddedAlarmStart(void){

	//create the thread
	chThdCreateStatic(waPlayAddAlarmThd, sizeof(waPlayAddAlarmThd), NORMALPRIO, PlayAddAlarmThd, NULL);
}

void playAddedAlarm(added_alarm_selection_t choice, play_melody_option_t option){

  melody_t* alarm = NULL;

  alarm = &added_alarms[choice];

  //SIMPLE_PLAY case
  if(option == ML_SIMPLE_PLAY){
    //if the reference is NULL, then the thread is already running
    //when the reference becomes not NULL, it means the thread is waiting
    if(play_add_alarm_ref != NULL){
      addplay = true;
      //tell the thread to play the alarm given
      chThdResume(&play_add_alarm_ref, (msg_t) alarm);
    }
  }//FORCE_CHANGE or WAIT_AND_CHANGE cases
  else{
    if(option == ML_FORCE_CHANGE){
      stopCurrentAlarm();
    }
    waitAlarmHasFinished();
    addplay = true;
    //tell the thread to play the alarm given
    chThdResume(&play_add_alarm_ref, (msg_t) alarm);
  }
}

void stopCurrentAlarm(void){
    addplay = false;
}

void waitAlarmHasFinished(void) {
  //if a melody is playing
  if(play_add_alarm_ref == NULL){
    //waits until the current melody is finished
    chMtxLock(&play_add_alarm_lock);
    chCondWait(&play_add_alarm_condvar);
    chMtxUnlock(&play_add_alarm_lock);
  }
}
