/* A sensor that returns a strong value when near a light source, and a low value when far from a light source. */

#include "stdio.h"

#include <ode/ode.h>
#include <drawstuff/drawstuff.h>

#ifndef _LIGHT_SENSOR_CPP
#define _LIGHT_SENSOR_CPP

#include "lightSensor.h"

#ifdef dDOUBLE
#define dsDrawLine dsDrawLineD
#define dsDrawBox dsDrawBoxD
#define dsDrawSphere dsDrawSphereD
#define dsDrawCylinder dsDrawCylinderD
#endif

extern double MAX_LIGHT_SENSOR_DISTANCE;

LIGHT_SENSOR::LIGHT_SENSOR(OBJECT *cO) {

	containerObject = cO;
	
	Reset();
}

LIGHT_SENSOR::~LIGHT_SENSOR(void) {

	if ( containerObject )
		containerObject = NULL;
}

double LIGHT_SENSOR::Difference(LIGHT_SENSOR *other) {

	double otherValue = other->Get_Value();

	double diff = fabs( value - otherValue );

	return( diff );
}

void LIGHT_SENSOR::Draw(double radius, double x, double y, double z) {
	
	Update(x,y,z);

	pos[0] = x;
	pos[1] = y;
	pos[2] = z;

	dRFromZAxis(R,0,0,1);

	dsSetColor(value,value,value);

	dsDrawSphere(pos,R,radius);
}

void LIGHT_SENSOR::Draw(double radius, double *pos) {

	Draw(radius,pos[0],pos[1],pos[2]);
}

double LIGHT_SENSOR::Get_Value(void) {

	return( value );
}

void LIGHT_SENSOR::Reset(void) {

	value = 0.0;
}

void LIGHT_SENSOR::Update(double *pos) {

	Update(pos[0],pos[1],pos[2]);
}

void LIGHT_SENSOR::Update(double x, double y, double z) {

	OBJECT *cO 	= containerObject;
	ROBOT *cR 	= cO->containerRobot;
	ENVIRONMENT *cE	= cR->containerEnvironment;

	double lsX = cE->lightSource->x;
	double lsY = cE->lightSource->y;
	double lsZ = cE->lightSource->z;

	cO = NULL;
	cR = NULL;
	cE = NULL;

	double distFromLightSource =

		sqrt(	pow(x-lsX,2) + 
			pow(y-lsY,2) + 
			pow(z-lsZ,2) );

	if ( distFromLightSource > MAX_LIGHT_SENSOR_DISTANCE )

		value = 0;
	else
		value = 1 - 	(distFromLightSource /
				 MAX_LIGHT_SENSOR_DISTANCE);

	// Sensor values must always be in [-1,+1].
	value = (value*2.0) - 1.0;
}

void LIGHT_SENSOR::Write(void) {

	printf("%3.3f ",value);
}

#endif
