#ifndef PARK_H_
#define PARK_H_

#include "speed_reg.h"
#include "carolo.h"


namespace ki{

class Park {
	
 public:
	    Park(carolo_telemetry*,SpeedSteering*);		
		States looking();
		States backing();

 private:
		int substatus; /* status in finding or parking */
		float pre_distance; /*store the previous distance */
		carolo_telemetry *tele;
		SpeedSteering *s_s;  /*to call reg in speed_reg.h  */
		const float PARK_SPACE = 0.6; 
};
 
}

#endif




