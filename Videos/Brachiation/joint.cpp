#include "stdio.h"
#include "iostream"
#include "fstream"

using namespace std;

#ifndef _JOINT_CPP
#define _JOINT_CPP

#include "joint.h"

#include "robot.h"

extern double ROBOT_STARFISH_JOINT_RANGE;
extern double ROBOT_STARFISH_MOTOR_SPEED;
extern double ROBOT_STARFISH_MOTOR_STRENGTH;

JOINT::JOINT(void) {

	containerRobot = NULL;

	obj1Index = -1;
	obj2Index = -1;

	physicalized = false;
}

JOINT::JOINT(ROBOT *cR, int o1Index, int o2Index,
		double posX, double posY, double posZ,
		double axX,  double axY,  double axZ,
		double maxF, double maxE) {

	containerRobot = cR;

	obj1Index = o1Index;
	obj2Index = o2Index;

	x = posX;
	y = posY;
	z = posZ;

	axisX = axX;
	axisY = axY;
	axisZ = axZ;

	maxFlexion = maxF;
	maxExtension = maxE;

	physicalized = false;

	proprioceptiveSensor = NULL;
}

JOINT::JOINT(ROBOT *cR, JOINT *other) {

	containerRobot = cR;

	obj1Index = other->obj1Index;
	obj2Index = other->obj2Index;

	x = other->x;
	y = other->y;
	z = other->z;

	axisX = other->axisX;
	axisY = other->axisY;
	axisZ = other->axisZ;

	maxFlexion = other->maxFlexion;
	maxExtension = other->maxExtension;

	physicalized = false;

	if ( other->proprioceptiveSensor )

		proprioceptiveSensor = new PROP_SENSOR();

	else
		proprioceptiveSensor = NULL;
}

JOINT::JOINT(ROBOT *cR, ifstream *inFile) {

	containerRobot = cR;

	(*inFile) >> obj1Index	>> obj2Index;

	(*inFile) >> x 		>>  y 		>> z;

	(*inFile) >> axisX 	>>  axisY 	>> axisZ;

	(*inFile) >> maxFlexion	>> maxExtension;

	int containsProprioceptiveSensor;

	(*inFile) >> containsProprioceptiveSensor;

	physicalized = false;

	if ( containsProprioceptiveSensor )

		proprioceptiveSensor = new PROP_SENSOR;
	else
		proprioceptiveSensor = NULL;
}

JOINT::~JOINT(void) {

	if ( physicalized )
		Remove_From_Simulator();

	if ( containerRobot )
		containerRobot = NULL;

	if ( proprioceptiveSensor ) {
		delete proprioceptiveSensor;
		proprioceptiveSensor = NULL;
	}
}

void JOINT::Make_Incorporeal(void) {

	// Remove the joint from the physical simulator.

	if ( !physicalized )

		return;

	Remove_From_Simulator();

	Sensors_Reset();
}

void JOINT::Make_Physical(dWorldID world) {

	// Add the joint to the physical simulator.

	// Already added to the simulator.
	if ( physicalized )
		return;

	// If the robot doesn't exist...
	if ( !containerRobot )
		return;

	// If the first object doesn't exist...
	if ( !containerRobot->objects[obj1Index] )
		return;

	// If the second object doesn't exist...
	if ( !containerRobot->objects[obj2Index] )
		return;

	physicalized = true;

	joint = dJointCreateHinge(world,0);

	dJointAttach(	joint,
			containerRobot->objects[obj1Index]->body,
			containerRobot->objects[obj2Index]->body);

	dJointSetHingeAnchor(joint,x,y,z);

	dJointSetHingeAxis(joint,axisX,axisY,axisZ);

	dJointSetHingeParam(joint,dParamLoStop,
		Degrees_To_Radians(maxFlexion));

	dJointSetHingeParam(joint,dParamHiStop,
		Degrees_To_Radians(maxExtension));
}

