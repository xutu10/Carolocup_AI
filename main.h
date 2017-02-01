#ifndef MAIN_H_
#define MAIN_H_

#include <limits>
#include <cmath>
#include <cust_math.h>
#include <stdint.h>
#include "shared.h"

/**
 * enum for the switchcase which calls the different functions
 **/
enum States {FOLLOW_ROAD, STARTBOX, OVERTAKE, INTERSECTION, PARKING};

enum startboxStates {SB_INIT, SB_LINE_FOUND};

enum intersectionStates {IS_INIT, IS_CHECK_FOR_STOPPING, IS_STOPPING, IS_WAITING, IS_TRY_PASSING};

enum overtakeStates {OT_INIT, OT_CHANGE_RL, OT_PASS_OBSTACLE, OT_CHANGE_LR, OT_CHANGE_LR_ABORT};


namespace ki {

/** struct for the logging **/
class Data {
	public:
		Data() : steering(0), speed(0), x(0), rc_modus(0), modus(0), state(FOLLOW_ROAD) {}

		float maxspeed;               /** if max speed if required **/
		int steering;               /** steering 500 straight 0 left 1000 right **/
		float speed;                /** carspeed in m/s **/
		float x;                    /** from -1 to 1 to calculate the sterling **/
		uint32_t rc_modus;          /** Remotecontrol on/off **/
		uint32_t modus;             /** 0=parking 1=without obstacles 2=with obstacles 3=testing **/
		States state;               /** what must happen next (Overtake,Intersection,Follow_Road,Parking) **/
		bool neural_network;        /** True=use neural network | False=use hardcoded values  **/
		int startboxState;
		int intersectionState;
		int overtakeState;
};


}
#endif /* ifndef MAIN_H_ */

