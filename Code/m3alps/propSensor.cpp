#include "math.h"
#include "stdio.h"

#ifndef _PROP_SENSOR_CPP
#define _PROP_SENSOR_CPP

#include "propSensor.h"

PROP_SENSOR::PROP_SENSOR(void) {

	Reset();
}

PROP_SENSOR::~PROP_SENSOR(void) {

}

double PROP_SENSOR::Difference(PROP_SENSOR *other) {

	double otherValue = other->Get_Value();

//	printf("%3.3f \t %3.3f\n",
//		value,otherValue);

	double diff = fabs( value - otherValue );

	return( diff );
}

double PROP_SENSOR::Get_Value(void) {

	return( value );
}

void PROP_SENSOR::Reset(void) {

	value = 0.0;
}

void PROP_SENSOR::Update(double v) {

	value = v;
}

void PROP_SENSOR::Write(void) {

	printf("%3.3f ",value);
}

#endif
