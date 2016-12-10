#ifndef MAIN_H_
#define MAIN_H_

#include <stdint.h>

enum States {FOLLOW_ROAD, STARTBOX, OVERTAKE, INTERSECTION, PARKING};
void rc_wait();



/** struct for the logging **/
class Data {
public:
Data() : steering(0), speed(0), x(0), rc_modus(0), modus(0), state(0) {
}
int maxspeed;        /** if max speed if required **/
int steering;        /** steering 500 straight 0 left 1000 right **/
int speed;           /** carspeed in cm/s **/
float x;               /** from -1 to 1 to calculate the sterling **/
uint32_t rc_modus;       /** Remotecontrol on/off **/
uint32_t modus;           /** 0=parking 1=without obstacles 2=with obstacles 3=remote control **/
int state;           /** what must happen next **/


};

#endif /* ifndef MAIN_H_ */

