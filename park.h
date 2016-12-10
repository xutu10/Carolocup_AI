#pragma once
#include "main.h"

#define TIRE_DIAMETER  0.05      /* 0.05m = 50mm */
#define PARKLOT_LENGTH 0.6  /* 0.6m = 600 mm */

class Sensor {
public:
double current_rotation;
bool side_sensor;     /* side_sensor == true means something beside */

double back_dis;
double set_rotation;
double set_steering;     /* negative steering is left */
};

class Park {
public:
Park();
~Park();
int controlling();
int finding();
int backing();

int status;     /*   flag to call the finding() or parking()
				  1 stands for finding, 2 for parking, 3 for finish*/

int substatus;       /* status in finding or parking */
Sensor *sensor;
Data *data;          /*get from main.h, pass the speed and steering
					   what I want to set into it */        
};

