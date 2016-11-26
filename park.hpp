#pragma once

#define TIRE_DIAMETER  0.05      /* 0.05m = 50mm */
#define PARKLOT_LENGTH 0.6  /* 0.6m = 600 mm */

/*??? sensor,rotation,steering as parameter to pass or as class member  */
/*how to set the timer and blink */
/*??? the value of back_sensor when too far awary from something back */

class Sensor{
public:
	double current_rotation;
	bool side_sensor; /* side_sensor == true means something beside */

	double back_dis;
	double set_rotation;
	double set_steering; /* negative steering is left */
};

class Park {
public:
	Park();
	~Park();
	int controlling();
	int finding();
	int backing();
	 /* flag to call the finding() or parking()
		1 stands for finding, 2 for parking, 3 for finish  */
	int status;  
	int substatus;   /* status in finding or parking */
	Sensor* sensor; 
};

