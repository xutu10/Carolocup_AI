#pragma once

class Find{
 public:

	bool finding(bool side_sensor);
	
	int is_lot = 0;
	double current_rotation;
	double set_rotation = 0;
	const double tire_diameter = 0.05;  // 0.05m = 50mm
	double timer = 0;	//start the timer when potential is true
	double current_speed;
};

class Park{
 public:
	int parking();
   
	int park_status;
	double back_dis = 0;       // back sensor
	double set_rotation = 0; 
	double set_steering = 0;
	
};
