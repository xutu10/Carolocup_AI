using namespace std;

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#include "speed_reg.h"
#include "carolo.h"

#define SLOWSPEED 0.5
#define MIDSPEED 0.5
#define HIGHSPEED 0.5

/**
 * regulate the speed depending on the next point the car drives to
 * x is the value for the steering; -1 left, 0 straight, 1 right
 * speed_steering *s_s is init in main.cpp and cointans the data foor the system (speed,steering)
 * float max_speed if a max speed is need else just set on 10
 **/

void SpeedSteering::reg(float x, int max_speed) {
	if (data.neural_network) {
		printf("neural\n");
		speed = 0;
		steering = 0;
	} else if (data.modus == 0) {
		speed = max_speed;
		steering = 500 + x * 500;
	} else {
		steering = 500 + x * 500;
		if (x == NAN) {
			speed = 0.3;
		} else if (fabs(x) < 0.2) {
			if (HIGHSPEED < max_speed) {
				speed = HIGHSPEED;
			} else {
				speed = max_speed;
			}
		} else if (fabs(x) < 0.5) {
			if (MIDSPEED < max_speed) {
				speed = MIDSPEED;
			} else {
				speed = max_speed;
			}
		} else if (fabs(x) >= 0.5) {
			if (SLOWSPEED < max_speed) {
				speed = SLOWSPEED;
			} else {
				speed = max_speed;
			}
		} else {
			speed = 0;
		}
	}
	data.x = x;
	data.speed = speed;
	data.maxspeed = max_speed;
	data.steering = (uint32_t) steering;
}

