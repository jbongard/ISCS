#include "math.h"
#include "stdio.h"

#ifndef _OPTIMIZER_CPP
#define _OPTIMIZER_CPP

#include "optimizer.h"
#include "robot.h"

extern int	EVALUATION_TIME;
extern double	MUTATION_PROBABILITY;
extern int	EVALS_UNTIL_EVAL_TIME_EXTENDED;
extern int	STARTING_EVALUATION_TIME;
extern int	EVAL_TIME_EXTENSION_PERIOD;
extern double	WORST_FITNESS;

OPTIMIZER::OPTIMIZER(int numberOfSensors, int numberOfMotors) {

	numSensors = numberOfSensors;
	numMotors  = numberOfMotors;

	Initialize();
}

OPTIMIZER::OPTIMIZER(ifstream *inFile) {

	timer = 0;

	(*inFile) >> numSensors;
	(*inFile) >> numMotors;

	(*inFile) >> evaluationPeriod;
	(*inFile) >> evalsSinceLastChampFound;
	
	(*inFile) >> childFitness;
	(*inFile) >> parentFitness;

	childSensorSum = 0.0;
	parentSensorSum = 0.0;

	(*inFile) >> childTime;
	(*inFile) >> parentTime;

	(*inFile) >> previousBestFitness;
	(*inFile) >> mutationProbability;

	int isParent;
	(*inFile) >> isParent;
	if ( isParent )
		parent = new NEURAL_NETWORK(inFile);
	else
		parent = NULL;

	int isChild;
	(*inFile) >> isChild;
	if ( isChild )
		child = new NEURAL_NETWORK(inFile);
	else
		child = NULL;
}

OPTIMIZER::~OPTIMIZER(void) {

	Destroy();
}

int  OPTIMIZER::Current_Robot_Failed(double currentFitness) {

	// If the parent is being evaluated, just keep going.
	if ( !child )
		return( false );

	return ( currentFitness < parentFitness );
}

void OPTIMIZER::EvaluationPeriod_Decrease(void) {

	evaluationPeriod--;

	if ( evaluationPeriod < 10 )

		evaluationPeriod = 10;
}

void OPTIMIZER::EvaluationPeriod_Increase(void) {

	evaluationPeriod++;
}

void OPTIMIZER::Fitness_Receive(double fitness) {

	// A neutral mutation occurred.
	if ( SensorSums_Match() ) {

		childFitness = parentFitness;
		return;
	}

	if ( !child ) {
		parentFitness = fitness;
		parentTime = timer;
	}
	else {
		childFitness = fitness;
		childTime = timer;
	}
}

NEURAL_NETWORK *OPTIMIZER::Genome_Get_Best(void) {

	return( parent );
}

NEURAL_NETWORK *OPTIMIZER::Genome_Get_Curr_To_Evaluate(void) {

	if ( !child ) {

		if ( !parent )

			return( NULL );
		else
			return( parent );
	}
	else
		return( child );
}

NEURAL_NETWORK *OPTIMIZER::Genome_Get_Next_To_Evaluate(void) {

	if ( !parent ) {

		parent = new NEURAL_NETWORK(numMotors);

		return( parent );
	}
	else if ( !child ) {

		child = new NEURAL_NETWORK(parent);
		child->Mutate(mutationProbability);
	}

	return( child );
}

void OPTIMIZER::MutationProbability_Decrease(void) {

	mutationProbability = mutationProbability /2.0;
}

void OPTIMIZER::MutationProbability_Increase(void) {

	mutationProbability = mutationProbability * 2.0;

	if ( mutationProbability > 1.0 )
		mutationProbability = 1.0;
}

void OPTIMIZER::Print(void) {

	printf("t: %d \t",
		evaluationPeriod);

	if ( timer>=100 )
		printf("tu: %d \t",
			timer);
	else if ( timer>=10 )
		printf("tu: 0%d \t",
			timer);
	else
		printf("tu: 00%d \t",
			timer);

	printf("e: %d \t",
		evalsSinceLastChampFound);

	printf("m: %5.5f \t",
		mutationProbability);

	printf("p: %5.5f \t",
		parentFitness);
	if ( parent )
		printf("dp: %5.5f \t",
			parent->Density());

	printf("c: %5.5f \t",
		childFitness);
	if ( child )
		printf("dc: %5.5f \t",
			child->Density());
	printf("\n");
}

