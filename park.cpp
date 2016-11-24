#include"park.hpp"
#include<iostream>
using namespace std;

#define PARKLOT_LENGTH 0.6  // 0.6m = 600 mm

// side_sensor == true means that something beside
bool Find::finding(bool side_sensor){
	
	if(0 == is_lot && false == side_sensor){
		is_lot = 1;
		// ??? start the timer
		return false;
	}
		
	else if(1 == is_lot && true  == side_sensor ){
		is_lot = 0;
		// ??? stop the timer		
		return false;
	}

	else if(1 == is_lot && false == side_sensor){
		current_rotation = 100;            // get from sensor
		current_speed = tire_diameter  * current_rotation/60;   // m/s
		if(current_speed * timer > PARKLOT_LENGTH){
			timer = 0;            // reset the timer
			is_lot = 2;
			set_rotation = -50;    // found a park lot, car backwards
		}
		return false;
	}
	// it's not sure when start to call parking()
	// must be tested with the car
	if(2 == is_lot && false == side_sensor){
		is_lot = 3;               //flag to call the parking()
		return true;
	}		
}
// negative steering is left
int Park::parking(){

	front_dis = 0.2;            // get from sensor
	back_dis = 0.3;
	//  desired speed 0.02m/s 
	if(0 == park_status){
		if( NULL == back_dis){
			set_rotation = -50;        // not sure
			set_steering = 3.14/6;      // turn right 30 degrees
		}else{
			park_status = 1;
			set_rotation = -50;
			set_steering = 3.14/6;    // turn left		
		}
		return 0;
	}

	else if(1 == park_status){
		if( NULL == back_dis){
			park_status = 2;
			set_rotation = -50;
			set_steering = -3.14/6;	
		}else{
			park_status = 1;
			set_rotation = -50;
			set_steering = 3.14/6;    // turn left		
		}
		return 0;
	}
	
	else if(2 == park_status){
		if(back_dis < 0.05){
			park_status = 3;
			set_rotation = 50;
			set_steering = 3.14/9;     // turn right 20 degrees		
		}else{
			set_rotation = -50;
			set_steering = -3.14/6;
		}
		return 0;
	}
	else if(3 == park_status ){
		if( back_dis > 0.2){
			park_status = 4;
		    set_rotation = -50;
			set_steering = -3.14/18;    // turn left 10 degrees backward		}else{
			set_rotation = 50;
			set_steering = 3.14/9;
		}			
		return 0;
	}
	else if(4 == park_status){
		if( back_dis < 0.05){
			park_status = 5;
			set_rotation = 50;
			set_steering = 3.14/18;
		}else{
			set_rotation = 50;
			set_steering = 3.14/9;
		}
		return 0;
	}
	else if(5 == park_status){
		if (back_dis > 0.12)
			return 1;          // parking finish
		else{
			set_rotation = 50;
			set_steering = 3.14/9;
		}
		return 0;
	}
}

