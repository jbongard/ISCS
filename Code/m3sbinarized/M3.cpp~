#include "stdlib.h"
#include <ode/ode.h>
#include <drawstuff/drawstuff.h>

#include "envs.h"

ENVS *envs;

static dWorldID world;
static dSpaceID space;
static dJointGroupID contactgroup;
static dGeomID ground;

extern int	RAND_SEED;
extern double	STEP_SIZE;
extern double 	MOVE_INCREMENT;
extern int	MODE_SIMULATE_DESIGN;
extern int	MODE_SIMULATE_EVOLVE;
extern int	MODE_SIMULATE_CHAMP;

extern int      OBJECT_FACE_FRONT;
extern int      OBJECT_FACE_BACK;
extern int      OBJECT_FACE_LEFT;
extern int      OBJECT_FACE_RIGHT;
extern int      OBJECT_FACE_TOP;
extern int      OBJECT_FACE_BOTTOM;

dsFunctions 	Simulator_Create(void);
void 		Simulator_Destroy(void);

int		showGraphics;

int Geoms_Can_Interpenetrate(dGeomID o1, dGeomID o2) {

	int object1IsPartOfRobot = false;

	if ( o1!=ground ) {
		OBJECT *obj1 = (OBJECT *)dGeomGetData(o1);
		object1IsPartOfRobot = obj1->Is_Part_Of_Robot();
	}

	int object2IsPartOfRobot = false;

	if ( o2!=ground ) {
		OBJECT *obj2 = (OBJECT *)dGeomGetData(o2);
		object2IsPartOfRobot = obj2->Is_Part_Of_Robot();
	}

	int bothArePartOfRobot =	object1IsPartOfRobot && 					object2IsPartOfRobot;
	// Robot body parts can interpenetrate

	int bothArePartOfEnvironment =	(!object1IsPartOfRobot) &&
					(!object2IsPartOfRobot);
	// Don't bother computing contacts between different
	// parts of the environment, because they do not move.

	return( bothArePartOfRobot || bothArePartOfEnvironment );
}

void Set_Touch_Sensor(dGeomID o) {

	OBJECT* obj = (OBJECT *)dGeomGetData(o);

	if ( obj )

		obj->Sensor_Touch_Set();
}

void Resolve_Contact(dGeomID o1, dGeomID o2) {

	int i,n;

	const int N = 10;
	dContact contact[N];
	n = dCollide (o1,o2,N,&contact[0].geom,sizeof(dContact));

	// Two objects that shouldn't interpenetrate have
	// collided with one another.
	if (n > 0) {

		Set_Touch_Sensor(o1);
		Set_Touch_Sensor(o2);

		for (i=0; i<n; i++) {

		contact[i].surface.slip1 = 0.01;
		contact[i].surface.slip2 = 0.01;
		contact[i].surface.mode = dContactSoftERP |
			dContactSoftCFM |
			dContactApprox1 |
			dContactSlip1 | dContactSlip2;
		contact[i].surface.mu = 50.0;

                double kp = 100.0/STEP_SIZE;
                double kd = 0.0;
                contact[i].surface.soft_erp = 	STEP_SIZE*kp/
						(STEP_SIZE*kp+kd);
                contact[i].surface.soft_cfm = 	1.0/(STEP_SIZE*kp+kd);

		dJointID c = dJointCreateContact (world,
						contactgroup,
						&contact[i]);
		dJointAttach (c,
		dGeomGetBody(contact[i].geom.g1),
		dGeomGetBody(contact[i].geom.g2));
		}
	}
}

static void nearCallback (void *data, dGeomID o1, dGeomID o2)
{

	if ( Geoms_Can_Interpenetrate(o1,o2) )
		return;

	Resolve_Contact(o1,o2);
}

static void start()
{
  dAllocateODEDataForThread(dAllocateMaskAll);

  static float xyz[3] = {0.8317f,-0.9817f,0.8000f};
  static float hpr[3] = {121.0000f,-27.5000f,0.0000f};
  dsSetViewpoint (xyz,hpr);
/*
  printf ("Press:\t'a' to increase speed.\n"
	  "\t'z' to decrease speed.\n"
	  "\t',' to steer left.\n"
	  "\t'.' to steer right.\n"
	  "\t' ' to reset speed and steering.\n"
	  "\t'1' to save the current state to 'state.dif'.\n");
*/
	envs->Mode_View_Set_Back();
}

void Simulator_Initialize(void) {

	dInitODE2(0);
	world = dWorldCreate();
	space = dHashSpaceCreate (0);
	contactgroup = dJointGroupCreate (0);
	dWorldSetGravity (world,0,0,-0.5);
	ground = dCreatePlane (space,0,0,1,0);
}

