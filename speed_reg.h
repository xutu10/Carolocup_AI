#ifndef SPEED_STERLING_H_
#define SPEED_STERLING_H_

#include "carolo.h"
#include "main.h"

/**
 * this class is to set the speed and steering.
 * the reg function wants an x value between -1 and 1 to calculate the steering and the speed. the second value is to set and maxspeed
 * if you need to regulate the speed (at example parking)
 **/


class SpeedSteering {
	private:
		float speed;
		float steering;
	public:
		Data& data;
		struct carolo *car;
		SpeedSteering(Data &data, struct carolo *car) : speed(0), steering(500),
			data(data) {}

		/**
		 * @param x -1(left) to 1 (right) to set the steering correct
		 * @param max_speed. if a maximal speed is needed
		 * calculate the speed and the steering and calls the functions in libcarolo to set them
		 * neural network sets them if enabled and fixed values otherwise
		 **/
		void reg(float x, int max_speed = 10);

};


#endif /* ifndef SPEED_STERLING_H_ */

