#include "math.h"
#include "stdio.h"

#ifndef _NEURAL_NETWORK_CPP
#define _NEURAL_NETWORK_CPP

#include "neuralNetwork.h"
#include "matrix.h"

extern double		NN_TAUS_MIN;
extern double		NN_TAUS_MAX;

extern double		NN_WEIGHTS_MIN;
extern double		NN_WEIGHTS_MAX;

extern double		NN_OMEGAS_MIN;
extern double		NN_OMEGAS_MAX;

extern double		NN_SENSORWEIGHTS_MIN;
extern double		NN_SENSORWEIGHTS_MAX;

extern double		STEP_SIZE;

extern double		SENSOR_CHANGE_THRESHOLD_FOR_STOPPING_EVALUATION;

NEURAL_NETWORK::NEURAL_NETWORK(	MATRIX *t, MATRIX *w,
                               	MATRIX *o, MATRIX *sW,
				int nS,	int nM) {

        taus = new MATRIX(t);

        weights = new MATRIX(w);
	//weights = NULL;

        omegas = new MATRIX(o);

        sensorWeights = new MATRIX(sW);

        numSensors = nS;
        numMotors = nM;

	Initialize();
}

NEURAL_NETWORK::NEURAL_NETWORK(	NEURAL_NETWORK *other ) {

        taus = new MATRIX(		other->taus);

        weights = new MATRIX(		other->weights);
	//weights = NULL;

        omegas = new MATRIX(		other->omegas);

        sensorWeights = new MATRIX(	other->sensorWeights);

	numSensors = other->numSensors;
	numMotors  = other->numMotors;

	Initialize();
}

NEURAL_NETWORK::NEURAL_NETWORK(ifstream *inFile) {

	int isTaus;
	(*inFile) >> isTaus;
	if ( isTaus ) {
		taus = new MATRIX(inFile);
	}
	else
		taus = NULL;

	int isWeights;
	(*inFile) >> isWeights;
	if ( isWeights ) {
		weights = new MATRIX(inFile);
	}
	else
		weights = NULL;

	int isOmegas;
	(*inFile) >> isOmegas;
	if ( isOmegas ) {
		omegas = new MATRIX(inFile);
	}
	else
		omegas = NULL;

	int isSensorWeights;
	(*inFile) >> isSensorWeights;
	if ( isSensorWeights ) {
		sensorWeights = new MATRIX(inFile);
	}
	else
		sensorWeights = NULL;

        (*inFile) >> numSensors;
        (*inFile) >> numMotors;
        (*inFile) >> numNeurons;

	Initialize();
}

NEURAL_NETWORK::~NEURAL_NETWORK(void) {

	if ( taus ) {
		delete taus;
		taus = NULL;
	}

	if ( weights ) {
		delete weights;
		weights = NULL;
	}

	if ( omegas ) {
		delete omegas;
		omegas = NULL;
	}

	if ( sensorWeights ) {
		delete sensorWeights;
		sensorWeights = NULL;
	}

	if ( sensors ) {
		delete sensors;
		sensors = NULL;
	}

	if ( sensorsPrevious ) {
		delete sensorsPrevious;
		sensorsPrevious = NULL;
	}

	if ( sensorsChange ) {
		delete sensorsChange;
		sensorsChange = NULL;
	}

	if ( neurons ) {
		delete neurons;
		neurons = NULL;
	}

	if ( currY ) {
		delete currY;
		currY = NULL;
	}

	if ( kn1 ) {
		delete kn1;
		kn1 = NULL;
	}

	if ( kn2 ) {
		delete kn2;
		kn2 = NULL;
	}

	if ( kn3 ) {
		delete kn3;
		kn3 = NULL;
	}

	if ( kn4 ) {
		delete kn4;
		kn4 = NULL;
	}

}

double NEURAL_NETWORK::Get_Motor_Neuron_Value(int motorIndex) {

	// It is assumed by the calling function 
	// that the neuron value is in the range [-1,+1]

	double rawValue = omegas->vals[motorIndex] - neurons->vals[motorIndex];

	double afterActivationFunction = tanh(rawValue);

	return( afterActivationFunction );
}

void NEURAL_NETWORK::Mutate(double mutationProbability) {

	if ( taus )
		taus->Mutate(	      mutationProbability, NN_TAUS_MIN,		NN_TAUS_MAX);

	if ( weights )
		weights->Mutate(      mutationProbability, NN_WEIGHTS_MIN,	NN_WEIGHTS_MAX);

	if ( omegas )
		omegas->Mutate(	      mutationProbability, NN_OMEGAS_MIN,	NN_OMEGAS_MAX);

	if ( sensorWeights )
		sensorWeights->Mutate(	mutationProbability, 
					NN_SENSORWEIGHTS_MIN,	NN_SENSORWEIGHTS_MAX);
}

void NEURAL_NETWORK::Print(void) {

	if ( taus )

		taus->Print();
}

