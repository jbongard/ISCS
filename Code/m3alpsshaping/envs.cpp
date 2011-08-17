#include "stdio.h"
#include "sys/stat.h"

#include <ode/ode.h>
#include <drawstuff/drawstuff.h>

#include <GL/glut.h>


#ifndef _ENVS_CPP
#define _ENVS_CPP

#include "envs.h"
#include "optimizer.h"

extern int	MODE_VIEW_SIDE;
extern int	MODE_VIEW_TOP;
extern int	MODE_VIEW_BACK;

extern int	MODE_SIMULATE_DESIGN;
extern int	MODE_SIMULATE_EVOLVE;
extern int	MODE_SIMULATE_CHAMP;

extern int	TIME_STEPS_PER_FRAME;

extern int	SELECTION_LEVEL_OBJECT;
extern int	SELECTION_LEVEL_ENVIRONMENT;
extern int	SELECTION_LEVEL_ENVS;

extern int	MAX_ENVIRONMENTS;

extern double	MOVE_INCREMENT;

extern int	MAX_EVALS_BEFORE_SAVING;

extern double	WORST_FITNESS;

extern int	TIME_TO_CHECK_FOR_NEUTRAL_MUTATION;

ENVS::ENVS(void) {

	simulateMode=MODE_SIMULATE_DESIGN;

	numberOfEnvs = 1;
	currNumberOfEnvs = 1;

	taskEnvironments = new ENVIRONMENT * [MAX_ENVIRONMENTS];

	for (int i=0;	i<MAX_ENVIRONMENTS;	i++)
		taskEnvironments[i] = NULL;

	taskEnvironments[0] = new ENVIRONMENT();

	taskEnvironments[0]->Add_Light_Source();
	taskEnvironments[0]->Add_Robot_Starfish();
	taskEnvironments[0]->Set_Color(1,0,0);

	optimizer = NULL;

	targetSensorValuesRecorded = false;

	recordingVideo = false;

	savedFileIndex = -1;

	selectionLevel = SELECTION_LEVEL_OBJECT;
	activeEnvironment = 0;

	evaluationsSinceLastSave = 0;}

ENVS::~ENVS(void) {

	if ( optimizer ) {
		delete optimizer;
		optimizer = NULL;
	}

	for (int i=0;	i<MAX_ENVIRONMENTS;	i++) {

		if ( taskEnvironments[i] ) {
			delete taskEnvironments[i];
			taskEnvironments[i] = NULL;
		}
	}
	delete[] taskEnvironments;
	taskEnvironments = NULL;
}

void ENVS::Active_Element_Copy(void) {

	if ( simulateMode == MODE_SIMULATE_DESIGN ) {

		if ( selectionLevel == SELECTION_LEVEL_ENVIRONMENT )

			Environment_Copy();

		else if ( selectionLevel == SELECTION_LEVEL_OBJECT )

			taskEnvironments[activeEnvironment]->Active_Element_Copy();
	}
}

void ENVS::Active_Element_Delete(void) {

	if ( simulateMode == MODE_SIMULATE_DESIGN )

		if ( selectionLevel == SELECTION_LEVEL_ENVIRONMENT )

			Environment_Delete();

		else if ( selectionLevel == SELECTION_LEVEL_OBJECT )

			taskEnvironments[activeEnvironment]->Active_Element_Delete();
}

void ENVS::Active_Element_Load(void) {

	if ( simulateMode == MODE_SIMULATE_DESIGN ) {

		     if ( selectionLevel == SELECTION_LEVEL_ENVIRONMENT )

			taskEnvironments[activeEnvironment]->Load();

		else if ( selectionLevel == SELECTION_LEVEL_ENVS )
			Load(true);

		// Robots during design are always red.
		for (int i=0;	i<numberOfEnvs;	i++)
			taskEnvironments[i]->Robots_Set_Color(1,0,0);
	}
}

void ENVS::Active_Element_Move(double x, double y, double z) {

	if ( simulateMode == MODE_SIMULATE_DESIGN )

		if ( selectionLevel == SELECTION_LEVEL_ENVS )

			Move(x,y,z);

		else if ( selectionLevel == SELECTION_LEVEL_ENVIRONMENT )

			Environment_Move(x,y,z);

		else if ( selectionLevel == SELECTION_LEVEL_OBJECT )

			taskEnvironments[activeEnvironment]->Active_Element_Move(x,y,z);
}

