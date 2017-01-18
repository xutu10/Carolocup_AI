#ifndef MAIN_H_
#define MAIN_H_

#include <limits>
#include <cmath>
#include <cust_math.h>
#include <stdint.h>

enum States {FOLLOW_ROAD, STARTBOX, OVERTAKE, INTERSECTION, PARKING};

struct Line {
	Vec2f start;
	Vec2f end;
	bool is_valid();
	void invalidate();
	Vec2f to_vec();
};

typedef struct Lane {
	struct Line left;
	struct Line right;
} Lane;

struct from_image {
	bool obstacles;                 /** obstacles on our lane **/
	struct Lane lane;               /** the left and the right lane of the street **/
	float stop_line;                /** if there is an intersection distance to that**/
	float start_line;               /** distance to the start line **/
	struct Line vertical_obstacle;  /** car at the intersection **/
};

/** struct for the logging **/
class Data {
	public:
		Data() : steering(0), speed(0), x(0), rc_modus(0), modus(0), state(0) {}

		int maxspeed; /** if max speed if required **/
		int steering; /** steering 500 straight 0 left 1000 right **/
		int speed;   /** carspeed in cm/s **/
		float x;     /** from -1 to 1 to calculate the sterling **/
		uint32_t rc_modus; /** Remotecontrol on/off **/
		uint32_t modus; /** 0=parking 1=without obstacles 2=with obstacles 3=remote control **/
		int state;   /** what must happen next **/
};

float calc_steering(Lane l);

#endif /* ifndef MAIN_H_ */

