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
		data.state = PARKING;
	}

	/*
	 * CLASSES FROM THE KI
	 */
	/* struct from image processing */
	struct from_image img;
	/* to see if the values have changed */
	int old_img_counter = -1;
	int time_counter_bv = 0;
	/*speed and steering */
	SpeedSteering s_s;
	s_s.car = car;
	s_s.data = &data;
	/* class for parking */
	/* TODO initialised with constructor
	   Park park(&tele, &s_s);*/
	Park park;
	park.tele = &tele;
	park.s_s = &s_s;
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
	for (;;) {
		/* modus 0 = parking */
		while (data.modus == 0) {
			/* TODO ??? for what */
			if (ret < 0) {
				return EXIT_FAILURE;
			}
			/* get the telemetry of the car */
			ret = carolo_getTelemetry(car, &tele);
			if (ret < 0) {
				return EXIT_FAILURE;
			}
			/* check if the remote control is activ */
			if (tele.status & SHM_TELEMETRIE_STATUS_RC) {
				data.state = FOLLOW_ROAD;
				usleep(5000);
			} else {
				/* read the data of the Image Proccessing out of the shm */
				ret = -1;
				while (ret == -1) {
					/* make sure the steering is correct */
					assert(data.steering < 1000);
					assert(data.steering > 0);
					/* set speed and steering */
					/* TODO ??? calling the control already done in reg*/
					carolo_control(car, data.speed, data.steering);
					/* read data from the bv */
					ret =
						proc_shm_read(procShm,
						              SHM_BV_ACCESS,
						              sizeof(struct from_image),
						              &img);
					/* if incorrect data wait and ask for new */
					if (ret < 0) {
						time_counter_bv += 1;
						usleep(5000);
					}
				}
				/* TODO ??? is it necessary to call startbox */
				/* only do something if there is a new image */
				if (img.counter != old_img_counter) {
					switch (data.state) {
						case STARTBOX:
							data.state = startbox.run();
							break;
						case PARKING:
							/*EXIT_SUCCESS park finished, EXIT_FAILURE something wrong, should stop
							 2 stands for going on*/
							park.controlling();
							data.modus = 3;
							break;
						default:
							printf("Invalid state. state is set to parking\n");
							data.state = PARKING;
							break;
					}
					time_counter_bv = 0;
				} else {/* if bv takes to long slow down */
					if (time_counter_bv > 10) {
						data.speed = 0.3;
					}
					if (time_counter_bv > 15) {
						data.speed = 0.0;
					}
					usleep(5000);
					time_counter_bv += 1;
				}
				/* write data for logging in the shm */
				/*TODO should be proc_shm_write()???*/
				data.counter++;
				ret =
					proc_shm_read(procShm,
					              SHM_KI_ACCESS,
					              sizeof(Data),
					              &data);
				printf("\r %f %d", data.speed, data.steering);
			}
		}
		/* modus 1 = without obsticles */
		while (data.modus == 1) {
			/* get the telemetry of the car */
			ret = carolo_getTelemetry(car, &tele);
			if (ret < 0) {
				return EXIT_FAILURE;
			}
			/* check if the remote control is activ */
			if (tele.status & SHM_TELEMETRIE_STATUS_RC) {
				data.state = FOLLOW_ROAD;
				usleep(10000);
			} else {
				/* read the data of the Image Proccessing out of the shm */
				ret = -1;
				assert(data.steering < 1000);
				assert(data.steering > 0);
				while (ret == -1) {
					/* set speed and steering */
					carolo_control(car, data.speed, data.steering);
					/* read data from the bv */
					ret =
						proc_shm_read(procShm,
						              SHM_BV_ACCESS,
						              sizeof(struct from_image),
						              &img);
					/* if incorrect data wait and ask for new */
					if (ret < 0) {
						time_counter_bv += 1;
						usleep(5000);
					}
				}
				/* only do something if there is a new image */
				if (img.counter != old_img_counter) {
					switch (data.state) {
						case STARTBOX:
							data.state = startbox.run();
							break;
						case FOLLOW_ROAD:
							data.state = followRoad(200, false);
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
				} else {/* if bv takes to long slow down */
					if (time_counter_bv > 10) {
						data.speed = 0.3;
					}
					if (time_counter_bv > 15) {
						data.speed = 0.0;
					}
					usleep(5000);
					time_counter_bv += 1;
				}
				/* write data for logging in the shm */
				data.counter++;
				ret =
					proc_shm_read(procShm,
					              SHM_KI_ACCESS,
					              sizeof(Data),
					              &data);
				printf("\r %f %d", data.speed, data.steering);
			}
		}
		/* modus 2 = with obsticles */
		while (data.modus == 2) {
			ret = carolo_setCTRL(car, ctrl);
			if (ret < 0) {
				return EXIT_FAILURE;
			}
			/* get the telemetry of the car */
			ret = carolo_getTelemetry(car, &tele);
			if (ret < 0) {
				return EXIT_FAILURE;
			}
			/* check if the remote control is activ */
			if (tele.status & SHM_TELEMETRIE_STATUS_RC) {
				data.state = FOLLOW_ROAD;
				usleep(10000);
			} else {
				/* make sure the steering is correct */
				assert(data.steering < 1000);
				assert(data.steering > 0);
				/* read the data of the Image Proccessing out of the shm */
				ret = -1;
				while (ret == -1) {
					/* set speed and steering */
					carolo_control(car, data.speed, data.steering);
					/* read data from the bv */
					ret =
						proc_shm_read(procShm,
						              SHM_BV_ACCESS,
						              sizeof(struct from_image),
						              &img);
					/* if incorrect data wait and ask for new */
					if (ret < 0) {
						usleep(5000);
						time_counter_bv += 1;
					}
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
				} else { /* if bv takes to long slow down */
					if (time_counter_bv > 10) {
						data.speed = 0.3;
					}
					if (time_counter_bv > 15) {
						data.speed = 0.0;
					}
					usleep(5);
					time_counter_bv += 1;
				}
				/* write data for logging in the shm */
				data.counter++;
				ret =
					proc_shm_read(procShm,
					              SHM_KI_ACCESS,
					              sizeof(Data),
					              &data);
				printf("\r %f %d", data.speed, data.steering);
			}
		}
		/* later wait if someone change the modus with a button press */
		while (data.modus == 3) {
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
	if (data.neural_network) {}
	carolo_deinit(car);
	proc_shm_destroy(procShm);
	return EXIT_SUCCESS;
}