void ENVS::Active_Element_Next(void) {

	if ( simulateMode == MODE_SIMULATE_DESIGN )

		if ( selectionLevel == SELECTION_LEVEL_ENVIRONMENT )

			Environment_Next();

		else if ( selectionLevel == SELECTION_LEVEL_OBJECT )

			taskEnvironments[activeEnvironment]->Active_Element_Next();
}

void ENVS::Active_Element_Previous(void) {

	if ( simulateMode == MODE_SIMULATE_DESIGN )

		if ( selectionLevel == SELECTION_LEVEL_ENVIRONMENT )

			Environment_Previous();

		else if ( selectionLevel == SELECTION_LEVEL_OBJECT )

			taskEnvironments[activeEnvironment]->Active_Element_Previous();
}

void ENVS::Active_Element_Resize(double changeX, double changeY, double changeZ) {

	if ( simulateMode == MODE_SIMULATE_DESIGN )

		if ( selectionLevel == SELECTION_LEVEL_OBJECT )

			taskEnvironments[activeEnvironment]->Active_Element_Resize(changeX,changeY,changeZ);
}

void ENVS::Active_Element_Save(void) {

	     if ( selectionLevel == SELECTION_LEVEL_ENVIRONMENT )

		taskEnvironments[activeEnvironment]->Save();

	else if ( selectionLevel == SELECTION_LEVEL_ENVS )
		Save(true);
}

void ENVS::Draw(void) {

	for (int i=0;	i<currNumberOfEnvs;	i++) {

		if ( taskEnvironments[i] )
			taskEnvironments[i]->Draw();
	}
}

void ENVS::EvaluationPeriod_Decrease(dWorldID world, dSpaceID space, dJointGroupID contactgroup) {

/*
	if ( In_Design_Mode() )
		return;

	Destroy_Simulated_Objects();

	optimizer->Reset_But_Keep_Best();
		
	if ( optimizer )

		optimizer->EvaluationPeriod_Decrease();
*/
}

void ENVS::EvaluationPeriod_Increase(dWorldID world, dSpaceID space, dJointGroupID contactgroup) {

/*
	if ( In_Design_Mode() )
		return;

	Destroy_Simulated_Objects();

	optimizer->Reset_But_Keep_Best();

	if ( optimizer )

		optimizer->EvaluationPeriod_Increase();
*/
}

void ENVS::Evolve(	dWorldID      world, 
			dSpaceID      space) {

	// If there is no robot being evaluated yet,
	// create one.
	if ( !Robot_Being_Evaluated() ) {

		Create_Robot_To_Evaluate(world,space);

		evaluationsSinceLastSave++;
		if ( evaluationsSinceLastSave == MAX_EVALS_BEFORE_SAVING ) {
			selectionLevel = SELECTION_LEVEL_ENVS;
			Save(false);
			optimizer->Print();	
			evaluationsSinceLastSave = 0;
		}
	}
	// If there is a robot being evaluated, and its time
	// is not yet up, allow it to move.
	if ( !Eval_Finished() ) {

		for (int i=0;	i<currNumberOfEnvs;	i++) {

			taskEnvironments[i]->Allow_Robot_To_Move(optimizer->timer);
		}
		optimizer->Timer_Update();
	}

	// If the time limit for the robot being evaluated has
	// expired, record its fitness, remove it from the simulator,
	// and prepare for the evaluation of the next robot.
	else {

		double fitness = Fitness_Get();

		optimizer->Fitness_Receive(fitness);

		Destroy_Simulated_Objects();

		optimizer->Timer_Reset();
	}
}

int ENVS::In_Champ_Mode(void) {

	return( simulateMode == MODE_SIMULATE_CHAMP );
}

int ENVS::In_Design_Mode(void) {

	return( simulateMode == MODE_SIMULATE_DESIGN );
}

int ENVS::In_Evolution_Mode(void) {

	return( simulateMode == MODE_SIMULATE_EVOLVE );
}

