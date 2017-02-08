/* c includes */
#include <errno.h>
#include <stdio.h>
#include <argp.h> /* args */
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <assert.h>

/* System includes*/
#include "shm.h"
#include "procshm.h"
#include "carolo.h"

/* KI includes */
#include "main.h"
#include "speed_reg.h"
#include "park.h"
#include "discalc.h"
#include "intersection.h"
#include "startbox.h"
#include "overtake.h"

using  namespace ki;

struct ki_struct {
	struct carolo *carKI;
	struct proc_shm *procShmKI;
	struct carolo_telemetry teleKI;
	struct from_image imgKI;
	Data dataKI;
	uint32_t time_counter_bvKI;
};

int fetch_data(struct ki_struct &ki_s) {
	/* get the telemetry of the car */
	int ret;
	ret = carolo_getTelemetry(ki_s.carKI, &ki_s.teleKI);
	if (ret < 0) {
		return EXIT_FAILURE;
	}
	/* check if the remote control is activ */
	if (!(ki_s.teleKI.status & SHM_CONTROLL_CTRL_DRIVE)) {
		ki_s.dataKI.state = FOLLOW_ROAD;
		usleep(5000);
		return EXIT_FAILURE;
	} else {
		/* read the data of the Image Proccessing out of the shm */
		ret = -1;
		while (ret == -1) {
			/* set speed and steering */
			float speed = ki_s.dataKI.speed;
			int steering = ki_s.dataKI.steering;
			assert(steering > 0);
			assert(steering < 1000);
			carolo_control(ki_s.carKI, speed, steering);
			/* read data from the bv */
			ret =
				proc_shm_read(ki_s.procShmKI,
				              SHM_BV_ACCESS,
				              sizeof(struct from_image),
				              &ki_s.imgKI);
			/* if incorrect data wait and ask for new */
			if (ret < 0) {
				ki_s.time_counter_bvKI += 1;
				usleep(5000);
			}
		}
		/* only do something if there is a new image */
	}
	return 0;
}

int logging_KI(Data &data, struct proc_shm *procShm) {
	int ret;
	/* write data for logging in the shm */
	data.counter++;
	ret =
		proc_shm_write(procShm,
		               SHM_KI_ACCESS,
		               sizeof(Data),
		               &data);
	return ret;
}

void check_time(Data &data, uint32_t &time_counter_bv) {
	if (time_counter_bv > 10) {
		data.speed = 0.3;
	}
	if (time_counter_bv > 15) {
		data.speed = 0.0;
	}
	usleep(5000);
	time_counter_bv += 1;
}

/*
 * GLOBAL
 * For the argp
 */

/* Program documentation. */
static char doc[] = "main function for the ki";

/* A description of the arguments we accept. */
static char args_doc[] =
	"-d 0=Parking; 1=without obsticles; 2=with obsticles; 3=testing\n-n neural network on";

/* The options we understand. */
static struct argp_option options[] = {
	{"discipline", 'd', "<discipline>", 0, "select discipline"},
	{"neural", 'n', 0, 0, "neural on/off"}, {0}
};

/* Used by main to communicate with parse_opt. */
struct arguments {
	int discipline;
	bool neural = false;
};

/* Our argp parser. */
static error_t parse_opt (int key, char *arg, struct argp_state *state);
static struct argp argp = { options, parse_opt, args_doc, doc };

/**
 * For the argp
 * Parse a single option.
 */
static error_t parse_opt (int key, char *arg, struct argp_state *state) {
	/* Get the input argument from argp_parse, which we
	*  know is a pointer to our arguments structure. */
	struct arguments *arguments = (struct arguments *) state->input;
	switch (key) {
		case 'd':
			arguments->discipline = atoi(arg);
			break;
		case 'n':
			arguments->neural = true;
			break;
		default:
			return ARGP_ERR_UNKNOWN;
	}
	return 0;
}

/**
 *  main function KI
 *  decides what is to do
 *  -d (0=parking,1=without obsticles,2=with obsticles,3=testing) for discipline
 *  -n to activate the neural network
 */