void Simulator_Destroy(void) {

	dGeomDestroy(ground);
	dJointGroupDestroy (contactgroup);
	dSpaceDestroy (space);
	dWorldDestroy (world);
	dCloseODE();
}

void Simulate(int pause) {

	if ( showGraphics )
		envs->Draw();

	if ( envs->recordingVideo )

		envs->Video_Record();

	if ( !pause ) {

		dSpaceCollide (space,0,&nearCallback);
		dWorldStep (world,STEP_SIZE);
		dJointGroupEmpty(contactgroup);

		if ( envs->In_Evolution_Mode() )

			envs->Evolve(		world,
						space);

		else if ( envs->In_Champ_Mode() )

			envs->Show_Champ(	world, 
						space);

	}
}

static void simLoop (int pause)
{

	Simulate(pause);
}

void Change_EvaluationPeriod(char cmd) {

	     if ( cmd=='t' )
		envs->EvaluationPeriod_Decrease(world,space,contactgroup);

	else if ( cmd=='T' )
		envs->EvaluationPeriod_Increase(world,space,contactgroup);
}

void Change_MutationProbability(char cmd) {

	     if ( cmd=='m' )
		envs->MutationProbability_Decrease();

	else if ( cmd=='M' )
		envs->MutationProbability_Increase();
}

void Change_Selection_Level(char cmd) {

	     if ( cmd=='-' )
		envs->Selection_Level_Raise();

	else if ( cmd=='+' )
		envs->Selection_Level_Lower();
}

int Command_To_Change_EvaluationPeriod(char cmd) {

	return(	(cmd=='t') ||
		(cmd=='T') );
}

int Command_To_Change_MutationProbability(char cmd) {

	return(	(cmd=='m') ||
		(cmd=='M') );
}

int Command_To_Change_Selection_Level(char cmd) {

	return(	(cmd=='-') ||
		(cmd=='+') );
}

int Command_To_Load_Or_Save_Environment(char cmd) {

	return(	(cmd=='f') ||	// Load
		(cmd=='F') );	// Save
}

int Command_To_Move_Rapidly(char cmd) {

	return(	(cmd=='W') ||
		(cmd=='A') ||
		(cmd=='S') ||
		(cmd=='D') ||
		(cmd=='E') ||
		(cmd=='Z') );
}

int Command_To_Move_Slowly(char cmd) {

	return(	(cmd=='w') ||
		(cmd=='a') ||
		(cmd=='s') ||
		(cmd=='d') ||
		(cmd=='e') ||
		(cmd=='z') );
}

int Command_To_Resize_Object(char cmd) {

	return(	(cmd=='j') ||
		(cmd=='J') ||
		(cmd=='k') ||
		(cmd=='K') ||
		(cmd=='l') ||
		(cmd=='L') );
}

int Command_To_Switch_ActiveElement(char cmd) {

	return(	(cmd=='[') ||
		(cmd==']') );
}

int Command_To_Switch_Mode(char cmd) {

	return(	(cmd=='v') ||
		(cmd=='g') ||
		(cmd=='h') );
}

int Command_To_Switch_View(char cmd) {

	return(	(cmd=='y') ||
		(cmd=='o') ||
		(cmd=='b') );
}

void Load_Or_Save_Environment(char cmd) {

	     if (cmd=='f') // Load from a file.
		envs->Active_Element_Load();

	else if (cmd=='F') // Save to a file.
		envs->Active_Element_Save();
}

void Move_Rapidly(char cmd) {

	     if (cmd=='W') // Move forward
		envs->Active_Element_Move(0,10*+MOVE_INCREMENT,0);

	else if (cmd=='A') // Move left
		envs->Active_Element_Move(10*-MOVE_INCREMENT,0,0);

	else if (cmd=='S') // Move backward
		envs->Active_Element_Move(0,10*-MOVE_INCREMENT,0);

	else if (cmd=='D') // Move right
		envs->Active_Element_Move(10*+MOVE_INCREMENT,0,0);

	else if (cmd=='E') // Move up
		envs->Active_Element_Move(0,0,10*+MOVE_INCREMENT);

	else if (cmd=='Z') // Move down
		envs->Active_Element_Move(0,0,10*-MOVE_INCREMENT);
}

void Move_Slowly(char cmd) {

	     if (cmd=='w') // Move forward
		envs->Active_Element_Move(0,1*+MOVE_INCREMENT,0);

	else if (cmd=='a') // Move left
		envs->Active_Element_Move(1*-MOVE_INCREMENT,0,0);

	else if (cmd=='s') // Move backward
		envs->Active_Element_Move(0,1*-MOVE_INCREMENT,0);

	else if (cmd=='d') // Move right
		envs->Active_Element_Move(1*+MOVE_INCREMENT,0,0);

	else if (cmd=='e') // Move up
		envs->Active_Element_Move(0,0,1*+MOVE_INCREMENT);

	else if (cmd=='z') // Move down
		envs->Active_Element_Move(0,0,1*-MOVE_INCREMENT);
}

