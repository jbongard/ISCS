#include "stdio.h"
#ifndef _ROBOT_CPP
#define _ROBOT_CPP

#include "robot.h"
#include "matrix.h"

extern int 	ROBOT_STARFISH;

extern int	SHAPE_CYLINDER;
extern int 	SHAPE_RECTANGLE;

extern double	ROBOT_STARFISH_BODY_LENGTH;
extern double	ROBOT_STARFISH_BODY_WIDTH;
extern double	ROBOT_STARFISH_BODY_HEIGHT;
extern double	ROBOT_STARFISH_LEG_RADIUS;
extern double   ROBOT_STARFISH_LEG_LENGTH;
extern double	ROBOT_STARFISH_JOINT_RANGE;

extern double	WORST_FITNESS;

ROBOT::ROBOT(ENVIRONMENT *cE, int robotType) {

	containerEnvironment = cE;

	if ( robotType == ROBOT_STARFISH )

		Create_Starfish();

	for (int i=0;	i<numObjects;	i++)
		if ( objects[i] )
			objects[i]->containerRobot = this;

	for (int j=0;	j<numJoints;	j++)
		if ( joints[j] )
			joints[j]->containerRobot = this;

	hidden = false;
	physicalized = false;

	neuralNetwork = NULL;

	sensorDifferences = 0.0;
}

ROBOT::ROBOT(ROBOT *other) {

	containerEnvironment = other->containerEnvironment;

	Initialize(other);
}

ROBOT::ROBOT(ENVIRONMENT *cE, ROBOT *other) {

	containerEnvironment = cE;

	Initialize(other);
}

ROBOT::ROBOT(ENVIRONMENT *cE, ifstream *inFile) {

	containerEnvironment = cE;

	Initialize(inFile);
}

ROBOT::~ROBOT(void) {

	if ( containerEnvironment )
		containerEnvironment = NULL;

	if ( objects ) {
		for (int i=0;	i<numObjects;	i++) {
			if ( objects[i] ) {
				delete objects[i];
				objects[i] = NULL;
			}
		}
		delete[] objects;
		objects = NULL;
	}

	if ( joints ) {
		for (int j=0;	j<numJoints;	j++) {
			if ( joints[j] ) {
				delete joints[j];
				joints[j] = NULL;
			}
		}
		delete[] joints;
		joints = NULL;
	}

	if ( neuralNetwork ) {

		delete neuralNetwork;
		neuralNetwork = NULL;
	}
}

void ROBOT::Activate(void) {

	for (int i=0;	i<numObjects;	i++)

		if ( objects[i] )
			objects[i]->Activate();
}

void ROBOT::Deactivate(void) {

	for (int i=0;	i<numObjects;	i++)

		if ( objects[i] )
			objects[i]->Deactivate();
}

void ROBOT::Draw(void) {

	if ( hidden )
		return;

	for (int i=0;	i<numObjects;	i++)

		if ( objects[i] )
			objects[i]->Draw();
}

double ROBOT::Fitness_Get(ROBOT *targetRobot) {

/*
//	if ( neuralNetwork->inAFixedAttractor )
//		return( WORST_FITNESS );

	double fitness = 0.0;

	double diff = Sensors_Get_Total_Differences(targetRobot);

	//double diff = Sensors_Get_Largest_Difference(targetRobot);

	fitness = -diff;
	
//	fitness = Sensors_Get_Total();

	return( fitness );
*/

	return( -sensorDifferences );
}

int  ROBOT::Has_Stopped(void) {

	if ( !neuralNetwork )
		return( true );

	return( neuralNetwork->inAFixedAttractor );
}

void ROBOT::Hide(void) {

	hidden = true;

	for (int i=0;	i<numObjects;	i++)

		if ( objects[i] )
			objects[i]->Hide();
}

int  ROBOT::In_Simulator(void) {

	return( physicalized );
}

void ROBOT::Label(NEURAL_NETWORK *genome, int environmentIndex) {

	if ( neuralNetwork )
		delete neuralNetwork;

	neuralNetwork = new NEURAL_NETWORK(genome);
}