int main(int argc, char *argv[]) {
	int32_t ret;
	/* arg parser */
	struct arguments arguments;
	arguments.discipline = 10;

	/*
	 * Parse our arguments;
	 * every option seen by parse_opt will be reflected in arguments.
	 */
	argp_parse(&argp, argc, argv, 0, 0, &arguments);
	/* struct for the telemetry daten*/
	struct carolo_telemetry tele;
	/* struct for the car needed to get telemetry*/
	struct carolo *car;
	car = carolo_init();
	if (!car) {
		printf("Can't open Carolo\n");
		return EXIT_FAILURE;
	}
	uint32_t ctrl;
	ctrl = SHM_CONTROLL_CTRL_DRIVE;
	ret = carolo_setCTRL(car, ctrl);
	if (ret < 0) {
		return EXIT_FAILURE;
	}
	/*shared memory to get struct from image processing */
	struct proc_shm *procShm = proc_shm_init();
	/* class for logging */
	Data data;
	/* get discipline + neuronal nework from argp_parse */
	data.modus = arguments.discipline;
	data.neural_network = arguments.neural;
	data.counter = 0;
	data.rc_modus = 0;
	/* check if modus has a correct value*/
	if (data.modus < 0 || data.modus > 3) {
		printf("Wrong arguments\nCall main_ki --help for more information");
		carolo_deinit(car);
		proc_shm_destroy(procShm);
		return -1;
	}
	/* set state to the correct starting value for the selected discipline*/
	printf("Modus %d \n", data.modus);
	if (data.modus != 0) {
		data.state = FOLLOW_ROAD;
	} else {
		data.state = LOOKING;
	}

	/*
	 * CLASSES FROM THE KI
	 */
	/* struct from image processing */
	struct from_image img;
	img.counter = 0;
	/* to see if the values have changed */
	int old_img_counter = -1;
	uint32_t time_counter_bv = 0;

	/*speed and steering */
	SpeedSteering s_s(data, car);

	/* class for parking */
	Park park(&tele,&s_s);
	/* follow road init */
	using std::placeholders::_1;
	using std::placeholders::_2;
	FollowRoad followRoad(std::bind(&SpeedSteering::reg, &s_s, _1, _2), img, data.modus);
	/* intersection init */
	ki::DisCalc discCalc;
	ki::Intersection intersection(&followRoad, &discCalc, &img, &tele, &data);
	/* class fo the startbox */
	ki::Startbox startbox(&followRoad, &discCalc, &img, &tele, &data);
	/* overtake */
	ki::Overtake overtake(&followRoad, &s_s, &tele, &data);
	/* neural network init */
	if (data.neural_network) {
		printf("neural network is active\n");
	}
	/* start car first time with speed 0 and steering 500 (straight) */
	data.speed = 0;
	data.steering = 500;
	carolo_control(car, data.speed, data.steering);

	/* wait 50 ms so it's not possible to change speed or steering befor the control saw it */
	usleep(50000);

	/*img.obstacles = false;
	 *  struct Line l;
	 *  l.start.x = -1;
	 *  l.start.y = 0;
	 *  l.end.x = 1;
	 *  l.end.y = 1;
	 *  struct Line r;
	 *  r.start.x = -0.5;
	 *  r.start.y = 0;
	 *  r.end.x = 1;
	 *  r.end.y = 1;
	 *  struct Lane la;
	 *  la.left = l;
	 *  la.right = r;
	 *  img.lane = la;
	 *  img.counter = 1;
	 *  struct Line h;
	 *  h.start.x = -1;
	 *  h.start.y = 0.2;
	 *  h.end.x = 1;
	 *  h.end.y = 0.2;
	 *  img.stop_line = h;*/


	struct ki_struct ki {car, procShm, tele, img, data, time_counter_bv};

	for (;;) {
		switch (data.modus) {
			/* modus 0 = parking */
			case 0:
				ret = fetch_data(ki);
				if (ret < 0) {
					printf("fetch_data failed\n");
					break;
				}
				switch (data.state) {
				    case LOOKING:
					    data.state = park.looking();
					    break;
      				case BACKING:
	    				data.state = park.backing();
		    			break;
			    	case PARK_FINISH:
				    	data.modus =3;
					    break;
    				default:
	    				printf("Invalid state. state is set to parking\n");
		    			data.state = LOOKING;
			    		break;
				}
				time_counter_bv = 0;
				ret = logging_KI(data, procShm);
				if (ret < 0) {
					printf("logging failed\n");
				}

				printf("\r %f %d", data.speed, data.steering);
				break;
			/* modus 1 = without obsticles */
			case 1:
				ret = fetch_data(ki);
				if (ret < 0) {
					printf("fetch_data failed\n");
					break;
				}
				if (img.counter != old_img_counter) {
					switch (data.state) {
						case STARTBOX:
							data.state = startbox.run();
							break;
						case FOLLOW_ROAD:
							data.state = followRoad(200, false);
							break;
						default:
							printf("Invalid state. state is set to follow road");
							data.state = FOLLOW_ROAD;
							break;
					}
					time_counter_bv = 0;
					/* write data for logging in the shm */
					ret = logging_KI(data, procShm);
					if (ret < 0) {
						printf("logging failed\n");
					}
				} else {     /* if bv takes to long slow down */
					check_time(data, time_counter_bv);
				}

				printf("\r %f %d", data.speed, data.steering);
				break;
			/* modus 2 = with obsticles */
			case 2:
				ret = fetch_data(ki);
				if (ret < 0) {
					printf("fetch_data failed\n");
					break;
				}
				/* only do something if there is a new image */
				if (img.counter != old_img_counter) {
					switch (data.state) {
						case STARTBOX:
							data.state = startbox.run();
							break;
						case FOLLOW_ROAD:
							data.state = followRoad(2000, false);
							break;
						case OVERTAKE:
							data.state = overtake.run();
							break;
						case INTERSECTION:
							intersection.reset();
							data.state = intersection.run();
							break;
						default:
							printf("Invalid state. state is set to follow road");
							data.state = FOLLOW_ROAD;
							break;
					}
					time_counter_bv = 0;
					/* write data for logging in the shm */
					ret = logging_KI(data, procShm);
					if (ret < 0) {
						printf("logging failed\n");
					}

				} else {     /* if bv takes to long slow down */
					check_time(data, time_counter_bv);
				}
				printf("\r %f %d", data.speed, data.steering);
				break;
			/* later wait if someone change the modus with a button press */
			default:
				carolo_control(car, data.speed, data.steering);
				if (ret < 0) {
					return EXIT_FAILURE;
				}
				printf("waiting\n");
				s_s.reg(0.0, 0.0);
				time_counter_bv = 0;
				usleep(10000);
		}
	}

	/* deinit but it will never be reached because for(;;) */
	carolo_deinit(car);
	proc_shm_destroy(procShm);
	return EXIT_SUCCESS;
}

