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
	int timer;
	int evaluationPeriod;
	NEURAL_NETWORK *child;
	NEURAL_NETWORK *parent;
private:

	double childFitness;
	double parentFitness;

	double childSensorSum;
	double parentSensorSum;

	int numSensors;
	int numMotors;

	int evalsSinceLastChampFound;

	int childTime;
	int parentTime;

	double previousBestFitness;
	double mutationProbability;

public:
	OPTIMIZER(int numberOfSensors, int numberOfMotors);
	OPTIMIZER(ifstream *inFile);
	~OPTIMIZER(void);
	int	Current_Robot_Failed(double currentFitness);
	void    EvaluationPeriod_Decrease(void);
	void    EvaluationPeriod_Increase(void);
	void	Fitness_Receive(double fitness);
	NEURAL_NETWORK *Genome_Get_Best(void);
	NEURAL_NETWORK *Genome_Get_Curr_To_Evaluate(void);
	NEURAL_NETWORK *Genome_Get_Next_To_Evaluate(void);
	void	Load(ifstream *inFile);
	void    MutationProbability_Decrease(void);
	void    MutationProbability_Increase(void);
	void    Print(void);
	void	PrintParentChild(void);
	void	Reproduce(void);
	void	Reset(void);
	void	Reset_But_Keep_Best(void);
	void	Save(ofstream *outFile);
	void	SensorSum_Receive(double sensorSum);
	int     SensorSums_Match(void);
	int	Time_Elapsed(void);
	void	Timer_Reset(void);
	void	Timer_Update(void);

private:
	void    Destroy(void);
	int	FlipCoin(void);
	void	Initialize(void);
	double	Rand(double min, double max);
};

#endif
