#include "park.h"
#include <iostream>
using namespace std;

/**
 *  control the whole parking process, according to the status
 *  call the corresponding function
 *
 *  @return 1 to main, if the parking finished
 */

int Park::controlling() {
	switch (status) {
		case 0: status = finding();
			break;
		case 1: status = backing();
			break;
		case 2: return 1; /* park finish */
	}
	return 0;
}

/**
 *  look for a park lot whose length is greater than 600mm
 *  side_sensor, distance from telemetry struct are needed
 *
 *  @return the current status back to controlling(),
 *  either 0 stands for finding(), or 1 for backing()
 */
int Park::finding() {

	bool side_sensor = false;        /* !!! it should be from telemetry struct */
	if (0 == substatus && true  == side_sensor) {
		substatus = 1;
	} else if (1 == substatus && false == side_sensor) {
		substatus = 2;
		pre_distance = tele->distance;      /*start to count distance */
	} else if (2 == substatus && true == side_sensor) {
		substatus = 1;
		pre_distance = 0;    /*stop counting*/
	} else if (2 == substatus && false == side_sensor) {
		substatus = 3;
	} else if (3 == substatus) {
		if (tele->distance - pre_distance > PARKLOT_LENGTH) {
			pre_distance = 0;                        /*stop countring*/
			substatus = 0;                           /*initial for backing*/
			return 1;                                /* to call backing() */
		} else  {
			if (true == side_sensor) {
				pre_distance = 0;
				substatus = 1;
			}
		}
	}
s_s->reg(0,0.1);
return 0;
}

/**
 *  backing the car into the park lot,  desired speed  0.05m/s
 *  based on back_sensor to change the substatus
 *  x is from -1 to 1, temporarily i set 1 = 30 degree,
 *  -1 = -30 which means turn left 30 degree
 *
 *  @return status to controlling(), 1 for back, 2 for finish
 */
int Park::backing() {
	float x = 0;
	float max_speed = 0.05;

	if (0 == substatus) {
		if (0 != tele->us[0]) {         /* back right 30 */
			substatus = 1;
			x = 1;
		} else {                        /*initiallizing back right 30 */
			x = 1;
		}
	} else if (1 == substatus && 0 == tele->us[0]) {     /*back left 30*/
		substatus = 2;
		x = -1;
	}
  else if (2 == substatus && tele->us[0] < 0.05) {        /*forward right 20*/
	substatus = 3;
	x = 2 / 3;
}
 else if (3 == substatus && tele->us[0] > 0.15) {                 /*back left 20*/
	substatus = 4;
	x = -2 / 3;
}
 else if (4 == substatus && tele->us[0] < 0.05) {              /*forward right 10*/
	substatus = 5;
	x = 1 / 3;
}
 else if (5 == substatus && tele->us[0] > 0.12) {
	s_s->reg(0, 0);                        /*stop the car*/                                  
	return 2;                              /*park is finished*/
}

s_s->reg(x, max_speed);
return 1;
}