void ENVS::Mode_Simulate_Set_Champ(void) {

	// Switch to visualizing the best robot so far.

	// Already in champ mode.
	if ( simulateMode==MODE_SIMULATE_CHAMP )

		return;

	// If designing task environments, there is no
	// best robot yet.

	if ( simulateMode==MODE_SIMULATE_DESIGN )

		return;

	// Destroy the evolving robot.
	Destroy_Simulated_Objects();

	if ( optimizer )
		optimizer->Timer_Reset();

	simulateMode = MODE_SIMULATE_CHAMP;

	// The best robot is always green.
	for (int i=0;	i<numberOfEnvs;	i++)

		taskEnvironments[i]->Robots_Set_Color(0,1,0);
}

void ENVS::Mode_Simulate_Set_Design(void) {

	// Switch to the designing of task environments.

	// Already in design mode.
	if ( simulateMode==MODE_SIMULATE_DESIGN )

		return;

	if ( simulateMode==MODE_SIMULATE_EVOLVE )

		optimizer->Genome_Discard_Being_Evaluated();

	Destroy_Simulated_Objects();

	simulateMode=MODE_SIMULATE_DESIGN;

	currNumberOfEnvs = numberOfEnvs;

	// Robots during design are always red.
	for (int i=0;	i<numberOfEnvs;	i++)
		taskEnvironments[i]->Robots_Set_Color(1,0,0);
}

void ENVS::Mode_Simulate_Set_Evolve(dWorldID world, dSpaceID space) {

	// Switch to evolution.

	// Already in evolve mode.
	if ( simulateMode==MODE_SIMULATE_EVOLVE )
		return;

	// No task for the robot to perform has yet been set.
	if ( End_State_Missing() )
		return;

	// Just finished defining the task environment.
	if ( simulateMode==MODE_SIMULATE_DESIGN )

		Optimizer_Initialize();

	// Just finished viewing the current champ.
	else if ( simulateMode==MODE_SIMULATE_CHAMP ) {

		Destroy_Simulated_Objects();

		if ( optimizer )
			optimizer->Timer_Reset();
	}

	simulateMode=MODE_SIMULATE_EVOLVE;

	// The evolving robots are always blue.
	for (int i=0;i<numberOfEnvs;i++)
		taskEnvironments[i]->Robots_Set_Color(0,0,1);
}

void ENVS::Mode_View_Set_Back(void) {

	viewMode = MODE_VIEW_BACK;

	Viewpoint_Set(0,-2.792,0.79,90,-10.5,0);
}

void ENVS::Mode_View_Set_Side(void) {

	viewMode = MODE_VIEW_SIDE;

	Viewpoint_Set(-7.274,1.658,2.16,0,-6,0);
}

void ENVS::Mode_View_Set_Top(void) {

	viewMode = MODE_VIEW_TOP;

	Viewpoint_Set(0,2,7,90,-90,0);
}

void ENVS::MutationProbability_Decrease(void) {

	if ( optimizer )

		optimizer->MutationProbability_Decrease();
}

void ENVS::MutationProbability_Increase(void) {

	if ( optimizer )

		optimizer->MutationProbability_Increase();
}

void ENVS::Prepare_To_Run_Without_Graphics(dWorldID world, dSpaceID space) {
	
	selectionLevel=SELECTION_LEVEL_ENVS;

	Load(false);

	Mode_Simulate_Set_Evolve(world,space);
}

void ENVS::Selection_Level_Lower(void) {

	// Already at lowest level...
	if ( selectionLevel==SELECTION_LEVEL_OBJECT )
		return;

	if ( selectionLevel==SELECTION_LEVEL_ENVIRONMENT ) {

		taskEnvironments[activeEnvironment]->Deactivate_All();

		taskEnvironments[activeEnvironment]->Activate_Light_Source();

		selectionLevel = SELECTION_LEVEL_OBJECT;
	}
	else {
		
		Deactivate_All();

		if ( activeEnvironment<0 )
			activeEnvironment=0;

		taskEnvironments[activeEnvironment]->Activate_All();

		selectionLevel = SELECTION_LEVEL_ENVIRONMENT;
	}
}

void ENVS::Selection_Level_Raise(void) {

	// Already at highest level...
	if ( selectionLevel==SELECTION_LEVEL_ENVS )
		return;

	else if ( selectionLevel==SELECTION_LEVEL_ENVIRONMENT ) {

		taskEnvironments[activeEnvironment]->Deactivate_All();

		Activate_All();

		selectionLevel=SELECTION_LEVEL_ENVS;
	}

	else {

		taskEnvironments[activeEnvironment]->Activate_All();

		selectionLevel=SELECTION_LEVEL_ENVIRONMENT;
	}
}

