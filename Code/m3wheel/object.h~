/* An object is an object that exists within the simulator.

It may or may not be active: if it active, it affects the physics of the simulation. If it is not, it is only drawn translucently. */

#include <ode/ode.h>
#include <drawstuff/drawstuff.h>
#include "iostream"
#include "fstream"

using namespace std;

#ifndef _OBJECT_H
#define _OBJECT_H

#include "lightSensor.h"
#include "touchSensor.h"
#include "robot.h"

class OBJECT {

public:
	class   ENVIRONMENT *containerEnvironment;
	class   ROBOT *containerRobot;
	int 	shape;
	double  color[3];
	double  radius;
	double  length, width, height;
	double	x, y, z;
	double  rotX, rotY, rotZ;
	int     active;

	LIGHT_SENSOR *lightSensor;
	TOUCH_SENSOR *touchSensor;

	dBodyID body;

private:
	double pos[3];
	dReal R[12];
	double sides[3];
	int hidden;
	int state;
	dMass m;
	dGeomID geom;

public:
	OBJECT(	int sh, 
		double l,      double w,    double h,
		double posX,   double posY, double posZ,
		double rX,     double rY,   double rZ);

	OBJECT(	int sh, 
		double radius, double length,
		double posX,   double posY, double posZ,
		double rX,     double rY,   double rZ);

	OBJECT( ROBOT *cR, OBJECT *other );
	OBJECT( ENVIRONMENT *cE, OBJECT *other );

	OBJECT( ROBOT *cR, ifstream *inFile );
	OBJECT( ENVIRONMENT *cE, ifstream *inFile );

	~OBJECT();
	void Activate(void);
	void Deactivate(void);
	void Draw(void);
	void Get_Position(double *pos);
	void Hide(void);
	int  Is_Cylinder(void);
	int  Is_Incorporeal(void);
	int  Is_Part_Of_Robot(void);
	int  Is_Physical(void);
	int  Is_Rectangular_Solid(void);
	void Make_Incorporeal(void);
	void Make_Physical(dWorldID world, dSpaceID space);
	void Make_Solid(dWorldID world, dSpaceID space);
	void Move(double deltaX, double deltaY, double deltaZ);
	void Reproduce(void);
	void Resize(double changeX, double changeY, double changeZ);
	void Save(ofstream *outFile);
	void Sensor_Light_Add(void);
	void Sensor_Touch_Add(void);
	void Sensor_Touch_Clear(void);
	void Sensor_Touch_Set(void);
	void Sensor_Touch_Print(void);
	int  Sensors_Number_Of(void);
	void Sensors_Update(void);
	void Set_Color(double r, double g, double b);
	void Unhide(void);

private:
	void Destroy(void);
	void Draw_Geometrically(void);
	void Draw_Physically(void);
	void Draw_Solidly(void);
	bool File_Exists(char *fileName);
	int  File_Index_Next_Available(void);
	void Initialize(double posX, double posY, double posZ,
			double rX,   double rY,   double rZ);
	void Initialize(OBJECT *other);
	void Initialize(ifstream *inFile);
	void Initialize_Cylinder(ifstream *inFile);
	void Initialize_Rectangular_Solid(ifstream *inFile);
	void Remove_From_Simulator(void);
	void Save_Cylinder(ofstream *outFile);
	void Save_Rectangular_Solid(ofstream *outFile);
	void Sensors_Reset(void);
};

#endif
