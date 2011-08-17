/* A sensor that returns a strong value when near a light source, and a low value when far from a light source. */

#include <ode/ode.h>
#include <drawstuff/drawstuff.h>

#include "object.h"

#ifndef _LIGHT_SENSOR_H
#define _LIGHT_SENSOR_H

class LIGHT_SENSOR {

public:
	class OBJECT *containerObject;
	double pos[3];
	dReal R[12];
	double sides[3];
	double value;

public:
	LIGHT_SENSOR(OBJECT *cO);
	~LIGHT_SENSOR(void);
	double Difference(LIGHT_SENSOR *other);
	void Draw(double radius, double x, double y, double z);
	void Draw(double radius, double *pos);
	double Get_Value(void);
	void Reset(void);
	void Update(double *pos);
	void Update(double x, double y, double z);
	void Write(void);
};

#endif