void ENVS::Show_Champ(	dWorldID      world, 
			dSpaceID      space) {

	// If there is no champ being evaluated yet,
	// create one.
	if ( !Robot_Being_Evaluated() )

		Create_Robot_Current_Best(world,space);

	// If there is a champ being evaluated, and its time
	// is not yet up, allow it to move.

	if ( !Eval_Finished() ) {

		for (int i=0;	i<currNumberOfEnvs;	i++) {

			taskEnvironments[i]->Allow_Robot_To_Move(optimizer->timer);
		}
		optimizer->Timer_Update();
	}

	// If the time limit for the champ has
	// expired, replay it.
	else {

		optimizer->Print();

		Destroy_Simulated_Objects();

		optimizer->Timer_Reset();
	}
}

void ENVS::Video_Record(void) {

	if ( timeStepsSinceLastFrameCapture < TIME_STEPS_PER_FRAME ) {
		timeStepsSinceLastFrameCapture++;
		return;
	}

	printf("Recording frame %d in Movie%d.\n",frameIndex,movieIndex);

	int width = 352;
	int height = 288;

	char s[100];

	if ( frameIndex<10 )
		sprintf (s,"Movie%d/0000%d.ppm",movieIndex,frameIndex);
	else if ( frameIndex<100 )
		sprintf (s,"Movie%d/000%d.ppm",movieIndex,frameIndex);
	else if ( frameIndex<1000 )
		sprintf (s,"Movie%d/00%d.ppm",movieIndex,frameIndex);
	else if ( frameIndex<10000 )
		sprintf (s,"Movie%d/0%d.ppm",movieIndex,frameIndex);
	else if ( frameIndex<100000 )
		sprintf (s,"Movie%d/%d.ppm",movieIndex,frameIndex);

	FILE *f = fopen (s,"wb");
	fprintf (f,"P6\n%d %d\n255\n",width,height);

	void *buf = malloc( width * height * 3 );
	glReadPixels( 0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, buf );

	for (int y=(height - 1); y>=0; y--) {
		for (int x=0; x<width; x++) {
			unsigned char *pixel = ((unsigned char *)buf)+((y*width+ x)*3);
			unsigned char b[3];
			b[0] = *pixel;
			b[1] = *(pixel+1);
			b[2] = *(pixel+2);
			fwrite(b,3,1,f);
		}
	}
	free(buf);
	fclose(f);

	frameIndex++;
	timeStepsSinceLastFrameCapture = 0;
}

void ENVS::Video_Start_Stop(void) {

	if ( recordingVideo )

		Video_Stop();
	else
		Video_Start();
}

void ENVS::Viewpoint_Get(void) {

	dsGetViewpoint(xyz,hpr);

	printf("%3.3f %3.3f %3.3f %3.3f %3.3f %3.3f\n",
		xyz[0],xyz[1],xyz[2],hpr[0],hpr[1],hpr[2]);
}

// ---------------- Private methods -------------------

int  ENVS::A_Robot_Has_Stopped(void) {

	int aRobotHasStopped = false;
	int currentEnvironment = 0;

	while (	(!aRobotHasStopped) &&
		(currentEnvironment<numberOfEnvs) ) {

		if ( taskEnvironments[currentEnvironment]->Robot_Has_Stopped(optimizer->timer) )

			aRobotHasStopped = true;
		else
			currentEnvironment++;
	}

	return( aRobotHasStopped );
}

void ENVS::Activate_All(void) {

	for (int i=0;	i<numberOfEnvs;	i++)

		if ( taskEnvironments[i] )

			taskEnvironments[i]->Activate_All();
}

void ENVS::Camera_Position_Load(ifstream *inFile, int showGraphics) {

	(*inFile) >> xyz[0] >> xyz[1] >> xyz[2];

	(*inFile) >> hpr[0] >> hpr[1] >> hpr[2];

	if ( showGraphics )
		dsSetViewpoint(xyz,hpr);
}

void ENVS::Camera_Position_Save(ofstream *outFile, int showGraphics) {

	if ( showGraphics )
		dsGetViewpoint(xyz,hpr);

	(*outFile) << xyz[0] << " " << xyz[1] << " " << xyz[2] << "\n";

	(*outFile) << hpr[0] << " " << hpr[1] << " " << hpr[2] << "\n";
}