void ROBOT::Make_Incorporeal(void) {

	for (int i=0;	i<numObjects;	i++)

		if ( objects[i] )
			objects[i]->Make_Incorporeal();

	for (int j=0;	j<numJoints;	j++)

		if ( joints[j] )
			joints[j]->Make_Incorporeal();

	physicalized = false;
}

void ROBOT::Make_Physical(dWorldID world, dSpaceID space) {

	// Add the robot to the physical simulator.

	if ( physicalized )
		return;

	physicalized = true;

	for (int i=0;	i<numObjects;	i++)

		if ( objects[i] )
			objects[i]->Make_Physical(world,space);

	for (int j=0;	j<numJoints;	j++)

		if ( joints[j] )
			joints[j]->Make_Physical(world);
}

int  ROBOT::Motors_Number_Of(void) {

	return( numJoints );
	// Assumes all joints are motorized.
}

void ROBOT::Move(int timeStep) {

	// The robot is moving itself.

	// The robot cannot move itself it is not physical.
	if ( !physicalized )
		return;

//	if ( timeStep==0 )
		Neural_Network_Set_Sensors();

	Neural_Network_Update(timeStep);

	Actuate_Motors();

	Sensors_Touch_Clear();
}

void ROBOT::Move(double x, double y, double z) {

	// The robot is being moved by the user.

	for (int i=0;	i<numObjects;	i++)

		if ( objects[i] )
			objects[i]->Move(x,y,z);

	for (int j=0;	j<numJoints;	j++)

		if ( joints[j] )
			joints[j]->Move(x,y,z);
}

void ROBOT::Save(ofstream *outFile) {

	(*outFile) << numObjects << "\n";

	for (int i=0;	i<numObjects;	i++)

		if ( objects[i] )
			objects[i]->Save(outFile);

	(*outFile) << numJoints << "\n";

	for (int j=0;	j<numJoints;	j++)

		if ( joints[j] )
			joints[j]->Save(outFile);
}

double ROBOT::Sensor_Sum(void) {

	return( Sensors_Get_Total() );
}

void ROBOT::Sensors_Add_Difference(ROBOT *other) {

	sensorDifferences = sensorDifferences + 

				Sensors_In_Objects_Total_Differences(other);
}

int ROBOT::Sensors_Number_Of(void) {

	int numSensors = 0;

	for (int i=0;	i<numObjects;	i++)

		if ( objects[i] )
			numSensors = 	numSensors + 
					objects[i]->Sensors_Number_Of();


	for (int j=0;	j<numJoints;	j++)

		if ( joints[j] )
			numSensors = 	numSensors + 
					joints[j]->Sensors_Number_Of();

	return( numSensors );
}

void ROBOT::Sensors_Update(void) {

	// Light sensors already updated during the
	// last drawing of the robot.
	for (int i=0;	i<numObjects;	i++)

		if ( objects[i] )
			objects[i]->Sensors_Update();

	// Touch sensors updated by nearCallback function.

	// Update all of the proprioceptive sensors.
	for (int j=0;	j<numJoints;	j++)

		if ( joints[j] )
			joints[j]->Sensors_Update();
}

void ROBOT::Sensors_Write(void) {

	Sensors_In_Objects_Write();

	Sensors_In_Joints_Write();

	printf("\n");
}

void ROBOT::Set_Color(double r, double g, double b) {

	for (int i=0;	i<numObjects;	i++)

		if ( objects[i] )
			objects[i]->Set_Color(r,g,b);
}

void ROBOT::Unhide(void) {

	hidden = false;

	for (int i=0;	i<numObjects;	i++)

		if ( objects[i] )
			objects[i]->Unhide();
}

// --------------------- Private methods ------------------------

void ROBOT::Actuate_Motors(void) {

	for (int j=0;	j<numJoints;	j++) {

		double motorNeuronValue = 

		neuralNetwork->Get_Motor_Neuron_Value(j);

		if ( joints[j] )
			joints[j]->Move(motorNeuronValue);
	}
}

void ROBOT::Create_Starfish(void) {

	Create_Starfish_Objects();

	Create_Starfish_Joints();
}