void Resize_Object(char cmd) {

	     if (cmd=='J') // Enlarge the first size dimension.
		envs->Active_Element_Resize(MOVE_INCREMENT,0,0);

	else if (cmd=='j') // Shrink the first size dimension.
		envs->Active_Element_Resize(-MOVE_INCREMENT,0,0);


	else if (cmd=='K') // Enlarge the second size dimension.
		envs->Active_Element_Resize(0,MOVE_INCREMENT,0);

	else if (cmd=='k') // Shrink the second size dimension.
		envs->Active_Element_Resize(0,-MOVE_INCREMENT,0);


	else if (cmd=='L') // Enlarge the third size dimension.
		envs->Active_Element_Resize(0,0,MOVE_INCREMENT);

	else if (cmd=='l') // Shrink the third size dimension.
		envs->Active_Element_Resize(0,0,-MOVE_INCREMENT);
}

void Switch_ActiveElement(char cmd) {

	     if (cmd=='[')
		envs->Active_Element_Previous();

	else if (cmd==']')
		envs->Active_Element_Next();
}

void Switch_Mode(char cmd) {

	// Switch to evol[v]e mode.
	if ( cmd=='v' )
		envs->Mode_Simulate_Set_Evolve(world,space);

	// Switch to desi[g]n mode.
	else if ( cmd=='g' )
		envs->Mode_Simulate_Set_Design();

	// Switch to c[h]amp mode.
	else if ( cmd=='h' )
		envs->Mode_Simulate_Set_Champ();
}

void Switch_View(char cmd) {

	// Set the s[y]de view
	if ( cmd=='y' )
		envs->Mode_View_Set_Side();

	// Set the t[o]p view
	else if ( cmd=='o' )
		envs->Mode_View_Set_Top();

	// Set the [b]ack view
	else if ( cmd=='b' )
		envs->Mode_View_Set_Back();
}

static void command (int cmd)
{

	     if ( Command_To_Switch_Mode(cmd) )
		Switch_Mode(cmd);

	else if ( Command_To_Switch_View(cmd) )
		Switch_View(cmd);

	else if ( Command_To_Move_Slowly(cmd) )
		Move_Slowly(cmd);

	else if ( Command_To_Move_Rapidly(cmd) )
		Move_Rapidly(cmd);

	else if ( Command_To_Switch_ActiveElement(cmd) )
		Switch_ActiveElement(cmd);

	else if ( cmd=='c' )
		envs->Active_Element_Copy();

	else if ( cmd=='x' )
		envs->Active_Element_Delete();

	else if ( Command_To_Change_MutationProbability(cmd) )
		Change_MutationProbability(cmd);

	else if ( Command_To_Change_EvaluationPeriod(cmd) )
		Change_EvaluationPeriod(cmd);

	else if ( cmd=='i' ) // Start or stop recording a mov[i]e
		envs->Video_Start_Stop();

	else if ( Command_To_Load_Or_Save_Environment(cmd) )
		Load_Or_Save_Environment(cmd);

	else if ( Command_To_Resize_Object(cmd) )
		Resize_Object(cmd);

	else if ( Command_To_Change_Selection_Level(cmd) )
		Change_Selection_Level(cmd);
	
}

dsFunctions Simulator_Create(void) {

	dsFunctions fn;
	fn.version = DS_VERSION;
	fn.start = &start;
	fn.step = &simLoop;
	fn.command = &command;
	fn.stop = 0;
	fn.path_to_textures = (char *)"textures";

	Simulator_Initialize();

	return( fn );
}

void Parse_Parameters(int argc, char **argv) {

        int currParam;

        for(currParam=0;currParam<argc;currParam++) {

//                if ( strcmp(argv[currParam],"-r") == 0 )
//                        randSeed = atoi(argv[currParam+1]);

		if ( strcmp(argv[currParam],"-null") == 0 )
			showGraphics = false;	
	}
}

int main (int argc, char **argv)
{

	srand(RAND_SEED);

	showGraphics = true;

	Parse_Parameters(argc,argv);

	dsFunctions fn = Simulator_Create();

	envs = new ENVS();

	if ( showGraphics )
		dsSimulationLoop (argc,argv,352,288,&fn);
	else {
		envs->Prepare_To_Run_Without_Graphics(world,space);
		while ( 1 )
			Simulate(false);
	}

	delete envs;
	envs = NULL;

  	return 0;
}
