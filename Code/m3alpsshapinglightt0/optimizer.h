/* 

The algorithm that optimizes the robots. This can be replaced with whatever optimization algorithm the user prefers. This one instantiates a hill climber. 

The genomes are encoded as two-dimensional matrices with each element
in the range [0,1].

*/

#ifndef _OPTIMIZER_H
#define _OPTIMIZER_H

#include "matrix.h"
#include "neuralNetwork.h"

class OPTIMIZER {

public:
	int 		timer;
	int 		evaluationPeriod;

private:
	long 		nextGenomeID;

	NEURAL_NETWORK **genomes;

	NEURAL_NETWORK *genomeUnderEvaluation;

	int		populationSize;

	int		nextVictimOnFirstLayer;

	int		layerIndex;

	int		genomesEvaluated;

	MATRIX		*ageCaps;
	MATRIX		*environments;

	int 		numSensors;
	int 		numMotors;

	double 		mutationProbability;

public:
	OPTIMIZER(int numberOfSensors, int numberOfMotors);
	OPTIMIZER(ifstream *inFile);
	~OPTIMIZER(void);
	void		Environments_Change(int numberOfEnvs);
	void    	EvaluationPeriod_Decrease(void);
	void    	EvaluationPeriod_Increase(void);
	void		Fitness_Receive(double fitness);
	void		Genome_Discard_Being_Evaluated(void);
	NEURAL_NETWORK *Genome_Get_Best(int maxEnvironments);
	NEURAL_NETWORK *Genome_Get_Curr_To_Evaluate(void);
	NEURAL_NETWORK *Genome_Get_Next_To_Evaluate(int *envsToEvaluateIn);
	void		Load(ifstream *inFile);
	void    	MutationProbability_Decrease(void);
	void    	MutationProbability_Increase(void);
	void    	Print(void);
	void		Reset(void);
	void		Reset_Genomes(void);
	int		Robot_Failed_On_Layer(double currFit);
	void		Save(ofstream *outFile);
	int		Time_Elapsed(void);
	void		Timer_Reset(void);
	void		Timer_Update(void);

private:
	void    	Destroy(void);
	void 		Destroy_Age_Caps(void);
	void		Destroy_Environments(void);
	int		FlipCoin(void);
	int 		Genome_Age(int layerIndex, int genomeIndex);
	NEURAL_NETWORK *Genome_Copy(int parentID);
	void    	Genome_Create_Random(int layerIndex, int genomeIndex);
	void    	Genome_Destroy(int layerIndex, int genomeIndex);
	int  		Genome_Evaluated(int genomeIndex);
	NEURAL_NETWORK *Genome_Find_Next_Not_Evaluated(void);
	NEURAL_NETWORK *Genome_Get(int layerIndex, int genomeIndex);
	int		Genome_Index(int layerIndex, int genomeIndex);
	void 		Genome_Load(int layerIndex, int genomeIndex, ifstream *inFile);
	void		Genome_Print(int layerIndex, int genomeIndex);
	NEURAL_NETWORK *Genome_Spawn(void);
	void    	Genome_Spawn_From_First_Layer(int layerIndex);
	void    	Genome_Spawn_From_Upper_Layer(int layerIndex);
	int  		Genome_Too_Old(int layerIndex, int genomeIndex);
	int		Genomes_All_Evaluated(void);
	void		Genomes_Create(void);
	void		Genomes_Destroy(void);
	void		Genomes_Load(ifstream *inFile);
	void    	Genomes_Print(void);
	void 		Genomes_Save(ofstream *outFile);
	void		Initialize(void);
	void 		Initialize_Age_Caps(void);
	void		Initialize_Environments(void);
	int  		Layer_Age_Cap(int layerIndex);
	int		Layer_All_Evaluated(int layerIndex);
	int		Layer_All_Of_Age(int layerIndex);
	int  		Layer_Environment(int layerIndex);
	int  		Layer_Find_Clone(int layerIndex, double aggressorFitness);
	int		Layer_Find_Dominated(int layerIndex, double aggressorFitness);
	int		Layer_Find_Too_Old(int layerIndex);
	int  		Layer_Find_Victim(int layerIndex, double aggressorFitness);
	int  		Layer_First_Index(int layerIndex);
	int  		Layer_Get_Parent_Index(int layerIndex);
	int     	Layer_Get_Random(void);
	int  		Layer_Last_Index(int layerIndex);
	NEURAL_NETWORK *Layer_Most_Fit_Genome(int layerIndex);
	int		Layer_Most_Fit_Genome_Index(int layerIndex);
	int		Layer_Random_Genome_Index(int layerIndex);
	void 		Layer_Sort(int layerIndex);
	int		Layer_Top(int layerIndex);
	double		Rand(double min, double max);
	int     	RandInt(int min, int max);
	void 		TryMoveUp(int layerIndx);
};

#endif