void ROBOT::Create_Starfish_Joints(void) {

	// Four joints connecting each lower and upper leg, and
	// four joints connecting each leg to the main body.
	numJoints = 4 + 4;

	joints = new JOINT * [numJoints];

	for (int j=0;	j<numJoints;	j++)
		joints[j] = NULL;

	// Attach the left upper and lower legs.
	joints[0] = new JOINT(this,1,5,
				-ROBOT_STARFISH_BODY_LENGTH/2.0
				-ROBOT_STARFISH_LEG_LENGTH,
				0,
				ROBOT_STARFISH_LEG_LENGTH
				+ROBOT_STARFISH_LEG_RADIUS,
				0,1,0,
				-ROBOT_STARFISH_JOINT_RANGE,+ROBOT_STARFISH_JOINT_RANGE);

	// Attach the right upper and lower legs.
	joints[1] = new JOINT(this,2,6,
				+ROBOT_STARFISH_BODY_LENGTH/2.0
				+ROBOT_STARFISH_LEG_LENGTH,
				0,
				ROBOT_STARFISH_LEG_LENGTH
				+ROBOT_STARFISH_LEG_RADIUS,
				0,-1,0,
				-ROBOT_STARFISH_JOINT_RANGE,+ROBOT_STARFISH_JOINT_RANGE);

	// Attach the forward upper and lower legs.
	joints[2] = new JOINT(this,3,7,
				0,
				+ROBOT_STARFISH_BODY_LENGTH/2.0
				+ROBOT_STARFISH_LEG_LENGTH,
				ROBOT_STARFISH_LEG_LENGTH
				+ROBOT_STARFISH_LEG_RADIUS,
				1,0,0,
				-ROBOT_STARFISH_JOINT_RANGE,+ROBOT_STARFISH_JOINT_RANGE);

	// Attach the back upper and lower legs.
	joints[3] = new JOINT(this,4,8,
				0,
				-ROBOT_STARFISH_BODY_LENGTH/2.0
				-ROBOT_STARFISH_LEG_LENGTH,
				ROBOT_STARFISH_LEG_LENGTH
				+ROBOT_STARFISH_LEG_RADIUS,
				-1,0,0,
				-ROBOT_STARFISH_JOINT_RANGE,+ROBOT_STARFISH_JOINT_RANGE);

	// Attach main body and the left upper leg.
	joints[4] = new JOINT(this,0,1,
				-ROBOT_STARFISH_BODY_LENGTH/2.0,
				0,
				ROBOT_STARFISH_LEG_LENGTH
				+ROBOT_STARFISH_LEG_RADIUS,
				0,1,0,
				-ROBOT_STARFISH_JOINT_RANGE,+ROBOT_STARFISH_JOINT_RANGE);

	// Attach main body and the right upper leg.
	joints[5] = new JOINT(this,0,2,
				+ROBOT_STARFISH_BODY_LENGTH/2.0,
				0,
				ROBOT_STARFISH_LEG_LENGTH
				+ROBOT_STARFISH_LEG_RADIUS,
				0,-1,0,
				-ROBOT_STARFISH_JOINT_RANGE,+ROBOT_STARFISH_JOINT_RANGE);

	// Attach main body and the forward upper leg.
	joints[6] = new JOINT(this,0,3,
				0,
				+ROBOT_STARFISH_BODY_LENGTH/2.0,
				ROBOT_STARFISH_LEG_LENGTH
				+ROBOT_STARFISH_LEG_RADIUS,
				1,0,0,
				-ROBOT_STARFISH_JOINT_RANGE,+ROBOT_STARFISH_JOINT_RANGE);

	// Attach main body and the back upper leg.
	joints[7] = new JOINT(this,0,4,
				0,
				-ROBOT_STARFISH_BODY_LENGTH/2.0,
				ROBOT_STARFISH_LEG_LENGTH
				+ROBOT_STARFISH_LEG_RADIUS,
				-1,0,0,
				-ROBOT_STARFISH_JOINT_RANGE,+ROBOT_STARFISH_JOINT_RANGE);

	for (int j=0;j<numJoints;j++)

		joints[j]->Sensor_Proprioceptive_Add();
}

