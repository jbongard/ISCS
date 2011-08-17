#include "stdio.h"
#include "iostream"
#include "fstream"

using namespace std;

#include <ode/ode.h>
#include <drawstuff/drawstuff.h>

#ifndef _OBJECT_CPP
#define _OBJECT_CPP

#include "object.h"

#ifdef dDOUBLE
#define dsDrawLine dsDrawLineD
#define dsDrawBox dsDrawBoxD
#define dsDrawSphere dsDrawSphereD
#define dsDrawCylinder dsDrawCylinderD
#define dsDrawCapsule dsDrawCapsuleD
#endif

extern int SHAPE_CYLINDER;
extern int SHAPE_RECTANGLE;

extern int OBJECT_STATE_INCORPOREAL;
extern int OBJECT_STATE_SOLID;
extern int OBJECT_STATE_PHYSICAL;

extern int      OBJECT_FACE_FRONT;
extern int      OBJECT_FACE_BACK;
extern int      OBJECT_FACE_LEFT;
extern int      OBJECT_FACE_RIGHT;
extern int      OBJECT_FACE_TOP;
extern int      OBJECT_FACE_BOTTOM;

OBJECT::OBJECT(	int sh, 
		double l, double w, double h,
		double posX, double posY, double posZ,
		double rX,   double rY,   double rZ) {

	Initialize(posX,posY,posZ,rX,rY,rZ);

	shape = sh;
	
	length = w;
	width = l;
	height = h;
}

OBJECT::OBJECT(	int sh, 
		double rd, double ln,
		double posX,   double posY, double posZ,
		double rX,     double rY,   double rZ) {

	Initialize(posX,posY,posZ,rX,rY,rZ);

	shape = sh;
	
	radius = rd;
	length = ln;
}

OBJECT::OBJECT(	int sh, 
		double rd,
		double posX,   double posY, double posZ,
		double rX,     double rY,   double rZ) {

	Initialize(posX,posY,posZ,rX,rY,rZ);

	shape = sh;
	
	radius = rd;
}

OBJECT::OBJECT(ROBOT *cR, OBJECT *other) {

	Initialize(	other->x,other->y,other->z,
			other->rotX,other->rotY,other->rotZ);

	Initialize(other);

	containerRobot 		= cR;
}

OBJECT::OBJECT(ENVIRONMENT *cE, OBJECT *other) {

	Initialize(	other->x,other->y,other->z,
			other->rotX,other->rotY,other->rotZ);

	Initialize(other);

	containerEnvironment 	= cE;

}

OBJECT::OBJECT(ROBOT *cR, ifstream *inFile) {

	Initialize(inFile);

	containerRobot 		= cR;
}


OBJECT::OBJECT(ENVIRONMENT *cE, ifstream *inFile) {

	Initialize(inFile);

	containerEnvironment 	= cE;
}

OBJECT::~OBJECT(void) {

	Destroy();
}

void OBJECT::Activate(void) {

	active = true;
}

void OBJECT::Deactivate(void) {

	active = false;
}

void OBJECT::Draw(void) {

	if ( hidden )
		return;

	if ( active )
		dsSetColor(color[0],color[1],color[2]);

	else
		dsSetColorAlpha(color[0],color[1],color[2],0.5);

	if ( state==OBJECT_STATE_INCORPOREAL )

		Draw_Geometrically();

	else if ( state==OBJECT_STATE_SOLID )

		Draw_Solidly();

	else
		Draw_Physically();
}

void OBJECT::Hide(void) {

	hidden = true;
}

int  OBJECT::Is_Cylinder(void) {

	return( shape==SHAPE_CYLINDER );
}

int  OBJECT::Is_Incorporeal(void) {

	return( state==OBJECT_STATE_INCORPOREAL );
}

int  OBJECT::Is_Part_Of_Robot(void) {

	return( containerRobot != NULL );
}

int  OBJECT::Is_Physical(void) {

	return( state==OBJECT_STATE_PHYSICAL );
}

int  OBJECT::Is_Rectangular_Solid(void) {

	return( shape==SHAPE_RECTANGLE );
}

