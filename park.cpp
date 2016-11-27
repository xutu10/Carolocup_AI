#include "park.hpp"
#include <iostream>
using namespace std;

Park::Park() {
	status = 0;
	substatus = 0;
	sensor = new Sensor();
}

Park::~Park() {
	delete sensor;
}

int Park::controlling() {
	switch (status) {
		case 1: status = finding();
			break;
		case 2: status = backing();
			break;
		case 3: return 1; /* park finish */
	}
	return 0;
}

/*return the value of status,either 1 stands for finding(),or 2 for backing() */
int Park::finding() {

	double current_speed, timer = 0;

	if (0 == substatus && true  == sensor->side_sensor) {
		substatus = 1;
		/* ??? stop the timer */
	} else if (1 == substatus && false == sensor->side_sensor) {
		substatus = 2;
		/*start timer */
	} else if (2 == substatus && true == sensor->side_sensor) {
		substatus = 1;
		/*stop timer*/
	} else if (2 == substatus && false == sensor->side_sensor) {
		sensor->current_rotation = 100;            /* get from sensor */
		current_speed = TIRE_DIAMETER * sensor->current_rotation / 60;   /* m/s */
		if (current_speed * timer > PARKLOT_LENGTH) {
			/*stop timer*/
			return 2; /* to call backing() */
		}
	}
	return 1;
}

/*return the value of status either 2 for backing or 3 for finish
 *  disired speed  0.02m/s */
int Park::backing() {

	if (0 == substatus) {
		/*
		 *??? the value when noting in the range of back sensor
		 *??? everytime must set the rotation and steering despite no status changed */
		if (0 != sensor->back_dis) {         /* back right 30 */
			substatus = 1;
			sensor->set_rotation = -50;
			sensor->set_steering = 3.14 / 6;
		} else {                             /*no change, back right 30 */
			sensor->set_rotation = -50;
			sensor->set_steering = 3.14 / 6;
		}
	} else if (1 == substatus)  {
		if (0 == sensor->back_dis) {
			substatus = 2;                     /*back left 30*/
			sensor->set_rotation = -50;
			sensor->set_steering = -3.14 / 6;
		} else {                              /*no change, back right 30 */
			sensor->set_rotation = -50;
			sensor->set_steering = 3.14 / 6;
		}
	} else if (2 == substatus)  {
		if (sensor->back_dis < 0.05) {
			substatus = 3;                        /*forward right 20*/
			sensor->set_rotation = 50;
			sensor->set_steering = 3.14 / 9;
		} else {                                 /*no change back left 30*/
			sensor->set_rotation = -50;
			sensor->set_steering = -3.14 / 6;
		}
	} else if (3 == substatus) {
		if (sensor->back_dis > 0.2) {             /*back left 20*/
			substatus = 4;
			sensor->set_rotation = -50;
			sensor->set_steering = -3.14 / 9;
		} else {                                 /*no change forward right 20*/
			sensor->set_rotation = 50;
			sensor->set_steering = 3.14 / 9;
		}
	} else if (4 == substatus) {
		if (sensor->back_dis > 0.2) {            /*forward right 10*/
			substatus = 5;
			sensor->set_rotation = 50;
			sensor->set_steering = 3.14 / 18;
		} else {                                 /*no change back left 20*/
			sensor->set_rotation = -50;
			sensor->set_steering = -3.14 / 9;
		}
	} else if (5 == substatus) {
		if (sensor->back_dis > 0.12) {
			/*set rotation and steering 0, blink the light*/
			return 2;  /*park is finished*/
		} else {                              /*no change forward right 10*/
			sensor->set_rotation = 50;
			sensor->set_steering = 3.14 / 18;
		}
	}
	return 1;
}

