// A neural network controls transforms the incoming sensor signals
// into outgoing motor commands.

#ifndef _NEURAL_NETWORK_H
#define _NEURAL_NETWORK_H

#include "matrix.h"

class NEURAL_NETWORK {

public:

	MATRIX  *weights;
	MATRIX  *nodeValues;
	MATRIX  *sensorValues;
	MATRIX  *temp;
	int	numSensors;
	int     numMotors;
	int	numNodes;
	int	inAFixedAttractor;

private:
	int	numConnections;
	MATRIX  *edgeList;
	int	ID;
	double  fitness;
	int	evaluated;
	int	genomesEvaluatedWhenCreated;

public:
	NEURAL_NETWORK(	int myID, int nS, int nM , int evals );
	NEURAL_NETWORK( NEURAL_NETWORK *other );
	NEURAL_NETWORK(ifstream *inFile);
	~NEURAL_NETWORK();
	int    Age_Get(int evalsCurrent);
	int    Beats(NEURAL_NETWORK *other);
	double Density(void);
	int    Evaluated(void);
	int    Evals_Get(void);
	int    Fitness_Equal_To(double fit);
	double Fitness_Get(void);
	void   Fitness_Set(double fit);
	int    Fitness_Worse_Than(double fit);
	double Get_Motor_Neuron_Value(int motorIndex);
	int    ID_Get(void);
	void   ID_Set(int myID);
	void   Mutate(double mutationProbability);
	void   Print(void);
	void   Reset(void);
	void   Save(ofstream *outFile);
	void   Sensor_Set(int sensorIndex, double sensorValue);
	void   Update(int timeStep);

private:
	void    Connection_Add(int nodeIndex);
	void    Connection_Remove(int nodeIndex);
	void    EdgeList_Create(void);
	int	FlipCoin(void);
	double	Incoming_Connections(int nodeIndex);
	void	Initialize(void);
	void	Mutate(int i, int j);
	int	Mutate_Biased(double mutationProbability);
	double  Rand(double min, double max);
	int 	RandInt(int min, int max);
	double  Scale(double value, double min1, double max1,
                                    double min2, double max2);
	void	Weights_Initialize_NumIncomingConnections(int nic);
};

#endif
