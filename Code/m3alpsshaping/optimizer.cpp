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

extern int	ALPS_NUM_LAYERS;

extern int	ALPS_GENOMES_PER_LAYER;

OPTIMIZER::OPTIMIZER(int numberOfSensors, int numberOfMotors) {

	Initialize();

	numSensors = numberOfSensors;
	numMotors  = numberOfMotors;
}

OPTIMIZER::OPTIMIZER(ifstream *inFile) {

	Initialize();

	(*inFile) >> nextGenomeID;

	(*inFile) >> nextVictimOnFirstLayer;

	(*inFile) >> layerIndex;

	(*inFile) >> genomesEvaluated;

	(*inFile) >> numSensors;
	(*inFile) >> numMotors;

	(*inFile) >> evaluationPeriod;
	(*inFile) >> mutationProbability;

	environments = new MATRIX(inFile);

	int areGenomes;
	(*inFile) >> areGenomes;

	if ( areGenomes )

		Genomes_Load(inFile);
	else
		genomes = NULL;
}

OPTIMIZER::~OPTIMIZER(void) {

	Destroy();
}

void OPTIMIZER::Environments_Change(int numberOfEnvs) {

	int layersPerEnvironment = int( ceil( double(ALPS_NUM_LAYERS) / double(numberOfEnvs) ) );

	int currentLayer;
	int currNumberOfEnvs = 1;
	int currLayersPerEnv = 0;

	for (int j=0;	j<ALPS_NUM_LAYERS;	j++) {

		environments->Set(0,j,currNumberOfEnvs);

		currLayersPerEnv++;
		if ( currLayersPerEnv==layersPerEnvironment ) {

			currLayersPerEnv = 0;
			currNumberOfEnvs++;
		}
	}

	Reset_Genomes();
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

	if ( genomeUnderEvaluation ) {

		genomeUnderEvaluation->Fitness_Set(fitness);

		TryMoveUp(layerIndex);
	}
	else {
		int numberOfEnvs;
		Genome_Get_Next_To_Evaluate(&numberOfEnvs)->Fitness_Set(fitness);
	}

	genomesEvaluated++;
}

void OPTIMIZER::Genome_Discard_Being_Evaluated(void) {

	if ( genomeUnderEvaluation ) {

		delete genomeUnderEvaluation;
		genomeUnderEvaluation = NULL;
	}
}

NEURAL_NETWORK *OPTIMIZER::Genome_Get_Best(int maxEnvironments) {

	double bestFit   = WORST_FITNESS;
	int    bestIndex = -1;

	for (int i=0;	i<ALPS_NUM_LAYERS;	i++)

		for (int j=0;	j<ALPS_GENOMES_PER_LAYER;	j++)

			if (	(environments->Get(0,i)==maxEnvironments) &&

				(Genome_Get(i,j)->Fitness_Get() > bestFit) ) {

				bestFit   = Genome_Get(i,j)->Fitness_Get();

				bestIndex = i*ALPS_GENOMES_PER_LAYER + j;
			}

	return( genomes[bestIndex] );
}

NEURAL_NETWORK *OPTIMIZER::Genome_Get_Curr_To_Evaluate(void) {

	if ( !genomes )

		Genomes_Create();


	if ( !genomeUnderEvaluation )

		return ( Genome_Find_Next_Not_Evaluated() );

	else
		return genomeUnderEvaluation;
}

NEURAL_NETWORK *OPTIMIZER::Genome_Get_Next_To_Evaluate(int *envsToEvaluateIn) {

	NEURAL_NETWORK *genomeToReturn;

	if ( !genomes )

		Genomes_Create();

	if ( genomeUnderEvaluation )

		genomeToReturn = genomeUnderEvaluation;

	else if ( !Genomes_All_Evaluated() )

		genomeToReturn = Genome_Find_Next_Not_Evaluated();
	else
		genomeToReturn = Genome_Spawn();

	(*envsToEvaluateIn) = environments->Get(0,layerIndex);

	return( genomeToReturn );
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

	Genomes_Print();
	printf("\n");
}

