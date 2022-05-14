/*
 * added_melodies.h
 *
 *  Created on: 8 avr. 2022
 *     Authors: Axel Praplan, Adrien Pannatier
 *
 *  Functions and defines to store and play custom melodies and alarms, using the play_melody.h library
 *
 *  adapted from the library code
 */

#ifndef ADDED_MELODIES_H_
#define ADDED_MELODIES_H_

#include <audio/play_melody.h>
#include <ch.h>
#include <hal.h>

//frequencies for the international alarm
#define A_0		700
#define A_1		735
#define A_2		770
#define A_3		805
#define A_4		840
#define A_5		875
#define A_6		910
#define A_7		945
#define A_8		980
#define A_9		1015
#define A_10	1050
#define A_11	1085
#define A_12	1120
#define A_13	1155
#define A_14	1190
#define A_15	1225
#define A_16	1260
#define A_17	1295
#define A_18	1330
#define A_19	1365
#define A_20	1400
#define A_21	1435
#define A_22	1470
#define A_23	1500

//available new songs
typedef enum{
	//internal songs available
	ROCKY = 0,
	GHOST_BUSTERS,
	NB_ADDED_SONGS,	//tell the number of internal songs
}added_song_selection_t;

//available alarms
typedef enum{
	//internal alarms available
	PINPON = 0,
	SIREN_INTERNATIONAL,
	BASEBALL,
	NB_ADDED_ALARMS,	//tell the number of internal alarms
}added_alarm_selection_t;

/**
 * @brief Plays the selected melody from the external library. Does nothing if the module has not been
 * 							 started with playMelodyStart()	This function doesn't block the current thread.
 * 							 It uses it's self thread
 *
 * @param choice 			Song selected (see added_song_selection_t)
 * @param option			Behavior to change the melody playing (see play_melody_option_t)
 *
 */
void playAddedMelody(added_song_selection_t choice, play_melody_option_t option);

/**
 * @brief Starts the play_added_alarm module
 *
 */
void playAddedAlarmStart(void);

/**
 * @brief Plays the selected alarm. Does nothing if the module has not been started with playAddedAlarmStart()
 * 							This function doesn't block the current thread. It uses it's self thread
 *
 * @param choice 			Alarm selected (see added_alarm_selection_t)
 * @param option			Behavior to change the alarm playing (see play_melody_option_t)
 *
 */
void playAddedAlarm(added_alarm_selection_t choice, play_melody_option_t option);

/**
 * @brief 	Stops the Alarm beeing played. Even if this function returns immediatly,
 * 			the stopping of the playback is not immediate. It can take a very little time.
 * 			Use waitAlarmHasFinished() to be sure the sound has stopped.
 */
void stopCurrentAlarm(void);

/**
 * @brief Waits until the alarm playing has finished (put the invocking thread in sleep)
 * 		  Immediatly returns if no alarm is playing
 */
void waitAlarmHasFinished(void);

#endif /* ADDED_MELODIES_H_ */