void ENVS::Create_Robot_Current_Best( 	dWorldID      world, 
					dSpaceID      space) {

	currNumberOfEnvs = numberOfEnvs;

	NEURAL_NETWORK *bestGenome = optimizer->Genome_Get_Best(currNumberOfEnvs);

	for (int i=0;	i<currNumberOfEnvs;	i++) {

		taskEnvironments[i]->Prepare_For_Simulation(world,space);

		taskEnvironments[i]->Label(bestGenome,i);
	}

	bestGenome = NULL;
}

void ENVS::Create_Robot_To_Evaluate(	dWorldID      world, 
					dSpaceID      space) {

	NEURAL_NETWORK *nextGenome = optimizer->Genome_Get_Next_To_Evaluate(&currNumberOfEnvs);

	for (int i=0;	i<currNumberOfEnvs;	i++) {

		taskEnvironments[i]->Prepare_For_Simulation(world,space);

		taskEnvironments[i]->Label(nextGenome,i);
	}

	nextGenome = NULL;
}

void ENVS::Deactivate_All(void) {

	for (int i=0;	i<numberOfEnvs;	i++)

		if ( taskEnvironments[i] )

			taskEnvironments[i]->Deactivate_All();
}

void ENVS::Destroy_Simulated_Objects(void) {

	for (int i=0;i<currNumberOfEnvs;i++)

		taskEnvironments[i]->Destroy_Simulated_Objects();
}

int  ENVS::Directory_Found(char *dirName) {

	struct stat stFileInfo;
	bool blnReturn;
	int intStat;

	intStat = stat(dirName,&stFileInfo);
	if(intStat == 0) {
		blnReturn = true;
	} else {
		blnReturn = false;
	}

	return(blnReturn);
}

void ENVS::Directory_Make(char *dirName) {

        char command[500];

        sprintf(command,"mkdir %s",dirName);
        system(command);
}

int  ENVS::End_State_Missing(void) {

	int missing = false;

	int currentEnvironment = 0;

	while ( (currentEnvironment<numberOfEnvs) && 
		(!missing) ) {

		if ( taskEnvironments[currentEnvironment]->numRobots<2 )
			missing = true;
		else
			currentEnvironment++;
	}

	return( missing );
}

void ENVS::Environment_Copy(void) {

	// Maximum number of environments already exist. 
	if ( numberOfEnvs == MAX_ENVIRONMENTS )
		return;

	taskEnvironments[numberOfEnvs] = 

		new ENVIRONMENT(taskEnvironments[activeEnvironment]);

	taskEnvironments[activeEnvironment]->Deactivate_All();

	taskEnvironments[numberOfEnvs]->Move(-MOVE_INCREMENT,MOVE_INCREMENT,0);
	taskEnvironments[numberOfEnvs]->Activate_All();

	activeEnvironment = numberOfEnvs;

	numberOfEnvs++;
	currNumberOfEnvs++;

	if ( optimizer )

		optimizer->Environments_Change(numberOfEnvs);
}

void ENVS::Environment_Delete(void) {

	// Cannot remove only remaining environment.
	if ( numberOfEnvs == 1 )
		return;

	// Can only remove the last environment.
	if ( activeEnvironment!=(numberOfEnvs-1) )
		return;

	delete taskEnvironments[numberOfEnvs-1];
	taskEnvironments[numberOfEnvs-1] = NULL;

	numberOfEnvs--;
	currNumberOfEnvs--;

	if ( optimizer )
	
		optimizer->Environments_Change(numberOfEnvs);

	activeEnvironment = numberOfEnvs-1;

	taskEnvironments[numberOfEnvs-1]->Activate_All();
}

void ENVS::Environment_Move(double x, double y, double z) {

	taskEnvironments[activeEnvironment]->Move(x,y,z);
}

void ENVS::Environment_Next(void) {

	taskEnvironments[activeEnvironment]->Deactivate_All();

	activeEnvironment++;

	if ( activeEnvironment==numberOfEnvs )
		activeEnvironment = 0;

	taskEnvironments[activeEnvironment]->Activate_All();
}

void ENVS::Environment_Previous(void) {

	taskEnvironments[activeEnvironment]->Deactivate_All();

	activeEnvironment--;

	if ( activeEnvironment==-1 )
		activeEnvironment = numberOfEnvs-1;

	taskEnvironments[activeEnvironment]->Activate_All();

}

