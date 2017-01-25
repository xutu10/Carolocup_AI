#pragma once
#include "speed_reg.h"

#define TIRE_DIAMETER  0.067    /*unit m */
#define PARKLOT_LENGTH 0.6  	/*unit m */

class Park {
	public:
		Park();
		~Park();
		int controlling();
		int finding();
		int backing();

		int status; /*   flag to call the finding() or parking()
	                 *  1 stands for finding, 2 for parking, 3 for finish*/

		int substatus; /* status in finding or parking */
		uint32_t pre_time; /*store the previous time */
		carolo_telemetry *tele;
		SpeedSteering *s_s;  /*to call reg in speed_reg.h  */
};