void OBJECT::Make_Incorporeal(void) {

	// Render the object only geometric: it is only drawn,
	// but cannot interact with any physical objects in
	// the simulator.

	if ( Is_Incorporeal() )
		return;

	state = OBJECT_STATE_INCORPOREAL;

	Remove_From_Simulator();

	Sensors_Reset();
}

void OBJECT::Make_Physical(dWorldID world, dSpaceID space) {

	// Add the object to the physical simulator.

	// Already added to the simulator.
	if ( Is_Physical() )
		return;

	state = OBJECT_STATE_PHYSICAL;

	body = dBodyCreate (world);
	dBodySetPosition(body,x,y,z);

	dRFromZAxis(R,rotX,rotY,rotZ);
    	dBodySetRotation(body,R);

	if ( shape == SHAPE_RECTANGLE ) {

		dMassSetBox (&m,1,length,width,height);
		geom = dCreateBox(space,length,width,height);

	}
	else if ( shape == SHAPE_CYLINDER ) {

		dMassSetSphere (&m,1,radius);
		geom = dCreateCapsule(space,radius,length);
	}

	dMassAdjust (&m,1);
	dBodySetMass (body,&m);

	dGeomSetBody (geom,body);

	dGeomSetData(geom,this);
	// Allow the geom to point to which object it is
	// associated with.
}

void OBJECT::Make_Solid(dWorldID world, dSpaceID space) {

	// Make the object solid in the simulator:
	// it cannot move, but other objects can collide
	// with it.

	// Already added to the simulator.
	if (	(state==OBJECT_STATE_SOLID) ||
		(state==OBJECT_STATE_PHYSICAL) )
		return;

	state = OBJECT_STATE_SOLID;

	if ( shape == SHAPE_RECTANGLE ) {

		geom = dCreateBox(space,length,width,height);

	}
	else if ( shape == SHAPE_CYLINDER ) {

		geom = dCreateCapsule(space,radius,length);
	}

	dGeomSetPosition(geom,x,y,z);

	dRFromZAxis(R,rotX,rotY,rotZ);
    	dGeomSetRotation(geom,R);

	dGeomSetData(geom,this);
	// Allow the geom to point to which object it is
	// associated with.
}

void OBJECT::Move(double deltaX, double deltaY, double deltaZ) {

	x = x + deltaX;
	y = y + deltaY;
	z = z + deltaZ;
}

void OBJECT::Resize(double changeX, double changeY, double changeZ) {

	// No negative sizes allowed.
	if ( (width	+ changeX)<0 )
		return;

	if ( (length	+ changeY)<0 )
		return;

	if ( (height	+ changeZ)<0 )
		return;

	width	= width 	+ changeX;
	length	= length	+ changeY;
	height	= height 	+ changeZ;
}

void OBJECT::Save(ofstream *outFile) {

	(*outFile) << shape << "\n";

	(*outFile) << color[0] << " " << color[1] << " " << color[2] << "\n";

	if ( Is_Cylinder() )
		Save_Cylinder(outFile);
	else
		Save_Rectangular_Solid(outFile);

	(*outFile) << x << " " << y << " " << z << "\n";

	(*outFile) << rotX << " " << rotY << " " << rotZ << "\n";

	(*outFile) << active << "\n";

	if ( lightSensor )
		(*outFile) << "1 \n"; // Contains light sensor
	else
		(*outFile) << "0 \n"; // Contains light sensor

	if ( touchSensor )
		(*outFile) << "1 \n"; // Contains touch sensor
	else
		(*outFile) << "0 \n"; // Contains touch sensor
}

void OBJECT::Sensor_Light_Add(void) {

	lightSensor = new LIGHT_SENSOR(this);
}

void OBJECT::Sensor_Touch_Add(void) {

	touchSensor = new TOUCH_SENSOR(this);
}

void OBJECT::Sensor_Touch_Print(void) {

	if ( touchSensor )

		touchSensor->Print();
}

void OBJECT::Sensor_Touch_Clear(void) {

	if ( touchSensor )

		touchSensor->Clear();
}

