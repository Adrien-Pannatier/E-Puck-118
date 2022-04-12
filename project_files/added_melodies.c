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

//	//REFRAIN
//	NOTE_A4, NOTE_A4, NOTE_B4,
//
//	NOTE_E4, NOTE_E4, NOTE_G4,
//
//	NOTE_A4, NOTE_A4, NOTE_B4,
//
//	NOTE_E4,
//
//
//	NOTE_C4, NOTE_D4, NOTE_C4, NOTE_D4,
//	NOTE_C4, NOTE_D4, NOTE_E4,
//
//	NOTE_E4, NOTE_C4, NOTE_C4, NOTE_B4,
//	NOTE_B4, NOTE_A4, NOTE_A4, NOTE_A4,
//	NOTE_G4, NOTE_F4,
//
//	NOTE_E4, NOTE_E4, NOTE_E4, NOTE_E4,
//	NOTE_E4, NOTE_E4, NOTE_E4,
//
//	NOTE_E4, NOTE_E4, NOTE_E4, NOTE_E4,
//	NOTE_E4, NOTE_E4, NOTE_G4,
//
//	NOTE_A4, NOTE_G4, NOTE_A4, NOTE_C4,
//
//	NOTE_A4,
};

//ROCKY tempo
static const float rocky_tempo[] = {
	50, 25, 12.5, 12.5,

	12.5, 7, 7, 12.5,
	7, 7, 12.5, 7,
	7, 12.5, 12.5,

	12.5, 7, 7, 12.5,
	7, 7, 12.5, 7,
	7, 25,

	12.5, 7, 7, 12.5,
	7, 7, 7, 7,
	12.5, 12.5, 12.5,

	12.5, 7, 7, 7,
	12.5, 7, 25, 7,
	18.75,

	75, 7, 18.75,

	75, 7, 18.75,

	75, 7, 18.75,

	100,

	12.5, 7, 7, 18.75,
	7, 7, 43.75,

	12.5, 7, 7, 12.5,
	7, 12.5, 7, 25,
	12.5,

	25, 25, 12.5, 12.5,
	12.5, 12.5,

	7, 12.5, 7, 12.5,
	7, 12.5, 7, 12.5,
	7, 18.75,

//	75, 7, 18.75,
//
//	75, 7, 18.75,
//
//	75, 7, 18.75,
//
//	100,
//
//	12.5, 7, 7, 18.75,
//	7, 7, 43.75,
//
//	12.5, 7, 7, 12.5,
//	7, 7, 7, 7,
//	25, 12.5,
//
//	25, 12.5, 12.5, 12.5,
//	12.5, 12.5, 12.5,
//
//	12.5, 12.5, 12.5, 12.5,
//	25, 7, 18.75,
//
//	25, 7, 18.75, 25,
//
//	125,

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

