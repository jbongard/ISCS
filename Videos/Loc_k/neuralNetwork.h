// A neural network controls transforms the incoming sensor signals
// into outgoing motor commands.

#ifndef _NEURAL_NETWORK_H
#define _NEURAL_NETWORK_H

#include "matrix.h"

class NEURAL_NETWORK {

public:

        MATRIX *taus;
        MATRIX *weights;
        MATRIX *omegas;
        MATRIX *sensorWeights;

        MATRIX *sensors;
	MATRIX *sensorsPrevious;
	MATRIX *sensorsChange;

        int   numSensors;
        int   numMotors;
        int   numNeurons;
        MATRIX *neurons;

        MATRIX *currY;
        MATRIX *kn1;
        MATRIX *kn2;
        MATRIX *kn3;
        MATRIX *kn4;

public:
	NEURAL_NETWORK(	MATRIX *t, MATRIX *w,
			MATRIX *o, MATRIX *sW,
			int nS,	int nM);
	NEURAL_NETWORK( NEURAL_NETWORK *other );
	NEURAL_NETWORK(ifstream *inFile);
	~NEURAL_NETWORK();
	double Get_Motor_Neuron_Value(int motorIndex);
	void   Mutate(double mutationProbability);
	void   Print(void);
	void   Save(ofstream *outFile);
	int    Sensors_No_Longer_Changing(void);
	void   Set_Sensor_Neuron(int sensorIndex, double sensorValue);
	void   Update(void);

private:
	void   Initialize(void);
	void   UpdateNeurons(MATRIX *kn, MATRIX *currY);
};

#endif
