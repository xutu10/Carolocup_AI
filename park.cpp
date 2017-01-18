#include "park.h"
#include <iostream>
using namespace std;

Park::Park() status(0),substatus(0) {
	tele = new carolo_telemetry();
	s_s = new SpeedSteering();
}

Park::~Park() {
	delete tele;
	delete s_s;
}

/**
 *  control the whole parking process, according to the status
 *  call the corresponding function
 *
 *  @return 1 to main, if the parking finished
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
 *  search a park lot whose length is greater than 600mm
 *  side_sensor, timer and rotation are needed
 *
 *  @return the current status back to controlling(),
 *  either 1 stands for finding(), or 2 for backing()
 */
int Park::finding() {

	double current_speed = 0;
	uint32_t time = 0;
	bool side_sensor = false;        // it should be in telemetry struct
	if (0 == substatus && true  == side_sensor) {
		substatus = 1;
	} else if (1 == substatus && false == side_sensor) {
		substatus = 2;
		pre_time = tele-> time;     /*start to timer */
	} else if (2 == substatus && true == side_sensor) {
		substatus = 1;
		pre_time = 0;    /*stop timer*/
	} else if (2 == substatus && false == side_sensor) {
		substatus = 3;
	} else if (3 == substatus ){
		current_speed = TIRE_DIAMETER * tele->speed[0] / 60;   /* m/s */
		time = tele->time - pre_time; /* how to handle the cyclye ??? */
		if (current_speed * time > PARKLOT_LENGTH) {
			pre_time = 0;                            /*stop timer*/
			substatus = 0;                           /*initial for backing*/
			return 2;                                /* to call backing() */
		}else{
			if(true == side_sensor){
				pre_time = 0;
				substatus = 1;
			}				
		}
	}
	return 1;
}

/**
 *  backing the car into the park lot,  disired speed  0.02m/s
 *  based on back_sensor to change the substatus
 *  pass the desired rotation and steering to system through *data
 *  data-> x is from -1 to 1, temporarily i set 1 = 30 degree,
 *  -1 = -30 which means turn left 30 degree
 *
 *  @return status to controlling(), if the backing finished
 */
int Park::backing() {
    float x = 0;
	int max_speed = 0;
	
	if (0 == substatus) {
		if (0 != tele->us[0]) {         /* back right 30 */
			substatus = 1;
			maxspeed = 20;
			x = 1;
		} else {                        /*initiallizing back right 30 */
			maxspeed = 20;
			x = 1;
		}
	} else if (1 == substatus && 0 == tele->us[0]) {     /*back left 30*/
			substatus = 2;                    
			maxspeed = 20;
			x = -1;
		} 
	} else if (2 == substatus && tele->us[0] < 0.05) {    /*forward right 20*/
			substatus = 3;                    
			maxspeed = 100;
			x = 2 / 3;
		}
	} else if (3 == substatus && tele->us[0] > 0.15 ) {            /*back left 20*/
			substatus = 4;
			maxspeed = 20;
			x = -2 / 3;
		}
	} else if (4 == substatus && tele->us[0]  < 0.05) {          /*forward right 10*/
			substatus = 5;
			maxspeed = 100;
			x = 1 / 3;
		}
	} else if (5 == substatus && tele->us[0] > 0.12) {
			s_s->reg(0,0);                 /*stop the car*/
			                               /* blink the light???*/
			return 3;                      /*park is finished*/
		}
	}
	s_s->reg(x,maxspeed);
	return 2;
}

