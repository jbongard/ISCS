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

extern double		NN_TAUS_MIN;
extern double		NN_TAUS_MAX;

extern double		NN_WEIGHTS_MIN;
extern double		NN_WEIGHTS_MAX;

extern double		NN_OMEGAS_MIN;
extern double		NN_OMEGAS_MAX;

extern double		NN_SENSORWEIGHTS_MIN;
extern double		NN_SENSORWEIGHTS_MAX;

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

	(*inFile) >> childTime;
	(*inFile) >> parentTime;

	(*inFile) >> previousBestFitness;
	(*inFile) >> mutationProbability;

	int isParent;
	(*inFile) >> isParent;
	if ( isParent )
		parent = new NEURAL_NETWORK(inFile);

	int isChild;
	(*inFile) >> isChild;
	if ( isChild )
		child = new NEURAL_NETWORK(inFile);
}

OPTIMIZER::~OPTIMIZER(void) {

	Destroy();
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

		MATRIX *taus = new MATRIX(	1,			numMotors,
						NN_TAUS_MIN,		NN_TAUS_MAX);

		MATRIX *wts  = new MATRIX(	numMotors,		numMotors,
						NN_WEIGHTS_MIN,		NN_WEIGHTS_MAX);

		MATRIX *omegas = new MATRIX(	1,			numMotors,
						NN_OMEGAS_MIN,		NN_OMEGAS_MAX);

		// Sensor weights connect the current sensor readings, previous
		// sensor readings, and the change in sensor readings (3*numSensors)
		MATRIX *sensorWts = new MATRIX(	3*numSensors,	numMotors, 
						NN_SENSORWEIGHTS_MIN,	NN_SENSORWEIGHTS_MAX);

		parent = new NEURAL_NETWORK(	taus,wts,omegas,sensorWts,
						numSensors,numMotors);

		delete taus;
		taus = NULL;

		delete wts;
		wts = NULL;

		delete omegas;
		omegas = NULL;

		delete sensorWts;
		sensorWts = NULL;

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

	printf("t: %d    \t",
		evaluationPeriod);

	printf("tu: %d    \t",
		timer);

	printf("e: %d    \t",
		evalsSinceLastChampFound);

	printf("m: %5.5f \t",
		mutationProbability);

	printf("pr: %5.5f    \t",
		previousBestFitness);

	printf("p: %12.12f    \t",
		parentFitness);

	printf("c: %12.12f    \t",
		childFitness);

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
		parentTime = childTime;
		childFitness = WORST_FITNESS;
		childTime = -1;
		evalsSinceLastChampFound = 0;

	}
	else {
		delete child;
		child = NULL;
		childFitness = WORST_FITNESS;
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
	previousBestFitness 	= WORST_FITNESS;

	child = new NEURAL_NETWORK(parent);
	
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
