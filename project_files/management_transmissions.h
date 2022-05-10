/*
 * management_transmissions.h
 *
 *  Created on: 18 avr. 2022
 *      Authors: Axel Praplan, Adrien Pannatier
 *
 *  Thread and buffer to send labyrinth info to the computer
 *  via bluetooth
 */

#ifndef MANAGEMENT_TRANSMISSIONS_H_
#define MANAGEMENT_TRANSMISSIONS_H_

/**
 * @brief Activates the transmission thread
 *
 */
void management_transmissions_start(void);

/**
 * @brief Sends the orientation to computer
 *
 */
void send_orientation(uint16_t orientation);

/**
 * @brief Sends corridor surrounding to computer
 *
 */
void send_corridor(void);

/**
 * @brief Sends intersection surrounding to computer
 *
 */
void send_moving_in_intersection(void);

/**
 * @brief Sends crossing type to computer
 *
 */
void send_crossing(bool opening_right, bool opening_front, bool opening_left);

/**
 * @brief Sends fire existance to computer
 *
 */
void send_fire(void);


#endif /* MANAGEMENT_TRANSMISSIONS_H_ */
