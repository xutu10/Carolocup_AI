#ifndef PARK_H_
#define PARK_H_

#include "speed_reg.h"
#include "carolo.h"


namespace ki{

class Park {
	
 public:
	    Park(carolo_telemetry*,SpeedSteering*);
		enum status{FINDING, BACKING, FINISHED};
		
 private:
		int controlling();
		int finding();
		int backing();

		int status; /*   flag to call the finding() or parking()
	                 *  1 stands for finding, 2 for parking, 3 for finish*/
		int substatus; /* status in finding or parking */
		float pre_distance; /*store the previous distance */
		carolo_telemetry *tele;
		SpeedSteering *s_s;  /*to call reg in speed_reg.h  */
		const float PARK_SPACE = 0.6; 
};
 
}

#endif