void JOINT::Move(double motorNeuronValue) {

	// This joint will apply torque to the two objects
	// it connects, which will cause the robot (of which
	// this joint is a part) to move.

        dJointSetHingeParam(joint,dParamFMax,ROBOT_STARFISH_MOTOR_STRENGTH); // Motor force

	// Scale the motor neuron's value, which is in the range
	// [-1,1], to the joint's range of motion.
	double desiredAngle = Scale(motorNeuronValue,-1,+1,
					-ROBOT_STARFISH_JOINT_RANGE,
					+ROBOT_STARFISH_JOINT_RANGE);

//	printf("%3.3f %3.3f\n",motorNeuronValue,desiredAngle);
//	       desiredAngle = Degrees_To_Radians( desiredAngle );


        double actualAngle =  dJointGetHingeAngle(joint);
        double actualRate = dJointGetHingeAngleRate(joint);

        double ks = ROBOT_STARFISH_MOTOR_SPEED; // Motor speed
        double kd = 0.0;
        double error = 	ks*(desiredAngle - actualAngle) -
			kd*actualRate;

        dJointSetHingeParam(joint,dParamVel,error);
}

void JOINT::Move(double deltaX, double deltaY, double deltaZ) {

	// The robot (of which this joint is a part)
	// is being moved by the user.

	x = x + deltaX;
	y = y + deltaY;
	z = z + deltaZ;
}

void JOINT::Save(ofstream *outFile) {

	(*outFile) << obj1Index		<< " " << obj2Index			<< " \n";

	(*outFile) << x 		<< " " << y 		<< " " << z 	<< " \n";

	(*outFile) << axisX 		<< " " << axisY 	<< " " << axisZ << " \n";

	(*outFile) << maxFlexion	<< " " << maxExtension			<< " \n";

	if ( proprioceptiveSensor )

		(*outFile) << "1 \n";
	else
		(*outFile) << "0 \n";	
}

void JOINT::Sensor_Proprioceptive_Add(void) {

	if ( !proprioceptiveSensor ) {

		proprioceptiveSensor = new PROP_SENSOR();
	}
}

int JOINT::Sensors_Number_Of(void) {

	if ( proprioceptiveSensor )

		return( 1 );

	else

		return( 0 );
}

void JOINT::Sensors_Update(void) {

	// If the robot isn't physicalized, it cannot generate
	// sensor values.
	if ( !physicalized )
		return;

	if ( proprioceptiveSensor ) {

		double actualAngle = dJointGetHingeAngle(joint);

		actualAngle = Radians_To_Degrees(actualAngle);

		actualAngle = Scale(	actualAngle,
					-ROBOT_STARFISH_JOINT_RANGE,
					+ROBOT_STARFISH_JOINT_RANGE,
					-1,1);
		// Sensor values are always scaled to [-1,1]

		proprioceptiveSensor->Update(actualAngle);
	}
}

// --------------------- Private methods --------------------

double JOINT::Degrees_To_Radians(double degrees) {

	return( (3.14159*degrees)/180.0 );
}

double JOINT::Radians_To_Degrees(double radians) {

	return( (180.0*radians)/3.14159 );
}

double JOINT::Rand(double min, double max) {

	// Return a random value in [min,max] with
	// a uniform distribution.

        double zeroToOne = ((double)rand()) / RAND_MAX;
        double returnVal;

        returnVal = (zeroToOne * (max-min)) + min;
        return returnVal;
}

void JOINT::Remove_From_Simulator(void) {

	dJointDestroy(joint);
	joint = NULL;

	physicalized = false;
}

double JOINT::Scale(double value, double min1, double max1,
                                       double min2, double max2) {

        if ( min1 < 0 )
                value = value - min1;
        else
                value = value + min1;

        return( (value*(max2-min2)/(max1-min1)) + min2 );
}

void JOINT::Sensors_Reset(void) {

	if ( proprioceptiveSensor )

		proprioceptiveSensor->Reset();
}

#endif