int  ENVS::Eval_Finished(void) {

	if ( optimizer->Time_Elapsed() )

		return( true );

/*
	if ( 	In_Evolution_Mode() &&

		(optimizer->timer == TIME_TO_CHECK_FOR_NEUTRAL_MUTATION) ) {

		optimizer->SensorSum_Receive(Sensor_Sum());

		if ( optimizer->SensorSums_Match() )

			return( true );
	}
*/

	if (	In_Evolution_Mode() &&
		
		optimizer->Robot_Failed_On_Layer(Fitness_Get()) )

		return( true );

	return( false );
}

bool  ENVS::File_Exists(char *fileName) {

	ifstream ifile(fileName);
	return ifile;
}

int  ENVS::File_Index_Next_Available(void) {

/*
	int fileIndex = 0;
	char fileName[100];
	sprintf(fileName,"SavedFiles/envs%d.dat",fileIndex);
	while ( File_Exists(fileName) ) {
		fileIndex++;
		sprintf(fileName,"SavedFiles/envs%d.dat",fileIndex);
	}

	return( fileIndex );
*/
	return( 0 );
}

double ENVS::Fitness_Get(void) {

	// Take average fitness.
	double fitness = taskEnvironments[0]->Fitness_Get();
	for (int i=1;	i<currNumberOfEnvs;	i++)
		fitness = fitness + taskEnvironments[i]->Fitness_Get();
	fitness = fitness / double(currNumberOfEnvs);

/*
	// Multiply fitnesses.
	double fitness = fabs(taskEnvironments[0]->Fitness_Get());
	for (int i=1;	i<numberOfEnvs;	i++)
		fitness = fitness * fabs(taskEnvironments[i]->Fitness_Get());
	fitness = -fitness;
*/

/*
	// Take worst fitness.
	double fitness = +1000.0;
	for (int i=0;	i<numberOfEnvs;	i++) {
		double temp = taskEnvironments[i]->Fitness_Get();
		if ( temp < fitness )
			fitness = temp;
	}
*/

	return( fitness );
}

void ENVS::Load(int showGraphics) {

	SavedFile_FindNext();

	char fileName[100];
	sprintf(fileName,"SavedFiles/envs%d.dat",savedFileIndex);
	printf("%s\n",fileName);

	ifstream *inFile = new ifstream(fileName);

	Camera_Position_Load(inFile,showGraphics);

	Load_Environments(inFile);

	Load_Optimizer(inFile);

	inFile->close();
	delete inFile;
	inFile = NULL;

	printf("envs %d loaded.\n",savedFileIndex);
}

void ENVS::Load_Environments(ifstream *inFile) {

	for (int i=0;i<numberOfEnvs;i++) {
		delete taskEnvironments[i];
		taskEnvironments[i] = NULL;
	}

	(*inFile) >> numberOfEnvs;

	for (int i=0;	i<numberOfEnvs;	i++)

		taskEnvironments[i] = new ENVIRONMENT(inFile);
}

void ENVS::Load_Optimizer(ifstream *inFile) {

	int isOptimizer;
	(*inFile) >> isOptimizer;

	if ( optimizer )
		delete optimizer;

	if ( isOptimizer )
		optimizer = new OPTIMIZER(inFile);
}

void ENVS::Move(double x, double y, double z) {

	for (int i=0;i<numberOfEnvs;i++)

		if ( taskEnvironments[i] )

			taskEnvironments[i]->Move(x,y,z);
}

void ENVS::Optimizer_Initialize(void) {

	if ( !optimizer ) {

		int numberOfSensors = 
		taskEnvironments[0]->robots[0]->Sensors_Number_Of();

		int numberOfMotors = 
		taskEnvironments[0]->robots[0]->Motors_Number_Of();
 
		// The genomes encode a synaptic weight from each
		// sensor to each hidden neuron, and from each
		// hidden neuron to each motor
		// of size = (inputs + outputs ) x hidden neurons
		optimizer = new OPTIMIZER(	numberOfSensors,
						numberOfMotors);
	}
}

void ENVS::Reset(dWorldID world, dSpaceID space, dJointGroupID contactgroup) {

	Destroy_Simulated_Objects();

	dJointGroupEmpty(contactgroup);

	optimizer->Genome_Discard_Being_Evaluated();

	Create_Robot_To_Evaluate(world, space);
}