void OBJECT::Sensor_Touch_Set(void) {

	if ( touchSensor )

		touchSensor->Activate();
}

int OBJECT::Sensors_Number_Of(void) {

	int numSensors = 0;

	if ( lightSensor )

		numSensors++;

	if ( touchSensor )

		numSensors++;

	return( numSensors );
}

void OBJECT::Sensors_Update() {

	if ( lightSensor ) {

		if ( state==OBJECT_STATE_INCORPOREAL )

			lightSensor->Update(x,y,z);

		else
			lightSensor->Update((double*)dBodyGetPosition(body));
	}
}

void OBJECT::Set_Color(double r, double g, double b) {

	color[0] = r;
	color[1] = g;
	color[2] = b;
}

void OBJECT::Unhide(void) {

	hidden = false;
}

// ------------------------- Private methods ------------------------

void OBJECT::Destroy(void) {

	if ( 	(state==OBJECT_STATE_SOLID) || 
		(state==OBJECT_STATE_PHYSICAL) )

		Remove_From_Simulator();

	if ( lightSensor ) {
		delete lightSensor;
		lightSensor = NULL;
	}

	if ( touchSensor ) {
		delete touchSensor;
		touchSensor = NULL;
	}

	if ( containerEnvironment ) {
		containerEnvironment = NULL;
	}

	if ( containerRobot ) {
		containerRobot = NULL;
	}
}

void OBJECT::Draw_Geometrically(void) {

	pos[0] = x;
	pos[1] = y;
	pos[2] = z;

	dRFromZAxis(R,rotX,rotY,rotZ);

	if ( shape == SHAPE_RECTANGLE ) {

		sides[0] = length;
		sides[1] = width;
		sides[2] = height;
		dsDrawBox(pos,R,sides);
	}
	else if ( shape == SHAPE_CYLINDER ) {

		dsDrawCapsule(pos,R,length,radius);
	}

	if ( lightSensor ) {

		if ( shape==SHAPE_CYLINDER )
			lightSensor->Draw(1.5*radius,x,y,z);

		else if ( shape == SHAPE_RECTANGLE ) {

			double minDimension = 1000.0;

			if ( length < minDimension )
				minDimension = length;

			if ( width < minDimension )
				minDimension = width;

			if ( height < minDimension )
				minDimension = height;

			lightSensor->Draw(1.5*minDimension,x,y,z);
		}
	}
}

void OBJECT::Draw_Physically(void) {

	if ( shape == SHAPE_RECTANGLE ) {

		dReal sides[3] = {length,width,height};
		dsDrawBox (	dBodyGetPosition(body),
				dBodyGetRotation(body),sides);
	}
	else if ( shape == SHAPE_CYLINDER ) {

		dsDrawCapsule(	dBodyGetPosition(body),
				dBodyGetRotation(body),length,radius);
	}

	if ( lightSensor ) {

		if ( shape == SHAPE_CYLINDER )
			lightSensor->Draw(1.5*radius,
			(double*)dBodyGetPosition(body));

		else if ( shape == SHAPE_RECTANGLE ) {

			double minDimension = 1000.0;

			if ( length < minDimension )
				minDimension = length;

			if ( width < minDimension )
				minDimension = width;

			if ( height < minDimension )
				minDimension = height;

			lightSensor->Draw(1.5*minDimension,
					(double*)dBodyGetPosition(body));
		}
	}
}

void OBJECT::Draw_Solidly(void) {

	if ( shape == SHAPE_RECTANGLE ) {

		dReal sides[3] = {length,width,height};
		dsDrawBox (	dGeomGetPosition(geom),
				dGeomGetRotation(geom),sides);
	}
	else if ( shape == SHAPE_CYLINDER ) {

		dsDrawCapsule(	dGeomGetPosition(geom),
				dGeomGetRotation(geom),length,radius);
	}
}

bool  OBJECT::File_Exists(char *fileName) {

	ifstream ifile(fileName);
	return ifile;
}

