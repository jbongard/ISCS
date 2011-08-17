
//#ifndef _CONSTANTS_H
//#define _CONSTANTS_H

// Global constants

int		RAND_SEED			= 2;

double		STEP_SIZE			= 0.05;

// Controls the speed vs. accuracy of the simulator:
// A larger step size makes the simulator to run faster,
// but less stably;
// A smaller step size causes the simulator to run slower,
// but more stably.


// Environment-related constants

int		MAX_ENVIRONMENTS		= 4;

int		MAX_OTHER_OBJECTS		= 10;

int		SELECTION_LEVEL_OBJECT		= 0;
int		SELECTION_LEVEL_ENVIRONMENT	= 1;
int		SELECTION_LEVEL_ENVS		= 2;

// Optimizer-related constants

int		STARTING_EVALUATION_TIME	= 500;
double		MUTATION_PROBABILITY		= 0.05;

int		EVALS_UNTIL_EVAL_TIME_EXTENDED	= 30;
int		EVAL_TIME_EXTENSION_PERIOD	= 10;

double		WORST_FITNESS			= -1000000.0;

int		MAX_EVALS_BEFORE_SAVING		= 100;

int		TIME_TO_CHECK_FOR_NEUTRAL_MUTATION = 50;

//	ALPS-related constants

int		ALPS_NUM_LAYERS			= 20;

int		ALPS_GENOMES_PER_LAYER		= 1;

// Constants related to viewing the simulation

int		MODE_VIEW_SIDE			= 0;
int		MODE_VIEW_TOP			= 1;
int		MODE_VIEW_BACK			= 2;

int		MODE_SIMULATE_DESIGN		= 0;
int		MODE_SIMULATE_EVOLVE		= 1;
int		MODE_SIMULATE_CHAMP		= 2;

double		MOVE_INCREMENT			= 0.1;

// Object constants

int		SHAPE_RECTANGLE			= 0;
int		SHAPE_CYLINDER			= 1;

int		OBJECT_STATE_INCORPOREAL	= 0;
int		OBJECT_STATE_SOLID		= 1;
int		OBJECT_STATE_PHYSICAL		= 2;

int		OBJECT_FACE_FRONT		= 0;
int		OBJECT_FACE_BACK		= 1;
int		OBJECT_FACE_LEFT		= 2;
int		OBJECT_FACE_RIGHT		= 3;
int		OBJECT_FACE_TOP			= 4;
int		OBJECT_FACE_BOTTOM		= 5;


// Light source constants

double		LIGHT_SOURCE_LENGTH		= 0.5;
double		LIGHT_SOURCE_DISTANCE		= 5.0;

double		MAX_LIGHT_SENSOR_DISTANCE	= 40;

// Robot-specific constants

int		MAX_ROBOTS			= 2;

int		ROBOT_STARFISH			= 0;
int		ROBOT_WHEEL			= 1;

// Robot starfish-specific constants

int		ROBOT_SNAKE			= 1;
int		ROBOT_QUADRUPED			= 2;

double		ROBOT_STARFISH_BODY_LENGTH	= 1.0;
double		ROBOT_STARFISH_BODY_WIDTH	= 1.0;
double		ROBOT_STARFISH_BODY_HEIGHT	= 0.1;
double		ROBOT_STARFISH_LEG_RADIUS	= 0.1;
double		ROBOT_STARFISH_LEG_LENGTH	= 1.0;


/*
double		ROBOT_STARFISH_MOTOR_STRENGTH	= 1.5;
double		ROBOT_STARFISH_MOTOR_SPEED	= 5.0;
*/
double		ROBOT_STARFISH_JOINT_RANGE	= 90.0;
double		ROBOT_STARFISH_MOTOR_STRENGTH	= 10.0;
double		ROBOT_STARFISH_MOTOR_SPEED	= 0.01;

// Robot wheel-specific constants

double		ROBOT_WHEEL_BODY_RADIUS		= 0.1;
double		ROBOT_WHEEL_LEG_LENGTH		= 0.5;
int 		ROBOT_WHEEL_LEG_SEGS		= 3;
int		ROBOT_WHEEL_LEGS		= 3;
double		ROBOT_WHEEL_LEG_RADIUS		= ROBOT_WHEEL_BODY_RADIUS/2.0;

double		ROBOT_WHEEL_FOOT_LENGTH		= ROBOT_WHEEL_LEG_LENGTH;
double		ROBOT_WHEEL_FOOT_WIDTH		= ROBOT_WHEEL_BODY_RADIUS*8.0;
double		ROBOT_WHEEL_FOOT_HEIGHT		= 4.0*ROBOT_WHEEL_LEG_RADIUS;

// Controller-specific constants

int		NODES_PER_MOTOR			= 4;

// Movie-specific constants

//int		TIME_STEPS_PER_FRAME		= 4;
int		TIME_STEPS_PER_FRAME		= 0;

//#endif
