/* This class defines a joint, which is a structure that combines two objects together and allows them to rotate around one another. */

#include <ode/ode.h>
#include <drawstuff/drawstuff.h>

#ifndef _JOINT_H
#define _JOINT_H

#include "propSensor.h"

class JOINT {

public:
	class   ROBOT 	*containerRobot;
	PROP_SENSOR 	*proprioceptiveSensor;

private:
	int		obj1Index;
	int		obj2Index;
	double		x,y,z;
	double  	axisX, axisY, axisZ;
	dJointID 	joint;
	int		physicalized;
	double		maxFlexion, maxExtension;
	// The joint's limits, expressed in degrees.

public:
	JOINT(void);
	JOINT(ROBOT *cR, int obj1Index, int obj2Index,
		double posX, double posY, double posZ,
		double axX,  double axY,  double axZ,
		double maxF, double maxE);

	JOINT(ROBOT *cR, JOINT *other);
	JOINT(ROBOT *cR, ifstream *inFile);
	~JOINT(void);
	void Make_Incorporeal(void);
	void Make_Physical(dWorldID world);
	void Move(double motorNeuronValue);
	void Move(double deltaX, double deltaY, double deltaZ);
	void Save(ofstream *outFile);
	void Sensor_Proprioceptive_Add(void);
	int  Sensors_Number_Of(void);
	void Sensors_Update(void);

private:
	double Degrees_To_Radians(double degrees);
	double Radians_To_Degrees(double radians);
	double Rand(double min, double max);
	void Remove_From_Simulator(void);
        double Scale(double value, double min1, double max1,
                         double min2, double max2);
	void Sensors_Reset(void);
};

#endif
