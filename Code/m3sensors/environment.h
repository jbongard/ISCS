#ifndef _ENVIRONMENT_H
#define _ENVIRONMENT_H

/* Each environment contains one robot, one light source, and zero or more other objects. */

#include "object.h"
#include "robot.h"

class ENVIRONMENT {

public:
	OBJECT 		*lightSource;
	int		numOtherObjects;
	OBJECT 		**otherObjects;
	int		numRobots;
	class ROBOT  	**robots;
	int    		activeElement;
	int		activated;

private:
	int 		savedFileIndex;

public:
	ENVIRONMENT();
	ENVIRONMENT(ENVIRONMENT *other);
	ENVIRONMENT(ifstream *inFile);
	~ENVIRONMENT();
	void Activate_All(void);
	void Activate_Light_Source(void);
	void Activate_Robot(int robotIndex);
	void Active_Element_Copy(void);
	void Active_Element_Delete(void);
	void Active_Element_Move(double x, double y, double z);
	void Active_Element_Next(void);
	void Active_Element_Previous(void);
	void Active_Element_Resize(double changeX, double changeY, double changeZ);
	void Add_Light_Source(void);
	void Add_Robot_Starfish(void);
	void Allow_Robot_To_Move(int timeStep);
	void Deactivate_All(void);
	void Destroy_Simulated_Objects(void);
	void Draw(void);
	double Fitness_Get(void);
	void Hide_Light_Source(void);
	void Hide_Other_Objects(void);
	void Hide_Robot(int robotIndex);
	void Label(class NEURAL_NETWORK *genome, int environmentIndex);
	void Load(void);
	void Move(double x, double y, double z);
	void Prepare_For_Simulation(dWorldID world, dSpaceID space);
	void Robot_Copy(void);
	void Robot_Delete(void);
	int  Robot_Has_Stopped(int timer);
	int  Robot_In_A_Fixed_Attractor(void);
	void Robots_Set_Color(double r, double g, double b);
	void Save(void);
	void Save(ofstream *outFile);
	double Sensor_Sum(void);
	void Set_Color(double r, double g, double b);
	void Unhide_All(void);

private:

	void Activate_Current_Element(void);
	int  Active_Element_Is_Object(void);
	void Deactivate_Current_Element(void);
	void Destroy(void);
	bool File_Exists(char *fileName);
	int  File_Index_Next_Available(void);
	void Initialize(void);
	void Load(ifstream *inFile);
	void Object_Copy(void);
	void Object_Delete(void);
	void Robot_Delete(int robotIndex);
	int  Robot_Stopped(int timer);
	void Robots_Recolorize(void);
	void SavedFile_FindNext(void);
};

#endif