void NEURAL_NETWORK::Save(ofstream *outFile) {

	if ( taus ) {
		(*outFile) << "1\n";
		taus->Write(outFile);
	}
	else
		(*outFile) << "0\n";

	if ( weights ) {
		(*outFile) << "1\n";
		weights->Write(outFile);
	}
	else
		(*outFile) << "0\n";

	if ( omegas ) {
		(*outFile) << "1\n";
		omegas->Write(outFile);
	}
	else
		(*outFile) << "0\n";

	if ( sensorWeights ) {
		(*outFile) << "1\n";
		sensorWeights->Write(outFile);
	}
	else
		(*outFile) << "0\n";

        (*outFile) << numSensors << "\n";
        (*outFile) << numMotors  << "\n";
        (*outFile) << numNeurons << "\n";
}



int NEURAL_NETWORK::Sensors_No_Longer_Changing(void) {

	return( 	sensors->AbsoluteDifference(sensorsPrevious) < 	

			SENSOR_CHANGE_THRESHOLD_FOR_STOPPING_EVALUATION );
}

void NEURAL_NETWORK::Set_Sensor_Neuron(int sensorIndex, double sensorValue) {

	sensorsPrevious->vals[sensorIndex] = 
		sensors->vals[sensorIndex];

	sensors->vals[sensorIndex] = sensorValue;

	sensorsChange->vals[sensorIndex] = 

		sensors->vals[sensorIndex] - 
		sensorsPrevious->vals[sensorIndex];
}

void NEURAL_NETWORK::Update(void){

	int i;

	for (i=0;i<numNeurons;i++)
		currY->vals[i] = neurons->vals[i];
	UpdateNeurons(kn1,currY);

	for (i=0;i<numNeurons;i++)
		currY->vals[i] = neurons->vals[i] + (STEP_SIZE*kn1->vals[i]/2.0 );
	UpdateNeurons(kn2,currY);

	for (i=0;i<numNeurons;i++)
		currY->vals[i] = neurons->vals[i] + (STEP_SIZE*kn2->vals[i]/2.0 );
	UpdateNeurons(kn3,currY);

	for (i=0;i<numNeurons;i++)
		currY->vals[i] = neurons->vals[i] + (STEP_SIZE*kn3->vals[i] );

	UpdateNeurons(kn4,currY);

	for (i=0;i<numNeurons;i++) {

		neurons->vals[i] = neurons->vals[i] +
					(STEP_SIZE/6.0)*(
					kn1->vals[i] +
					2*kn2->vals[i] +
					2*kn3->vals[i] +
					kn4->vals[i]
					);

		if ( neurons->vals[i]>100.0)
			neurons->vals[i] = 100.0;

		if ( neurons->vals[i]<-100.0)
			neurons->vals[i] = -100.0;

	}
}

// ---------------------------------------------- Private methods

void NEURAL_NETWORK::Initialize(void) {

        sensors 	= new MATRIX(1,numSensors,0.0);
        sensorsPrevious = new MATRIX(1,numSensors,0.0);
	sensorsChange	= new MATRIX(1,numSensors,0.0);

        numNeurons = omegas->width;

        neurons = new MATRIX(1,numNeurons,0.0);

        currY = new MATRIX(numNeurons,1,0);
        kn1 = new MATRIX(numNeurons,1,0);
        kn2 = new MATRIX(numNeurons,1,0);
        kn3 = new MATRIX(numNeurons,1,0);
        kn4 = new MATRIX(numNeurons,1,0);
}

void NEURAL_NETWORK::UpdateNeurons(MATRIX *kn, MATRIX *currY) {

	for (int i=0;i<numNeurons;i++) {

		kn->vals[i] = -currY->vals[i];


		for (int j=0;j<numNeurons;j++)
			kn->vals[i] = kn->vals[i] + 
					weights->vals[i*weights->width+j] *
					//1/( 1 + exp( omegas->vals[j] - currY->vals[j] ) );
					tanh(omegas->vals[j] - currY->vals[j]);

		// Influence of current sensor values
		for (int j=0;j<numSensors;j++)
			kn->vals[i] = kn->vals[i] + 
					sensorWeights->vals[j*sensorWeights->width+i]*
					sensors->vals[j];

		// Influence of previous sensor values
		for (int j=0;j<numSensors;j++)
			kn->vals[i] = kn->vals[i] + 
					sensorWeights->vals[(numSensors+j)*sensorWeights->width+i]*
					sensorsPrevious->vals[j];

		// Influence of change in sensor values
		for (int j=0;j<numSensors;j++)
			kn->vals[i] = kn->vals[i] + 
					sensorWeights->vals[(2*numSensors+j)*sensorWeights->width+i]*
					sensorsChange->vals[j];

		kn->vals[i] = kn->vals[i] / taus->vals[i];
	}
}

#endif