void OPTIMIZER::Reset(void) {

	Destroy();

	Initialize();
}

void OPTIMIZER::Reset_Genomes(void) {

/*
	if ( genomeUnderEvaluation ) {

		delete genomeUnderEvaluation;
		genomeUnderEvaluation = NULL;
	}
*/

	for (int i=0;	i<ALPS_NUM_LAYERS;	i++)

		for (int j=0;	j<ALPS_GENOMES_PER_LAYER;	j++)

			Genome_Get(i,j)->Reset();

	Timer_Reset();
}

int OPTIMIZER::Robot_Failed_On_Layer(double currFit) {
		
	int foundVictim = false;
	int currIndex   = ALPS_GENOMES_PER_LAYER-1;

	while (	(!foundVictim) &&
		(currIndex>=0) ) {

		if ( !Genome_Get(layerIndex,currIndex)->Evaluated() ) {

			return( false );
		}
		else if (	Genome_Too_Old(layerIndex,currIndex) ||

				(Genome_Get(layerIndex,currIndex)->Fitness_Get()<currFit) ) {

			foundVictim = true;
		}
		else
			currIndex--;
	}

	return( !foundVictim );
}

void OPTIMIZER::Save(ofstream *outFile) {

	(*outFile) << nextGenomeID		<< "\n";

	(*outFile) << nextVictimOnFirstLayer	<< "\n";

	(*outFile) << layerIndex		<< "\n";

	(*outFile) << genomesEvaluated		<< "\n";

	(*outFile) << numSensors		<< "\n";
	(*outFile) << numMotors			<< "\n";

	(*outFile) << evaluationPeriod		<< "\n";
	(*outFile) << mutationProbability	<< "\n";

	environments->Write(outFile);

	if ( genomes ) {
		(*outFile) << "1"		<< "\n";
		Genomes_Save(outFile);
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

	Destroy_Age_Caps();

	Destroy_Environments();

	Genomes_Destroy();
}

void OPTIMIZER::Destroy_Age_Caps(void) {

	delete ageCaps;
	ageCaps = NULL;
}

void OPTIMIZER::Destroy_Environments(void) {

	delete environments;
	environments = NULL;
}

int OPTIMIZER::FlipCoin(void) {

	return( Rand(0.0,1.0) < 0.5 );
}

int OPTIMIZER::Genome_Age(int layerIndex, int genomeIndex) {

	int evalsCreated = Genome_Get(layerIndex,genomeIndex)->Evals_Get();

	int age		 = genomesEvaluated - evalsCreated;

	    age		 = double(age) / double(populationSize);

	    age		 = 1 + int( floor(age) );

	return( age );
}

NEURAL_NETWORK *OPTIMIZER::Genome_Copy(int parentID) {

	NEURAL_NETWORK *temp = new NEURAL_NETWORK(genomes[parentID]);

	temp->ID_Set(nextGenomeID++);

	return( temp );
}

void OPTIMIZER::Genome_Create_Random(int layerIndex, int genomeIndex) {

	genomes[Genome_Index(layerIndex,genomeIndex)] = 

		new NEURAL_NETWORK(nextGenomeID++,numSensors,numMotors,genomesEvaluated);
}

void OPTIMIZER::Genome_Destroy(int layerIndex, int genomeIndex) {

	delete genomes[Genome_Index(layerIndex,genomeIndex)];

	genomes[Genome_Index(layerIndex,genomeIndex)] = NULL;
}

int OPTIMIZER::Genome_Evaluated(int genomeIndex) {

	return( genomes[genomeIndex]->Evaluated() );
}

NEURAL_NETWORK *OPTIMIZER::Genome_Find_Next_Not_Evaluated(void) {

	int found = false;
	
	int genomeIndex = 0;

	layerIndex = 0;
	int genomesCountedOnLayer = 0;

	while (	(!found) &&

		(genomeIndex < populationSize) ) {

		if ( !genomes[genomeIndex]->Evaluated() ) 

			found = true;
		else {
			genomeIndex++;
			genomesCountedOnLayer++;
			if ( genomesCountedOnLayer == ALPS_GENOMES_PER_LAYER ) {
				layerIndex++;
				genomesCountedOnLayer = 0;
			}
		}
	}

	return( genomes[genomeIndex] );
}

NEURAL_NETWORK *OPTIMIZER::Genome_Get(int layerIndex, int genomeIndex) {

	return( genomes[ (layerIndex*ALPS_GENOMES_PER_LAYER) + genomeIndex ] );
}

int  OPTIMIZER::Genome_Index(int layerIndex, int genomeIndex) {

	return( layerIndex*(ALPS_GENOMES_PER_LAYER) + genomeIndex );
}

void OPTIMIZER::Genome_Load(int layerIndex, int genomeIndex, ifstream *inFile) {

	genomes[Genome_Index(layerIndex,genomeIndex)] = 

		new NEURAL_NETWORK(inFile);
}

void OPTIMIZER::Genome_Print(int layerIndex, int genomeIndex) {

	if ( genomes[Genome_Index(layerIndex,genomeIndex)] ) {

		printf("age: %d ",Genome_Age(layerIndex,genomeIndex));

		genomes[Genome_Index(layerIndex,genomeIndex)]->Print();
	}
}

NEURAL_NETWORK *OPTIMIZER::Genome_Spawn(void) {

	genomeUnderEvaluation = NULL;

	while ( !genomeUnderEvaluation ) {

		layerIndex = Layer_Get_Random();

		if ( layerIndex == 0 )

			Genome_Spawn_From_First_Layer(layerIndex);

		else
			Genome_Spawn_From_Upper_Layer(layerIndex);

		// Some of the time, spawn a genome that competes
		// with genomes on the next highest layer.
		if (	(layerIndex<(ALPS_NUM_LAYERS-1)) &&
			(Rand(0,1)<0.5) )

			layerIndex++;
	}
	
	return( genomeUnderEvaluation );
}

void OPTIMIZER::Genome_Spawn_From_First_Layer(int layerIndex) {

	genomeUnderEvaluation = 

		new NEURAL_NETWORK(nextGenomeID++,numSensors,numMotors,genomesEvaluated);
}

void OPTIMIZER::Genome_Spawn_From_Upper_Layer(int layerIndex) {

	int parentID    = Layer_Get_Parent_Index(layerIndex);

	if ( parentID == -1 )

		return;

	genomeUnderEvaluation = Genome_Copy(parentID);

	genomeUnderEvaluation->Mutate(mutationProbability);
}

int  OPTIMIZER::Genome_Too_Old(int layerIndex, int genomeIndex) {

	// No genomes are too old on the top layer

	if ( Layer_Top(layerIndex) )

		return( false );

	int genomeAge = Genome_Age(layerIndex,genomeIndex);

	int ageCap    = Layer_Age_Cap(layerIndex);

	return( genomeAge > ageCap );
}

int OPTIMIZER::Genomes_All_Evaluated(void) {

	int allEvaluated = true;

	int genomeIndex = 0;

	while ( (allEvaluated) && 

		(genomeIndex < populationSize) ) {

		if ( !Genome_Evaluated(genomeIndex) )

			allEvaluated = false;

		else
			genomeIndex++;
	}

	return( allEvaluated );
}

void OPTIMIZER::Genomes_Create(void) {

	genomes = new NEURAL_NETWORK * [ ALPS_NUM_LAYERS * ALPS_GENOMES_PER_LAYER ];

	for (int i=0;	i<ALPS_NUM_LAYERS;	i++)

		for (int j=0;	j<ALPS_GENOMES_PER_LAYER;	j++)

			Genome_Create_Random(i,j);
}

void OPTIMIZER::Genomes_Destroy(void) {

	for (int i=0;	i<ALPS_NUM_LAYERS;	i++)

		for (int j=0;	j<ALPS_GENOMES_PER_LAYER;	j++)

			Genome_Destroy(i,j);

	delete[] genomes;
	genomes = NULL;
}

void OPTIMIZER::Genomes_Load(ifstream *inFile) {

	genomes = new NEURAL_NETWORK * [ ALPS_NUM_LAYERS * ALPS_GENOMES_PER_LAYER ];

	for (int i=0;	i<ALPS_NUM_LAYERS;	i++)

		for (int j=0;	j<ALPS_GENOMES_PER_LAYER;	j++)

			Genome_Load(i,j,inFile);
}

void OPTIMIZER::Genomes_Print(void) {

	for (int i=(ALPS_NUM_LAYERS-1);	i>=0;	i--) {

		printf("Layer: %d\t"   ,i);

		printf("Env: %d\t"    ,Layer_Environment(i) );

		printf("Age cap: %d\t",Layer_Age_Cap(i) );


		for (int j=0;	j<ALPS_GENOMES_PER_LAYER;	j++)

			Genome_Print(i,j);

		printf("\n");

		if (	(i>0) &&
			(Layer_Environment(i)!=Layer_Environment(i-1)) )

			printf("\n");
	}

	printf("\n");
}

void OPTIMIZER::Genomes_Save(ofstream *outFile) {

	for (int i=0;	i<populationSize;	i++)

		genomes[i]->Save(outFile);
}

void OPTIMIZER::Initialize(void) {

	genomes = NULL;
	genomeUnderEvaluation = NULL;

	nextGenomeID = 0;

	populationSize = ALPS_NUM_LAYERS * ALPS_GENOMES_PER_LAYER;

	nextVictimOnFirstLayer = 0;

	genomesEvaluated = 0;

	Initialize_Age_Caps();

	Initialize_Environments();

	evaluationPeriod = STARTING_EVALUATION_TIME;

	mutationProbability = MUTATION_PROBABILITY;

	Timer_Reset();
}

void OPTIMIZER::Initialize_Age_Caps(void) {

	ageCaps = new MATRIX(1,ALPS_NUM_LAYERS);

/*
	// Fibonacci
	ageCaps->Set(0,0,1);

	if ( ALPS_NUM_LAYERS < 2 )
		return;

	ageCaps->Set(0,1,2);

	for (int j=2;	j<ALPS_NUM_LAYERS;	j++)

		ageCaps->Set(0,j, ageCaps->Get(0,j-2) + ageCaps->Get(0,j-1) );
*/

	for (int j=0;	j<ALPS_NUM_LAYERS;	j++)

		ageCaps->Set(0,j,10*j);
}

void OPTIMIZER::Initialize_Environments(void) {

	environments = new MATRIX(1,ALPS_NUM_LAYERS,1.0);
}

int  OPTIMIZER::Layer_Age_Cap(int layerIndex) {

	return( int( ageCaps->Get(0,layerIndex) ) );
}

int OPTIMIZER::Layer_All_Evaluated(int layerIndex) {

	int currGenome			= 0;

	int allEvaluated		= true;
	
	while (	(allEvaluated) &&

		(currGenome<(ALPS_GENOMES_PER_LAYER)) ) {

		if ( !Genome_Get(layerIndex,currGenome)->Evaluated() )

			allEvaluated = false;

		else

			currGenome++;

	}

	return( allEvaluated );
}

int OPTIMIZER::Layer_All_Of_Age(int layerIndex) {

	int currGenome			= 0;

	int allOfAge			= true;
	
	while (	(allOfAge) &&

		(currGenome<(ALPS_GENOMES_PER_LAYER)) ) {

		if ( Genome_Too_Old(layerIndex,currGenome) )

			allOfAge = false;

		else

			currGenome++;

	}

	return( allOfAge );
}

int  OPTIMIZER::Layer_Environment(int layerIndex) {

	return( int( environments->Get(0,layerIndex) ) );
}

int  OPTIMIZER::Layer_Find_Clone(int layerIndex, double aggressorFitness) {

	int firstGenomeOnLayer		= Layer_First_Index(layerIndex);

	int lastGenomeOnLayer		= Layer_Last_Index(layerIndex);

	int currentIndex		= Rand(firstGenomeOnLayer,lastGenomeOnLayer);

	int genomesChecked		= 0;

	int found			= false;

	while (	(!found) &&

		(genomesChecked < ALPS_GENOMES_PER_LAYER) ) {

		if ( 	genomes[currentIndex] &&
			genomes[currentIndex]->Evaluated() &&

			genomes[currentIndex]->Fitness_Equal_To(aggressorFitness) )

			found = true;

		else {
			currentIndex++;

			if ( currentIndex > lastGenomeOnLayer )

				currentIndex = firstGenomeOnLayer;

			genomesChecked++;
		}
	}

	if ( !found )

		return( -1 );

	else
		return( currentIndex );
}

int  OPTIMIZER::Layer_Find_Dominated(int layerIndex, double aggressorFitness) {

	int firstGenomeOnLayer		= Layer_First_Index(layerIndex);

	int lastGenomeOnLayer		= Layer_Last_Index(layerIndex);

	int currentIndex		= Rand(firstGenomeOnLayer,lastGenomeOnLayer);

	int genomesChecked		= 0;

	int found			= false;

	while (	(!found) &&

		(genomesChecked < ALPS_GENOMES_PER_LAYER) ) {

		if ( 	genomes[currentIndex] &&
			genomes[currentIndex]->Evaluated() &&

			genomes[currentIndex]->Fitness_Worse_Than(aggressorFitness) )

			found = true;

		else {
			currentIndex++;

			if ( currentIndex > lastGenomeOnLayer )

				currentIndex = firstGenomeOnLayer;

			genomesChecked++;
		}
	}

	if ( !found )

		return( -1 );

	else
		return( currentIndex );
}

int  OPTIMIZER::Layer_Find_Too_Old(int layerIndex) {

	int firstGenomeOnLayer		= Layer_First_Index(layerIndex);

	int lastGenomeOnLayer		= Layer_Last_Index(layerIndex);

	int currentIndex		= Rand(firstGenomeOnLayer,lastGenomeOnLayer);

	int genomesChecked		= 0;

	int found			= false;

	while (	(!found) &&

		(genomesChecked < ALPS_GENOMES_PER_LAYER) ) {

		if ( 	genomes[currentIndex] &&
			genomes[currentIndex]->Evaluated() &&

			Genome_Too_Old(layerIndex,currentIndex-firstGenomeOnLayer) )

			found = true;

		else {
			currentIndex++;

			if ( currentIndex > lastGenomeOnLayer )

				currentIndex = firstGenomeOnLayer;

			genomesChecked++;
		}
	}

	if ( !found )

		return( -1 );

	else
		return( currentIndex );
}

int  OPTIMIZER::Layer_Find_Victim(int layerIndex, double aggressorFitness) {

	int victim;

	if ( layerIndex == 0 ) {

		victim = nextVictimOnFirstLayer;

		nextVictimOnFirstLayer++;

		if ( nextVictimOnFirstLayer == ALPS_GENOMES_PER_LAYER )

			nextVictimOnFirstLayer = 0;

	}
	else {
		victim = Layer_Find_Too_Old(layerIndex);

		if ( victim == -1 )

			victim = Layer_Find_Clone(layerIndex,aggressorFitness);

		if ( victim == -1 )

			victim = Layer_Find_Dominated(layerIndex,aggressorFitness);
	}

	return( victim );
}

int  OPTIMIZER::Layer_First_Index(int layerIndex) {

	return( layerIndex * ALPS_GENOMES_PER_LAYER );
}

int  OPTIMIZER::Layer_Get_Parent_Index(int layerIndex) {

	return( Layer_Most_Fit_Genome_Index(layerIndex) );

//	return( Layer_Random_Genome_Index(layerIndex) );
}

int OPTIMIZER::Layer_Get_Random(void) {

	return( RandInt(0,ALPS_NUM_LAYERS-1) );
}

int  OPTIMIZER::Layer_Last_Index(int layerIndex) {

	return( (layerIndex * ALPS_GENOMES_PER_LAYER) + 

		(ALPS_GENOMES_PER_LAYER-1) );
}

NEURAL_NETWORK *OPTIMIZER::Layer_Most_Fit_Genome(int layerIndex) {

	return( genomes[ Layer_Most_Fit_Genome_Index(layerIndex) ] );
}

int OPTIMIZER::Layer_Most_Fit_Genome_Index(int layerIndex) {

	int firstGenomeOnLayer		= Layer_First_Index(layerIndex);

	int lastGenomeOnLayer		= Layer_Last_Index(layerIndex);

	double highestFitness		= WORST_FITNESS;

	int	localChampIndex		= -1;

	for (int i=firstGenomeOnLayer;	i<=lastGenomeOnLayer;	i++) {

		if ( 	genomes[i] &&

			genomes[i]->Evaluated() &&

			(genomes[i]->Fitness_Get() > highestFitness) ) {

			highestFitness = genomes[i]->Fitness_Get();

			localChampIndex = i;
		}
	}

	return( localChampIndex );
}

int OPTIMIZER::Layer_Random_Genome_Index(int layerIndex) {

	int firstGenomeOnLayer		= Layer_First_Index(layerIndex);

	int lastGenomeOnLayer		= Layer_Last_Index(layerIndex);

	int genomeIndex			= RandInt(firstGenomeOnLayer,lastGenomeOnLayer);

	return( genomeIndex );
}

void OPTIMIZER::Layer_Sort(int layerIndex) {

	int firstGenomeOnLayer		= Layer_First_Index(layerIndex);

	int lastGenomeOnLayer		= Layer_Last_Index(layerIndex);

	int swapped = true;

	while ( swapped ) {

		swapped = false;

		for (int i=firstGenomeOnLayer+1;	i<=lastGenomeOnLayer;	i++) {

			if ( genomes[i]->Beats(genomes[i-1]) ) {

				NEURAL_NETWORK *temp = genomes[i-1];
				genomes[i-1] = genomes[i];
				genomes[i] = temp;
				temp = NULL;
				swapped = true;
			}
		}
	}
}

int    OPTIMIZER::Layer_Top(int layerIndex) {

	return( layerIndex == (ALPS_NUM_LAYERS-1) );
}

double OPTIMIZER::Rand(double min, double max) {

	// Return a random value in [min,max] with
	// a uniform distribution.

        double zeroToOne = ((double)rand()) / RAND_MAX;
        double returnVal;

        returnVal = (zeroToOne * (max-min)) + min;
        return returnVal;
}

int OPTIMIZER::RandInt(int min, int max) {

	if ( min == max )
		return( min );
	else {
		int val = (rand() % (max-min+1)) + min;
		if ( val > max )
			val = max;
		if ( val < min )
			val = min;	
		return( val );
	}
}

void OPTIMIZER::TryMoveUp(int layerIndx) {

	int victimIndex = Layer_Find_Victim(layerIndx,genomeUnderEvaluation->Fitness_Get());

	if ( victimIndex > -1 ) {

		if ( Layer_Top(layerIndx) ) {

			delete genomes[victimIndex];
			genomes[victimIndex] = genomeUnderEvaluation;
			genomeUnderEvaluation = NULL;

		}
		else {

			NEURAL_NETWORK *temp = genomes[victimIndex];
			genomes[victimIndex] = genomeUnderEvaluation;
			genomeUnderEvaluation = temp;
			temp = NULL;

			if ( 	environments->Get(0,layerIndx) == 
				environments->Get(0,layerIndx+1) )
			// If the layer above hosts the same number of environments
			// in which the displaced genome was evaluated, it can compete
			// against the genomes on that layer.
 
				TryMoveUp(layerIndx+1);

			else {
			// The displaced genome must be re-evaluated against
			// the greater number of environments found on the
			// layer above.

				genomeUnderEvaluation->Reset();
				layerIndex++;
			}
		}

		Layer_Sort(layerIndx);
	}
	else {
		delete genomeUnderEvaluation;
		genomeUnderEvaluation = NULL;
	}
}

#endif
