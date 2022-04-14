/*
 * added_melodies.c
 *
 *  Created on: 8 avr. 2022
 *      Author: AdrienPannatier
 */

#include "added_melodies.h"
#include "leds.h"

//ROCKY melody
static const uint16_t rocky_melody[] = {
	0, 0, 0, NOTE_C4,

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

	NOTE_A4, NOTE_C4, NOTE_C4, NOTE_B4,
	NOTE_B4, NOTE_A4, NOTE_B4, NOTE_B4,
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

	NOTE_A4,
};

//ROCKY tempo
static const float rocky_tempo[] = {
	500, 250, 125, 125,

	125, 62.5, 62.5, 125,
	62.5, 62.5, 125, 62.5,
	62.5, 125, 125,

	125, 62.5, 62.5, 125,
	62.5, 62.5, 125, 62.5,
	62.5, 250,

	125, 62.5, 62.5, 125,
	62.5, 62.5, 62.5, 62.5,
	125, 125, 125,

	125, 62.5, 62.5, 62.5,
	125, 62.5, 250, 62.5,
	187.5,

	750, 62.5, 187.5,

	750, 62.5, 187.5,

	750, 62.5, 187.5,

	1000,

	125, 62.5, 62.5, 187.5,
	62.5, 62.5, 437.5,

	125, 62.5, 62.5, 125,
	62.5, 125, 62.5, 250,
	125,

	250, 250, 125, 125,
	125, 125,

	62.5, 125, 62.5, 125,
	62.5, 125, 62.5, 125,
	62.5, 187.5,

	750, 62.5, 187.5,

	750, 62.5, 187.5,

	750, 62.5, 187.5,

	1000,

	125, 62.5, 62.5, 187.5,
	62.5, 62.5, 437.5,

	125, 62.5, 62.5, 125,
	62.5, 62.5, 62.5, 62.5,
	250, 125,

	250, 125, 125, 125,
	125, 125, 125,

	125, 125, 125, 125,
	250, 62.5, 187.5,

	250, 62.5, 187.5, 250,

	1250,

};




static const melody_t added_melodies[NB_ADDED_SONGS] = {
  //ROCKY
  {
    .notes = rocky_melody,
    .tempo = rocky_tempo,
    .length = sizeof(rocky_melody)/sizeof(uint16_t),
  },
};

void playAddedMelody(added_song_selection_t choice, play_melody_option_t option, uint16_t tempo_music){
	//if(sizeof(rocky_melody)/sizeof(uint16_t)==sizeof(rocky_tempo)/sizeof(float))set_led(LED1,1);
	playMelody(EXTERNAL_SONG, option, &added_melodies[choice]);
}

