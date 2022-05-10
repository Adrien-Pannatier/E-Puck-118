/*
 * management_transmissions.h
 *
 *  Created on: 18 avr. 2022
 *      Author: APrap
 */

#ifndef MANAGEMENT_TRANSMISSIONS_H_
#define MANAGEMENT_TRANSMISSIONS_H_

void management_transmissions_start(void);
void send_orientation(uint16_t orientation);
void send_corridor(void);
void send_moving_in_intersection(void);
void send_crossing(bool opening_right, bool opening_front, bool opening_left);
void send_fire(void);


#endif /* MANAGEMENT_TRANSMISSIONS_H_ */
