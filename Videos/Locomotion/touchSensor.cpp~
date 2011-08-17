#include "math.h"
#include "stdio.h"
#include "stdlib.h"

#ifndef _TOUCH_SENSOR_CPP
#define _TOUCH_SENSOR_CPP

#include "touchSensor.h"

TOUCH_SENSOR::TOUCH_SENSOR(OBJECT *cO) {

	containerObject = cO;

	Reset();
}

TOUCH_SENSOR::~TOUCH_SENSOR(void) {

	containerObject = NULL;
}

void TOUCH_SENSOR::Activate(void) {

	value = 1.0;
}

void TOUCH_SENSOR::Clear(void) {

	value = -1.0;
}

double TOUCH_SENSOR::Difference(TOUCH_SENSOR *other) {

	double otherValue = other->Get_Value();

//	printf("%3.3f \t %3.3f\n",
//		value,otherValue);

	double diff = fabs( value - otherValue );

	return( diff );
}

double TOUCH_SENSOR::Get_Value(void) {

	return( value );
}

void TOUCH_SENSOR::Print(void) {

	printf("%0.0f ",value);
}

void TOUCH_SENSOR::Reset(void) {

	value = -1.0;
}

#endif
