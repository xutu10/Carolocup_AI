
#ifndef SHARED_H_
#define SHARED_H_

#include <stdint.h>

struct Vec2f {
	float x;
	float y;
};


struct Line {
	struct Vec2f start;
	struct Vec2f end;
};

/**
 * left and right lane to follow the road
 **/

struct Lane {
	struct Line left;
	struct Line right;
};

/**
 * everything we get from the image processing
 **/
struct from_image {
	bool obstacles;                     /** obstacles on our lane = TRUE else FASLE **/
	struct Lane lane;                   /** the left and the right lane of the street **/
	struct Line stop_line;              /** if there is an intersection the stopline else NaN/NaN **/
	struct Line start_line;             /** if there is the start line the line else NaN/NaN **/
	struct Line vertical_obstacle;      /** if there is a car the line where it sarts or end else NaN/NaN  **/
	int counter;                        /** to check if the struct has changed since the last call **/
};

/**  * enum for the switchcase which calls the different functions  **/
enum States {
	FOLLOW_ROAD, STARTBOX, OVERTAKE, INTERSECTION, PARKING
};

/**  * enums for the startbox/intersection/overtaking Stats  **/
enum startboxStates {
	SB_INIT, SB_LINE_FOUND
};

enum intersectionStates {
	IS_INIT, IS_CHECK_FOR_STOPPING, IS_STOPPING, IS_WAITING,
	IS_TRY_PASSING
};

enum overtakeStates {
	OT_INIT, OT_CHANGE_RL, OT_PASS_OBSTACLE, OT_CHANGE_LR,
	OT_CHANGE_LR_ABORT
};

/** struct for the logging **/
typedef struct Data {
	/** if max speed if required **/
	float maxspeed;
	/** steering 500 straight 0 left 1000 right **/
	int steering;
	/** carspeed in m/s **/
	float speed;
	/** from -1 to 1 to calculate the sterling **/
	float x;
	/** Remotecontrol on/off **/
	uint32_t rc_modus;
	/** 0=parking 1=without obstacles 2=with obstacles 3=testing **/
	uint32_t modus;
	/** what must happen next (Overtake,Intersection,Follow_Road,Parking) **/
	enum States state;
	/** True=use neural network | False=use hardcoded values  **/
	bool neural_network;
	int startboxState;
	int intersectionState;
	int overtakeState;
	/* hearthbeat for loging */
	int counter;
} Data;

#endif /* ifndef SHARED_H_ */