void ROBOT::Create_Starfish_Objects(void) {

	// One main body, four upper legs and four lower legs
	numObjects = 1 + 4 + 4;

	objects = new OBJECT * [numObjects];

	for (int i=0;	i<numObjects;	i++)
		objects[i] = NULL;

	// Main body
  	objects[0] = new OBJECT(SHAPE_RECTANGLE, 
				ROBOT_STARFISH_BODY_LENGTH,
				ROBOT_STARFISH_BODY_WIDTH,
				ROBOT_STARFISH_BODY_HEIGHT,
				0,
				0,
				ROBOT_STARFISH_LEG_LENGTH
				+ROBOT_STARFISH_LEG_RADIUS,
				0,0,1);
	
	// Left upper leg
  	objects[1] = new OBJECT(SHAPE_CYLINDER, 
				ROBOT_STARFISH_LEG_RADIUS,
				ROBOT_STARFISH_LEG_LENGTH,
				-ROBOT_STARFISH_BODY_LENGTH/2.0
				-ROBOT_STARFISH_LEG_LENGTH/2.0,
				0,
				ROBOT_STARFISH_LEG_LENGTH
				+ROBOT_STARFISH_LEG_RADIUS,
				-1,0,0);

	// Right upper leg
  	objects[2] = new OBJECT(SHAPE_CYLINDER, 
				ROBOT_STARFISH_LEG_RADIUS,
				ROBOT_STARFISH_LEG_LENGTH,
				+ROBOT_STARFISH_BODY_LENGTH/2.0
				+ROBOT_STARFISH_LEG_LENGTH/2.0,
				0,
				ROBOT_STARFISH_LEG_LENGTH
				+ROBOT_STARFISH_LEG_RADIUS,
				+1,0,0);

	// Forward upper leg
  	objects[3] = new OBJECT(SHAPE_CYLINDER, 
				ROBOT_STARFISH_LEG_RADIUS,
				ROBOT_STARFISH_LEG_LENGTH,
				0,
				+ROBOT_STARFISH_BODY_LENGTH/2.0
				+ROBOT_STARFISH_LEG_LENGTH/2.0,
				ROBOT_STARFISH_LEG_LENGTH
				+ROBOT_STARFISH_LEG_RADIUS,
				0,+1,0);

	// Back upper leg
  	objects[4] = new OBJECT(SHAPE_CYLINDER, 
				ROBOT_STARFISH_LEG_RADIUS,
				ROBOT_STARFISH_LEG_LENGTH,
				0,
				-ROBOT_STARFISH_BODY_LENGTH/2.0
				-ROBOT_STARFISH_LEG_LENGTH/2.0,
				ROBOT_STARFISH_LEG_LENGTH
				+ROBOT_STARFISH_LEG_RADIUS,
				0,-1,0);

	// Left lower leg
  	objects[5] = new OBJECT(SHAPE_CYLINDER, 
				ROBOT_STARFISH_LEG_RADIUS,
				ROBOT_STARFISH_LEG_LENGTH,
				-ROBOT_STARFISH_BODY_LENGTH/2.0
				-ROBOT_STARFISH_LEG_LENGTH,
				0,
				ROBOT_STARFISH_LEG_LENGTH/2.0
				+ROBOT_STARFISH_LEG_RADIUS,
				0,0,+1);

	// Right lower leg
  	objects[6] = new OBJECT(SHAPE_CYLINDER, 
				ROBOT_STARFISH_LEG_RADIUS,
				ROBOT_STARFISH_LEG_LENGTH,
				+ROBOT_STARFISH_BODY_LENGTH/2.0
				+ROBOT_STARFISH_LEG_LENGTH,
				0,
				ROBOT_STARFISH_LEG_LENGTH/2.0
				+ROBOT_STARFISH_LEG_RADIUS,
				0,0,+1);

	// Forward lower leg
  	objects[7] = new OBJECT(SHAPE_CYLINDER, 
				ROBOT_STARFISH_LEG_RADIUS,
				ROBOT_STARFISH_LEG_LENGTH,
				0,
				+ROBOT_STARFISH_BODY_LENGTH/2.0
				+ROBOT_STARFISH_LEG_LENGTH,
				ROBOT_STARFISH_LEG_LENGTH/2.0
				+ROBOT_STARFISH_LEG_RADIUS,
				0,0,+1);

	// Back lower leg
  	objects[8] = new OBJECT(SHAPE_CYLINDER, 
				ROBOT_STARFISH_LEG_RADIUS,
				ROBOT_STARFISH_LEG_LENGTH,
				0,
				-ROBOT_STARFISH_BODY_LENGTH/2.0
				-ROBOT_STARFISH_LEG_LENGTH,
				ROBOT_STARFISH_LEG_LENGTH/2.0
				+ROBOT_STARFISH_LEG_RADIUS,
				0,0,+1);

	for (int i=0;i<numObjects;i++)

		objects[i]->Sensor_Light_Add();

	objects[5]->Sensor_Touch_Add();
	objects[6]->Sensor_Touch_Add();
	objects[7]->Sensor_Touch_Add();
	objects[8]->Sensor_Touch_Add();
}