int  OBJECT::File_Index_Next_Available(void) {

	int fileIndex = 0;
	char fileName[100];
	sprintf(fileName,"SavedFiles/object%d.dat",fileIndex);
	while ( File_Exists(fileName) ) {
		fileIndex++;
		sprintf(fileName,"SavedFiles/object%d.dat",fileIndex);
	}

	return( fileIndex );
}

void OBJECT::Get_Position(double *pos) {

	const dReal *p;

	if ( state==OBJECT_STATE_INCORPOREAL ) {

		pos[0] = x;
		pos[1] = y;
		pos[2] = z;

		return;
	}

	else if ( state==OBJECT_STATE_SOLID )

		p = dGeomGetPosition(geom);

	else
		p = dBodyGetPosition(body);

	pos[0] = p[0];
	pos[1] = p[1];
	pos[2] = p[2];

}

void OBJECT::Initialize(double posX, double posY, double posZ,
			double rX,   double rY,   double rZ) {

	containerEnvironment = NULL;
	containerRobot = NULL;
	lightSensor = NULL;
	touchSensor = NULL;

	x = posX;
	y = posY;
	z = posZ;

	rotX = rX;
	rotY = rY;
	rotZ = rZ;

	active = false;
	hidden = false;
	
	body = NULL;
	geom = NULL;

	state = OBJECT_STATE_INCORPOREAL;
}

void OBJECT::Initialize(OBJECT *other) {

	shape			= other->shape;
	color[0]		= other->color[0];
	color[1]		= other->color[1];
	color[2]		= other->color[2];

	if ( shape == SHAPE_CYLINDER ) {
		radius		= other->radius;
		length		= other->length;
	}
	else if ( shape == SHAPE_RECTANGLE ) {
		length		= other->length;
		width		= other->width;
		height		= other->height;
	}
	else {
		radius		= other->radius;
	}

	if ( other->lightSensor )
		lightSensor = new LIGHT_SENSOR(this);
	else
		lightSensor = NULL;

	if ( other->touchSensor )
		touchSensor = new TOUCH_SENSOR(this);
	else
		touchSensor = NULL;
}

void OBJECT::Initialize(ifstream *inFile) {

	containerEnvironment = NULL;
	containerRobot = NULL;
	lightSensor = NULL;
	touchSensor = NULL;

	(*inFile) >> shape;

	(*inFile) >> color[0] >> color[1] >> color[2];

	if ( Is_Cylinder() )
		Initialize_Cylinder(inFile);
	else
		Initialize_Rectangular_Solid(inFile);

	(*inFile) >> x >> y >> z;

	(*inFile) >> rotX >> rotY >> rotZ;

	(*inFile) >> active;

	int containsLightSensor;
	(*inFile) >> containsLightSensor;
	if ( containsLightSensor )
		lightSensor = new LIGHT_SENSOR(this);
	else
		lightSensor = NULL;

	int containsTouchSensor;
	(*inFile) >> containsTouchSensor;
	if ( containsTouchSensor )
		touchSensor = new TOUCH_SENSOR(this);
	else
		touchSensor = NULL;

	hidden = false;
	
	body = NULL;
	geom = NULL;

	state = OBJECT_STATE_INCORPOREAL;
}

void OBJECT::Initialize_Cylinder(ifstream *inFile) {

	(*inFile) >> radius >> length;
}

void OBJECT::Initialize_Rectangular_Solid(ifstream *inFile) {

	(*inFile) >> length >> width >> height;
}

void OBJECT::Remove_From_Simulator(void) {

	state = OBJECT_STATE_INCORPOREAL;

	if ( geom ) {
		dGeomDestroy(geom);
		geom = NULL;
	}

	if ( body ) {
		dBodyDestroy(body);
		body = NULL;
	}
}

void OBJECT::Save_Cylinder(ofstream *outFile) {

	(*outFile) << radius << " " << length << "\n";
}

void OBJECT::Save_Rectangular_Solid(ofstream *outFile) {

	(*outFile) << length << " " << width << " " << height << "\n";
}

void OBJECT::Sensors_Reset(void) {

	if ( lightSensor )

		lightSensor->Reset();

	if ( touchSensor )

		touchSensor->Reset();
}

#endif
