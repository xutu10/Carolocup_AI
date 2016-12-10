#include "park.h"
#include <iostream>
using namespace std;

Park::Park() {
	status = 0;
	substatus = 0;
	sensor = new Sensor();
	data = new Data();
}

Park::~Park() {
	delete sensor;
	delete data;
}

/**
   control the whole parking process, according to the status
   call the corresponding function

   @return 1 to main, if the parking finished
*/

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

/**
   search a park lot whose length is greater than 600mm
   side_sensor, timer and rotation are needed
   
   @return the current status back to controlling(),
   either 1 stands for finding(), or 2 for backing()
 */
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

/**
   backing the car into the park lot,  disired speed  0.02m/s
   based on back_sensor to change the substatus
   pass the desired rotation and steering to system through *data
   data-> x is from -1 to 1, temporarily I set 1 = 30 degree,
   -1 = -30 which means turn left 30 degree
   
  @return status to controlling(), if the backing finished
*/
int Park::backing() {

	if (0 == substatus) {
		if (0 != sensor->back_dis) {         /* back right 30 */
			substatus = 1;
			data->maxspeed = -50;
			data->x = 1;
		} else {                        /*no change, back right 30 */
			data->maxspeed = -50;
			data->x = 1;
		}
	} else if (1 == substatus)  {
		if (0 == sensor->back_dis) {
			substatus = 2;                     /*back left 30*/
			data->maxspeed = -50;
			data->x = -1;
		} else {                        /*no change, back right 30 */
			data->maxspeed = -50;
			data->x = 1;
		}
	} else if (2 == substatus)  {
		if (sensor->back_dis < 0.05) {
			substatus = 3;                     /*forward right 20*/
			data->maxspeed = 50;
			data->x = 2/3;
		} else {                           /*no change back left 30*/
			data->maxspeed = -50;
			data->x = -1;
		}
	} else if (3 == substatus) {
		if (sensor->back_dis > 0.2) {             /*back left 20*/
			substatus = 4;
			data->maxspeed = -50;
			data->x = -2/3;
		} else {                      /*no change forward right 20*/
			data->maxspeed = 50;
			data->x = 2/3;
		}
	} else if (4 == substatus) {
		if (sensor->back_dis > 0.2) {            /*forward right 10*/
			substatus = 5;
			data->maxspeed = 50;
			data->x = 1/3;
		} else {                         /*no change back left 20*/
			data->maxspeed = -50;
			data->x = -1/3;
		}
	} else if (5 == substatus) {
		if (sensor->back_dis > 0.12) {
			data->maxspeed = 0;
			data->x = 0;
			/* blink the light*/
			return 2;  /*park is finished*/
		} else {                      /*no change forward right 10*/
			data->maxspeed = 50;
			data->x = 1/3;
		}
	}
	return 1;
}