bool  ROBOT::File_Exists(char *fileName) {

	ifstream ifile(fileName);
	return ifile;
}

int  ROBOT::File_Index_Next_Available(void) {

	int fileIndex = 0;
	char fileName[100];
	sprintf(fileName,"SavedFiles/robot%d.dat",fileIndex);
	while ( File_Exists(fileName) ) {
		fileIndex++;
		sprintf(fileName,"SavedFiles/robot%d.dat",fileIndex);
	}

	return( fileIndex );
}

void ROBOT::Initialize(ifstream *inFile) {

	(*inFile) >> numObjects;

	objects = new OBJECT * [numObjects];

	for (int i=0;	i<numObjects;	i++)

		objects[i] = new OBJECT(this,inFile);


	(*inFile) >> numJoints;

	joints = new JOINT * [numJoints];

	for (int j=0;	j<numJoints;	j++)

		joints[j] = new JOINT(this,inFile);

	hidden = false;
	physicalized = false;

	neuralNetwork = NULL;

	sensorDifferences = 0.0;
}

void ROBOT::Initialize(ROBOT *other) {

	numObjects = other->numObjects;

	objects = new OBJECT * [numObjects];

	for (int i=0;	i<numObjects;	i++)
		objects[i] = new OBJECT(this,other->objects[i]);

	numJoints = other->numJoints;

	joints = new JOINT * [numJoints];

	for (int j=0;	j<numJoints;	j++)
		joints[j] = new JOINT(this,other->joints[j]);

	hidden = false;
	physicalized = false;

	if ( other->neuralNetwork )
		neuralNetwork = new NEURAL_NETWORK(other->neuralNetwork);
	else
		neuralNetwork = NULL;

	sensorDifferences = 0.0;
}

void ROBOT::Neural_Network_Set_Sensors(void) {

	int    sensorIndex = 0;
	double sensorValue = 0.0;

	for (int i=0;	i<numObjects;	i++)

		if ( objects[i] ) {

			if ( objects[i]->lightSensor ) {

				sensorValue = objects[i]->lightSensor->Get_Value();
				
				neuralNetwork->Sensor_Set(sensorIndex,sensorValue);

				sensorIndex++;
			}

			if ( objects[i]->touchSensor ) {

				sensorValue = objects[i]->touchSensor->Get_Value();
				//sensorValue = 0.0;

				neuralNetwork->Sensor_Set(sensorIndex,sensorValue);

				sensorIndex++;
			}
		}

	for (int j=0;	j<numJoints;	j++)

		if ( joints[j] ) {

			if ( joints[j]->proprioceptiveSensor ) {

				sensorValue = joints[j]->proprioceptiveSensor->Get_Value();
				//sensorValue = 0.0;

				neuralNetwork->Sensor_Set(sensorIndex,sensorValue);

				sensorIndex++;
			}
		}
}

void ROBOT::Neural_Network_Update(int timeStep) {

	if ( !neuralNetwork )
		return;

	neuralNetwork->Update(timeStep);
}

void   ROBOT::Sensors_Touch_Print(void) {

	for (int i=0;	i<numObjects;	i++)

		if ( objects[i] )

			objects[i]->Sensor_Touch_Print();

	printf("\n");
}

double ROBOT::Sensors_Get_Largest_Difference(ROBOT *other) {

	double largestDifferenceInJoints =

		Sensors_In_Joints_Largest_Difference(other);

	double largestDifferenceInObjects =

		Sensors_In_Objects_Largest_Difference(other);

	if ( largestDifferenceInJoints > largestDifferenceInObjects )

		return( largestDifferenceInJoints );

	else
		return( largestDifferenceInObjects );
}

