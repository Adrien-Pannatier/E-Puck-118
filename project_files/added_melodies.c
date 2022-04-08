/*
 * added_melodies.c
 *
 *  Created on: 8 avr. 2022
 *      Author: AdrienPannatier
 */

#include "added_melodies.h"

//ROCKY melody
static const uint16_t rocky_melody[] = {
    NOTE_G4, 0, 0, NOTE_G4,
    0, 0, NOTE_AS4, 0 ,
    NOTE_C5, 0, NOTE_G4, 0,
    0, NOTE_G4, 0, 0,
    NOTE_F4, 0, NOTE_FS4, 0,

    NOTE_G4, 0, 0, NOTE_G4,
    0, 0, NOTE_AS4, 0 ,
    NOTE_C5, 0, NOTE_G4, 0,
    0, NOTE_G4, 0, 0,
    NOTE_F4, 0, NOTE_FS4, 0,

    NOTE_AS5, NOTE_G5, NOTE_D5,0,
    0,0,0,0,
    0,0, NOTE_AS5, NOTE_G5,
    NOTE_CS5,0, 0, 0,
    0, 0, 0, 0,
    NOTE_AS5, NOTE_G5, NOTE_C5,0,
    0, 0, 0, 0,
    0, 0, NOTE_AS4, NOTE_C5,
    0, 0, 0, 0,
    0, 0, 0, 0

};

//ROCKY tempo
static const float rocky_tempo[] = {
      12, 12, 12, 12,
      12, 12, 12, 12,
      12, 12, 12, 12,
      12, 12, 12, 12,
      12, 12, 12, 12,

      12, 12, 12, 12,
      12, 12, 12, 12,
      12, 12, 12, 12,
      12, 12, 12, 12,
      12, 12, 12, 12,

      12, 12, 12, 12,
      12, 12, 12, 12,
      12, 12, 12, 12,
      12, 12, 12, 12,
      12, 12, 12, 12,

      12, 12, 12, 12,
      12, 12, 12, 12,
      12, 12, 12, 12,
      12, 12, 12, 12,
      12, 12, 12, 12

};

static const melody_t added_melodies[NB_ADDED_SONGS] = {
  //ROCKY
  {
    .notes = rocky_melody,
    .tempo = rocky_tempo,
    .length = sizeof(rocky_melody)/sizeof(uint16_t),

  },
};