void OPTIMIZER::PrintParentChild(void) {

	if ( parent )

		parent->Print();


	if ( child )

		child->Print();
}

void OPTIMIZER::Reproduce(void) {

	if ( childFitness >= parentFitness ) {

		delete parent;
		parent = child;
		child = NULL;
		parentFitness = childFitness;
		parentSensorSum = childSensorSum;
		parentTime = childTime;
		childFitness = WORST_FITNESS;
		childTime = -1;
		evalsSinceLastChampFound = 0;
	}
	else {
		delete child;
		child = NULL;
		childFitness = WORST_FITNESS;
		childSensorSum = 0.0;
		childTime = -1;
		evalsSinceLastChampFound++;
	}
}

void OPTIMIZER::Reset(void) {

	Destroy();

	Initialize();
}

void OPTIMIZER::Reset_But_Keep_Best(void) {

	if ( child ) {

		delete child;
		child = NULL;
	}

	parentFitness 		= WORST_FITNESS;
	parentSensorSum		= 0.0;

	previousBestFitness 	= WORST_FITNESS;

	child = new NEURAL_NETWORK(parent);
	childSensorSum		= 0.0;
	
	Timer_Reset();
}

void OPTIMIZER::Save(ofstream *outFile) {

	(*outFile) << numSensors		<< "\n";
	(*outFile) << numMotors			<< "\n";

	(*outFile) << evaluationPeriod		<< "\n";
	(*outFile) << evalsSinceLastChampFound	<< "\n";
	
	(*outFile) << childFitness		<< "\n";
	(*outFile) << parentFitness		<< "\n";

	(*outFile) << childTime			<< "\n";
	(*outFile) << parentTime		<< "\n";

	(*outFile) << previousBestFitness	<< "\n";
	(*outFile) << mutationProbability	<< "\n";

	if ( parent ) {
		(*outFile) << "1"		<< "\n";
		parent->Save(outFile);
	}
	else
		(*outFile) << "0"		<< "\n";

	if ( child ) {
		(*outFile) << "1"		<< "\n";
		child->Save(outFile);
	}
	else
		(*outFile) << "0"		<< "\n";
}

void OPTIMIZER::SensorSum_Receive(double sensorSum) {

	if ( !child )

		parentSensorSum = sensorSum;

	else
		childSensorSum  = sensorSum;
}

int OPTIMIZER::SensorSums_Match(void) {

	if ( !child )

		return( false );

	return( childSensorSum == parentSensorSum );
}

int OPTIMIZER::Time_Elapsed(void) {

	return (timer >= evaluationPeriod);

}

void OPTIMIZER::Timer_Reset(void) {

	timer = 0;
}

void OPTIMIZER::Timer_Update(void) {

	timer++;
}

// --------------------------- Private methods ------------------------

void OPTIMIZER::Destroy(void) {

	if ( child ) {
		delete child;
		child = NULL;
		childFitness = WORST_FITNESS;
	}

	if ( parent ) {
		delete parent;
		parent = NULL;
		parentFitness = WORST_FITNESS;
	}
}

int OPTIMIZER::FlipCoin(void) {

	return( Rand(0.0,1.0) < 0.5 );
}

void OPTIMIZER::Initialize(void) {

	evaluationPeriod = STARTING_EVALUATION_TIME;
	
	evalsSinceLastChampFound = 0;

	mutationProbability = MUTATION_PROBABILITY;

	Timer_Reset();

	parent = NULL;

	child = NULL;

	parentFitness       = WORST_FITNESS;
	childFitness        = WORST_FITNESS;

	parentSensorSum     = 0.0;
	childSensorSum      = 0.0;

	previousBestFitness = WORST_FITNESS;

}

double OPTIMIZER::Rand(double min, double max) {

	// Return a random value in [min,max] with
	// a uniform distribution.

        double zeroToOne = ((double)rand()) / RAND_MAX;
        double returnVal;

        returnVal = (zeroToOne * (max-min)) + min;
        return returnVal;
}

#endif