double ROBOT::Sensors_Get_Total_Differences(ROBOT *other) {

//	return(	Sensors_In_Joints_Total_Differences(other) +
//		Sensors_In_Objects_Total_Differences(other) );

	return( Sensors_In_Objects_Total_Differences(other) );
}

double ROBOT::Sensors_Get_Total(void) {

	double sum = 0.0;

	for (int i=0;	i<numObjects;	i++)

		if ( objects[i] )

			if ( objects[i]->lightSensor ) {
				
				sum = sum + 
				objects[i]->lightSensor->Get_Value();

			}

	return( sum );

}

double ROBOT::Sensors_In_Joints_Largest_Difference(ROBOT *other) {

	double diff = -1000.0;

	for (int j=0;	j<numJoints;	j++)

		if ( joints[j] )

			if ( joints[j]->proprioceptiveSensor ) {

/*
				PROP_SENSOR *otherSensor =
				other->joints[j]->proprioceptiveSensor;

				if ( otherSensor ) {
				
					double currDiff = 
					joints[j]->proprioceptiveSensor->
						Difference(otherSensor);

					if ( currDiff > diff )
						diff = currDiff;

					otherSensor = NULL;
				}
*/
			}

	return( diff );
}

double ROBOT::Sensors_In_Joints_Total_Differences(ROBOT *other) {

	double diff = 0.0;
	double num  = 1.0;

	for (int j=0;	j<numJoints;	j++)

		if (	joints[j] && 
			joints[j]->proprioceptiveSensor &&
			other->joints[j] &&
			other->joints[j]->proprioceptiveSensor ) {
 
			double myVal	= joints[j]->proprioceptiveSensor->Get_Value();
			double otherVal = other->joints[j]->proprioceptiveSensor->Get_Value();;


			diff = diff + fabs(myVal - otherVal);
			num++;
		}

	return( diff/num );
}

void ROBOT::Sensors_In_Joints_Write(void) {

	for (int j=0;	j<numJoints;	j++)

		if ( joints[j] )

			if ( joints[j]->proprioceptiveSensor )

				joints[j]->proprioceptiveSensor->Write();
}

double ROBOT::Sensors_In_Objects_Largest_Difference(ROBOT *other) {

	double diff = -1000.0;

	for (int i=0;	i<numObjects;	i++)

		if ( objects[i] ) {

			if ( objects[i]->lightSensor ) {

				LIGHT_SENSOR *otherSensor =
				other->objects[i]->lightSensor;

				if ( otherSensor ) {
				
					double currDiff = 
						objects[i]->lightSensor->
						Difference(otherSensor);

					if ( currDiff > diff )
						diff = currDiff;

					otherSensor = NULL;
				}
			}

/*
			if ( objects[i]->touchSensor ) {

				TOUCH_SENSOR *otherSensor =
				other->objects[i]->touchSensor;

				if ( otherSensor ) {
				
					double currDiff = 
						objects[i]->touchSensor->
						Difference(otherSensor);

					if ( currDiff > diff )
						diff = currDiff;

					otherSensor = NULL;
				}
			}
*/
		}

	return( diff );
}

double ROBOT::Sensors_In_Objects_Total_Differences(ROBOT *other) {

	double diff = 0.0;
	double num  = 1.0;

	for (int i=0;	i<numObjects;	i++)

		if (	objects[i] && 
			objects[i]->lightSensor &&
			other->objects[i] &&
			other->objects[i]->lightSensor ) {
 
			double myVal	= objects[i]->lightSensor->Get_Value();
			double otherVal = other->objects[i]->lightSensor->Get_Value();

			diff = diff + fabs(myVal - otherVal);
			num++;
		}

	return( diff/num );
}

void ROBOT::Sensors_In_Objects_Write(void) {

	for (int i=0;	i<numObjects;	i++)

		if ( objects[i] )

			if ( objects[i]->lightSensor )

				objects[i]->lightSensor->Write();

}

void ROBOT::Sensors_Touch_Clear(void) {

	for (int i=0;	i<numObjects;	i++)

		if ( objects[i] )

			objects[i]->Sensor_Touch_Clear();
}

#endif
