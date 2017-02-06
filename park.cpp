#include "park.h"
using namespace ki;
/**
 *  control the whole parking process, according to the status
 *  call the corresponding function
 *
 *  @return 1 to main, if the parking finished
 */

Park::Park(carolo_telemetry *tel, SpeedSteering *ss): tele(tel), s_s(ss), status(FINDING),substatus(0){}

int Park::controlling() {
	switch (status) {
		case FINDING: status = finding();
			break;
		case BACKING: status = backing();
			break;
		case FINISHED: return 1; /* park finish */
	}
	return 0;
}

/**
 *  look for a park space whose length is greater than 0.6m
 *  side_sensor, distance mebmer in telemetry struct are needed
 *
 *  @return the current status back to controlling(),
 */
int Park::finding() {

	bool side_sensor = false;        /* !!! it should be from telemetry struct. But so far it's broken */
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
		if (tele->distance - pre_distance > PARK_SPACE) {
			pre_distance = 0;                        /*stop countring*/
			substatus = 0;                           /*initial for backing*/
			return BACKING;                                /* to call backing() */
		} else  {
			if (true == side_sensor) {
				pre_distance = 0;
				substatus = 1;
			}
		}
	}
s_s->reg(0,0.1);
return FINDING;
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
	// ales ? move to config
	float maxspeed = 0.05;
	float tmp_backsensor = tele->us[0];
	
	if (0 == substatus) {
		if (0 != tmp_backsensor) {         /* back right 30 */
			substatus = 1;
			x = 1;
		} else {                        /*initiallizing back right 30 */
			x = 1;
		}
	} else if (1 == substatus && 0 == tmp_backsensor) {     /*back left 30*/
		substatus = 2;
		x = -1;
	}
  else if (2 == substatus && tmp_backsensor < 0.05) {        /*forward right 20*/
	substatus = 3;
	x = 2. / 3;
}
 else if (3 == substatus && tmp_backsensor > 0.15) {                 /*back left 20*/
	substatus = 4;
	x = -2. / 3;
}
 else if (4 == substatus && tmp_backsensor < 0.05) {              /*forward right 10*/
	substatus = 5;
	x = 1. / 3;
}
 else if (5 == substatus && tmp_backsensor > 0.12) {
	s_s->reg(0, 0);                        /*stop the car*/                                  
	return FINISHED;                              /*park is finished*/
}

s_s->reg(x, maxspeed);
return BACKING;
}
