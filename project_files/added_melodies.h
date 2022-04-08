/*
 * added_melodies.h
 *
 *  Created on: 8 avr. 2022
 *      Author: adrie
 */

#ifndef ADDED_MELODIES_H_
#define ADDED_MELODIES_H_

#include <audio/play_melody.h>

//available new songs
typedef enum{
	//internal songs available
	ROCKY = 0,
	NB_ADDED_SONGS,	//tell the number of internal songs
}added_song_selection_t;

#endif /* ADDED_MELODIES_H_ */
