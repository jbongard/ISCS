// A neural network controls transforms the incoming sensor signals
// into outgoing motor commands.

#ifndef _NEURAL_NETWORK_H
#define _NEURAL_NETWORK_H

#include "matrix.h"

class NEURAL_NETWORK {

public:

	MATRIX  *weights;
	MATRIX  *nodeValues;
	MATRIX  *temp;
	int     numMotors;
	int	numNodes;
	int	inAFixedAttractor;

private:
	int	numConnections;
	MATRIX  *edgeList;

public:
	NEURAL_NETWORK(	int nM );
	NEURAL_NETWORK( NEURAL_NETWORK *other );
	NEURAL_NETWORK(ifstream *inFile);
	~NEURAL_NETWORK();
	double Density(void);
	double Get_Motor_Neuron_Value(int motorIndex);
	void   Mutate(double mutationProbability);
	void   Print(void);
	void   Save(ofstream *outFile);
	void   Sensors_Set(int environmentIndex);
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
	void	Weights_Initialize_Randomly(void);
	void	Weights_Initialize_NumIncomingConnections(int nic);
};

#endif