int  ENVS::Robot_Being_Evaluated(void) {

	int allAreBeingEvaluated = true;

	int currentEnvironment = 0;

	while (	(allAreBeingEvaluated) &&
		(currentEnvironment<currNumberOfEnvs) )

		if ( taskEnvironments[currentEnvironment]->robots[0]->In_Simulator() )

			currentEnvironment++;
		else
			allAreBeingEvaluated = false;

	return( allAreBeingEvaluated );
}

int  ENVS::Robots_Have_Stopped(void) {

	int allHaveStopped = true;
	int currentEnvironment = 0;

	while (	(allHaveStopped) &&
		(currentEnvironment<currNumberOfEnvs) ) {

		if ( !taskEnvironments[currentEnvironment]->Robot_Has_Stopped(optimizer->timer) )

			allHaveStopped = false;
		else
			currentEnvironment++;
	}

	return( allHaveStopped );
}

void ENVS::Save(int showGraphics) {

	int fileIndex = File_Index_Next_Available();

	char fileName[100];
	sprintf(fileName,"SavedFiles/envs%d.dat",fileIndex);

	ofstream *outFile = new ofstream(fileName);

	Camera_Position_Save(outFile,showGraphics);

	Save_Environments(outFile);

	Save_Optimizer(outFile);

	outFile->close();
	delete outFile;
	outFile = NULL;

	printf("Envs %d saved.\n",fileIndex);
}

void ENVS::Save_Environments(ofstream *outFile) {

	(*outFile) << numberOfEnvs << "\n";

	for (int i=0;	i<numberOfEnvs;	i++)

		if ( taskEnvironments[i] )

			taskEnvironments[i]->Save(outFile);
}

void ENVS::Save_Optimizer(ofstream *outFile) {

	if ( optimizer ) {
		(*outFile) << "1\n";
		optimizer->Save(outFile);
	}
	else
		(*outFile) << "0\n";
}

void ENVS::SavedFile_FindNext(void) {

	/*
	savedFileIndex++;

	char fileName[100];
	sprintf(fileName,"SavedFiles/envs%d.dat",savedFileIndex);

	if ( !File_Exists(fileName) )

		savedFileIndex = 0;
	*/

	savedFileIndex = 0;
}

double ENVS::Sensor_Sum(void) {

	double sum = 0.0;

	for (int i=0;	i<numberOfEnvs;	i++)

		sum = sum + taskEnvironments[i]->Sensor_Sum();

	return( sum );
}

int  ENVS::Target_Sensor_Values_Recorded(void) {

	return( targetSensorValuesRecorded );
}

void ENVS::Video_Start(void) {

	movieIndex = 0;

	char dirName[100];

	sprintf(dirName,"Movie%d",movieIndex);

	while ( Directory_Found(dirName) ) {
		movieIndex++;
		sprintf(dirName,"Movie%d",movieIndex);
	}

	Directory_Make(dirName);

	frameIndex=1;
	timeStepsSinceLastFrameCapture = 0;

	recordingVideo = true;
}

void ENVS::Video_Stop(void) {

	char command[500];

	char fileName[500];
	sprintf(fileName,"Movie%d.bat",movieIndex);

	ofstream *outFile = new ofstream(fileName);

	(*outFile) << "cd Movie"<<movieIndex<<" \n";

	(*outFile) << "for f in *ppm ; do nice -n 20 convert -quality 100 $f `basename $f ppm`jpg; done \n";

	(*outFile) << "rm *.ppm \n";

	(*outFile) << "mencoder 'mf://*.jpg' -mf fps=60 -o Movie"<<movieIndex<<".avi -ovc lavc \n";

	(*outFile) << "rm *.jpg \n";

	outFile->close();
	delete outFile;

	sprintf(command,"chmod 777 Movie%d.bat",movieIndex);
	system(command);

	sprintf(command,"./Movie%d.bat &",movieIndex);
	system(command);

	recordingVideo = false;
}

void ENVS::Viewpoint_Set(	double x, double y, double z,
				double h, double p, double r) {

	xyz[0] = x;
	xyz[1] = y;
	xyz[2] = z;

	hpr[0] = h;
	hpr[1] = p;
	hpr[2] = r;

	dsSetViewpoint(xyz,hpr);
}
#endif
