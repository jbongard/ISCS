#include "math.h"
#include "stdio.h"

#ifndef _NEURAL_NETWORK_CPP
#define _NEURAL_NETWORK_CPP

#include "neuralNetwork.h"
#include "matrix.h"

extern int	NODES_PER_MOTOR;

NEURAL_NETWORK::NEURAL_NETWORK(	int nM ) {

	numMotors 	= nM;

	Initialize();

	weights		= new MATRIX(numNodes,numNodes,0);
	
//	Weights_Initialize_Randomly();
	Weights_Initialize_NumIncomingConnections(1);
}

NEURAL_NETWORK::NEURAL_NETWORK(	NEURAL_NETWORK *other ) {

        weights 	= new MATRIX(other->weights);

	numMotors  	= other->numMotors;

	Initialize();
}

NEURAL_NETWORK::NEURAL_NETWORK(ifstream *inFile) {

	int isWeights;
	(*inFile) >> isWeights;
	if ( isWeights ) {
		weights = new MATRIX(inFile);
	}
	else
		weights = NULL;

        (*inFile) >> numMotors;

	Initialize();
}

NEURAL_NETWORK::~NEURAL_NETWORK(void) {

	if ( weights ) {
		delete weights;
		weights = NULL;
	}

	if ( nodeValues ) {
		delete nodeValues;
		nodeValues = NULL;
	}

	if ( temp ) {
		delete temp;
		temp = NULL;
	}

	if ( edgeList ) {
		delete edgeList;
		edgeList = NULL;
	}
}

double NEURAL_NETWORK::Density(void) {

	double density = 0.0;

	for (int i=0;	i<numNodes;	i++)

		for (int j=0;	j<numNodes;	j++)

			if ( weights->Get(i,j)!=0 )

				density++;

	density = density / double(numNodes*numNodes);

	return( density );
}

double NEURAL_NETWORK::Get_Motor_Neuron_Value(int motorIndex) {

	// It is assumed by the calling function 
	// that the neuron value is in the range [-1,+1]

	if ( NODES_PER_MOTOR==2 ) {

		int firstBit =  nodeValues->vals[NODES_PER_MOTOR*motorIndex];
		if ( firstBit==-1 )
			firstBit=0;

		int secondBit = nodeValues->vals[NODES_PER_MOTOR*motorIndex+1];
		if ( secondBit==-1 )
			secondBit=0;

		// Uses gray codes
		double rawValue;
		     if ( (firstBit==0) && (secondBit==0) )
			rawValue = 0;

		else if ( (firstBit==0) && (secondBit==1) )
			rawValue = 1;

		else if ( (firstBit==1) && (secondBit==1) )
			rawValue = 2;

		else if ( (firstBit==1) && (secondBit==0) )
			rawValue = 3;

		double scaledValue = Scale(rawValue,0.0,3.0,-1.0,+1.0);

		return( scaledValue );
	}
	else if ( NODES_PER_MOTOR==3 ) {

		/*
		int firstBit =  nodeValues->vals[NODES_PER_MOTOR*motorIndex];
		if ( firstBit==-1 )
			firstBit=0;

		int secondBit = nodeValues->vals[NODES_PER_MOTOR*motorIndex+1];
		if ( secondBit==-1 )
			secondBit=0;

		int thirdBit = nodeValues->vals[NODES_PER_MOTOR*motorIndex+2];
		if ( thirdBit==-1 )
			thirdBit=0;

		// Uses gray codes
		double rawValue;
		     if ( (firstBit==0) && (secondBit==0) && (thirdBit==0) )
			rawValue = 0;

		else if ( (firstBit==0) && (secondBit==0) && (thirdBit==1) )
			rawValue = 1;

		else if ( (firstBit==0) && (secondBit==1) && (thirdBit==1) )
			rawValue = 2;

		else if ( (firstBit==0) && (secondBit==1) && (thirdBit==0) )
			rawValue = 3;


		else if ( (firstBit==1) && (secondBit==1) && (thirdBit==0) )
			rawValue = 4;

		else if ( (firstBit==1) && (secondBit==1) && (thirdBit==1) )
			rawValue = 5;

		else if ( (firstBit==1) && (secondBit==0) && (thirdBit==1) )
			rawValue = 6;

		else if ( (firstBit==1) && (secondBit==0) && (thirdBit==0) )
			rawValue = 7;

		double scaledValue = Scale(rawValue,0.0,7.0,-1.0,+1.0);
*/

		// The third bit is a hidden node
		int firstBit =  nodeValues->vals[NODES_PER_MOTOR*motorIndex];
		if ( firstBit==-1 )
			firstBit=0;

		int secondBit = nodeValues->vals[NODES_PER_MOTOR*motorIndex+1];
		if ( secondBit==-1 )
			secondBit=0;

		// Uses gray codes
		double rawValue;
		     if ( (firstBit==0) && (secondBit==0) )
			rawValue = 0;

		else if ( (firstBit==0) && (secondBit==1) )
			rawValue = 1;

		else if ( (firstBit==1) && (secondBit==1) )
			rawValue = 2;

		else if ( (firstBit==1) && (secondBit==0) )
			rawValue = 3;

		double scaledValue = Scale(rawValue,0.0,3.0,-1.0,+1.0);


		return( scaledValue );
	}
	// NODES_PER_MOTOR>3
	else  {

		// The third and fourth bits are hidden nodes
		int firstBit =  nodeValues->vals[NODES_PER_MOTOR*motorIndex];
		if ( firstBit==-1 )
			firstBit=0;

		int secondBit = nodeValues->vals[NODES_PER_MOTOR*motorIndex+1];
		if ( secondBit==-1 )
			secondBit=0;

		// Uses gray codes
		double rawValue;
		     if ( (firstBit==0) && (secondBit==0) )
			rawValue = 0;

		else if ( (firstBit==0) && (secondBit==1) )
			rawValue = 1;

		else if ( (firstBit==1) && (secondBit==1) )
			rawValue = 2;

		else if ( (firstBit==1) && (secondBit==0) )
			rawValue = 3;

		double scaledValue = Scale(rawValue,0.0,3.0,-1.0,+1.0);

		return( scaledValue );
	}
}

void NEURAL_NETWORK::Mutate(double mutationProbability) {

/*
	for (int i=0;i<numNodes;i++) {

		for (int j=0;j<numNodes;j++) {

			if ( Rand(0,1) < mutationProbability )

				Mutate(i,j);
		}
	}
*/

	int mutationOccurred = Mutate_Biased(mutationProbability);

	while ( !mutationOccurred )

		mutationOccurred = Mutate_Biased(mutationProbability);
}

void NEURAL_NETWORK::Print(void) {

	if ( weights )

		weights->Print();

	if ( nodeValues )

		nodeValues->Print();
}

void NEURAL_NETWORK::Save(ofstream *outFile) {

	if ( weights ) {
		(*outFile) << "1\n";
		weights->Write(outFile);
	}
	else
		(*outFile) << "0\n";

        (*outFile) << numMotors  << "\n";
}

void NEURAL_NETWORK::Sensors_Set(int environmentIndex) {

	// Each motor can initally 'see' which environment the robot
	// is operating in.

	if ( NODES_PER_MOTOR==2 ) {
		
		int firstBit, secondBit;

		// Uses gray codes
		switch ( environmentIndex ) {
		case 0:
			firstBit  = -1;
			secondBit = -1;
			break;
		case 1:
			firstBit  = -1;
			secondBit = +1;
			break;
		case 2:
			firstBit  = +1;
			secondBit = +1;
			break;
		case 3:
			firstBit  = +1;
			secondBit = -1;
			break;
		}

		for (int j=0;	j<numNodes;	j=j+2) {

			nodeValues->Set(0,j,  firstBit);
			nodeValues->Set(0,j+1,secondBit);
		}
	}
	else if ( NODES_PER_MOTOR==3 ) {
		
/*
		int firstBit, secondBit, thirdBit;

		// Uses gray codes
		switch ( environmentIndex ) {
		case 0:
			firstBit  = -1;
			secondBit = -1;
			thirdBit  = -1;
			break;
		case 1:
			firstBit  = -1;
			secondBit = -1;
			thirdBit  = +1;
			break;
		case 2:
			firstBit  = -1;
			secondBit = +1;
			thirdBit  = +1;
			break;
		case 3:
			firstBit  = -1;
			secondBit = +1;
			thirdBit  = -1;
			break;

		case 4:
			firstBit  = +1;
			secondBit = +1;
			thirdBit  = -1;
			break;
		case 5:
			firstBit  = +1;
			secondBit = +1;
			thirdBit  = +1;
			break;
		case 6:
			firstBit  = +1;
			secondBit = -1;
			thirdBit  = +1;
			break;
		case 7:
			firstBit  = +1;
			secondBit = -1;
			thirdBit  = -1;
			break;
		}

		for (int j=0;	j<numNodes;	j=j+3) {

			nodeValues->Set(0,j,  firstBit);
			nodeValues->Set(0,j+1,secondBit);
			nodeValues->Set(0,j+2,thirdBit);
		}
*/

		// The third bit is not used
		int firstBit, secondBit;

		// Uses gray codes
		switch ( environmentIndex ) {
		case 0:
			firstBit  = -1;
			secondBit = -1;
			break;
		case 1:
			firstBit  = -1;
			secondBit = +1;
			break;
		case 2:
			firstBit  = +1;
			secondBit = +1;
			break;
		case 3:
			firstBit  = +1;
			secondBit = -1;
			break;
		}

		for (int j=0;	j<numNodes;	j=j+3) {

			nodeValues->Set(0,j,  firstBit);
			nodeValues->Set(0,j+1,secondBit);
			nodeValues->Set(0,j+2,-1);
		}
	}
	// NODES_PER_MOTOR>3
	else  {

		/*
		// The third and fourth bits are not used
		int firstBit, secondBit;

		// Uses gray codes
		switch ( environmentIndex ) {
		case 0:
			firstBit  = -1;
			secondBit = -1;
			break;
		case 1:
			firstBit  = +1;
			secondBit = +1;
			break;
		case 2:
			firstBit  = -1;
			secondBit = +1;
			break;
		case 3:
			firstBit  = +1;
			secondBit = -1;
			break;
		}
		for (int j=0;	j<numNodes;	j=j+NODES_PER_MOTOR) {

			nodeValues->Set(0,j,  firstBit);
			nodeValues->Set(0,j+1,secondBit);

			for (int k=2;k<NODES_PER_MOTOR;k++)
				nodeValues->Set(0,j+k,-1);
		}
		*/

/*
		switch (environmentIndex) {
		case 0:
			for (int j=0;	j<numNodes;	j=j+2) {
				nodeValues->Set(0,j+0,-1);
				nodeValues->Set(0,j+1,-1);
			}
			break;
		case 1:
			for (int j=0;	j<numNodes;	j=j+2) {
				nodeValues->Set(0,j+0,+1);
				nodeValues->Set(0,j+1,+1);
			}
			break;
		case 2:
			for (int j=0;	j<numNodes;	j=j+2) {
				nodeValues->Set(0,j+0,-1);
				nodeValues->Set(0,j+1,+1);
			}
			break;
		case 3:
			for (int j=0;	j<numNodes;	j=j+2) {
				nodeValues->Set(0,j+0,+1);
				nodeValues->Set(0,j+1,-1);
			}
			break;
		}
*/

		for (int j=0;	j<numNodes;	j++)
			nodeValues->Set(0,j,-1);

		switch (environmentIndex) {
		case 0:
			for (int j=0;j<numNodes;j=j+6) {
				nodeValues->Set(0,j+0,-1); // Motor  neuron 1
				nodeValues->Set(0,j+1,-1); // Motor  neuron 2
				nodeValues->Set(0,j+2,-1); // Hidden neuron 1
				nodeValues->Set(0,j+3,-1); // Hidden neuron 2
				nodeValues->Set(0,j+4,-1); // Sensor neuron 1
				nodeValues->Set(0,j+5,+1); // Sensor neuron 2
			}
			break;
		case 1:
			for (int j=0;j<numNodes;j=j+6) {
				nodeValues->Set(0,j+0,-1); // Motor  neuron 1
				nodeValues->Set(0,j+1,-1); // Motor  neuron 2
				nodeValues->Set(0,j+2,-1); // Hidden neuron 1
				nodeValues->Set(0,j+3,-1); // Hidden neuron 2
				nodeValues->Set(0,j+4,+1); // Sensor neuron 1
				nodeValues->Set(0,j+5,-1); // Sensor neuron 2
			}
			break;
		case 2:
			for (int j=0;j<numNodes;j=j+6) {
				nodeValues->Set(0,j+0,-1); // Motor  neuron 1
				nodeValues->Set(0,j+1,-1); // Motor  neuron 2
				nodeValues->Set(0,j+2,-1); // Hidden neuron 1
				nodeValues->Set(0,j+3,-1); // Hidden neuron 2
				nodeValues->Set(0,j+4,+1); // Sensor neuron 1
				nodeValues->Set(0,j+5,+1); // Sensor neuron 2
			}
			break;
		case 3:
			for (int j=0;j<numNodes;j=j+6) {
				nodeValues->Set(0,j+0,-1); // Motor  neuron 1
				nodeValues->Set(0,j+1,-1); // Motor  neuron 2
				nodeValues->Set(0,j+2,-1); // Hidden neuron 1
				nodeValues->Set(0,j+3,-1); // Hidden neuron 2
				nodeValues->Set(0,j+4,-1); // Sensor neuron 1
				nodeValues->Set(0,j+5,-1); // Sensor neuron 2
			}
			break;
		}
	}
}

void NEURAL_NETWORK::Update(int timeStep) {

	if ( !edgeList )
		EdgeList_Create();

	for (int j=0;	j<numNodes;	j++)

		temp->vals[j] = 0;

	for (int i=0;	i<numConnections;	i++) {
 
		int sourceNode	= int(edgeList->vals[i*edgeList->width + 1]);
		int targetNode	= int(edgeList->vals[i*edgeList->width + 0]);
		double weight	=     edgeList->vals[i*edgeList->width + 2];

		temp->vals[targetNode] = temp->vals[targetNode] + 

					(nodeValues->vals[sourceNode]*weight);
	}


	for (int j=0;	j<numNodes;	j=j+6) {

		// Update the two motor and two hidden nodes,
		// but don't change the two sensor nodes.
		for (int k=0;	k<4;	k++) {

			if ( temp->vals[j+k]      < 0 )

				temp->vals[j+k]   = -1;

			else
				temp->vals[j+k]   = +1;
		}
	}

	for (int j=0;	j<numNodes;	j=j+6) {

		// Update the two motor and two hidden nodes,
		// but don't change the two sensor nodes.
		for (int k=0;	k<4;	k++) {

			nodeValues->vals[j+k] = temp->vals[j+k];
		}
	}
}

// Private methods ---------------------------------------------

void   NEURAL_NETWORK::Connection_Add(int nodeIndex) {

	int j = RandInt(0,numNodes-1);

	while ( weights->Get(nodeIndex,j) != 0 )

		j = RandInt(0,numNodes-1);

	if ( FlipCoin() )

		weights->Set(nodeIndex,j,-1);
	else
		weights->Set(nodeIndex,j,+1);
}

void   NEURAL_NETWORK::Connection_Remove(int nodeIndex) {

	int j = RandInt(0,numNodes-1);

	while ( weights->Get(nodeIndex,j) == 0 )

		j = RandInt(0,numNodes-1);

	weights->Set(nodeIndex,j,0);
}

void NEURAL_NETWORK::EdgeList_Create(void) {

	for (int i=0;i<weights->length;i++)

		for (int j=0;j<weights->width;j++)

			if ( weights->Get(i,j) != 0.0 )

				numConnections++;

	edgeList = new MATRIX(numConnections,3,0.0);

	numConnections = 0;

	for (int i=0;i<weights->length;i++)

		for (int j=0;j<weights->width;j++)

			if ( weights->Get(i,j) != 0.0 ) {

				edgeList->Set(numConnections,0,i);
				edgeList->Set(numConnections,1,j);
				edgeList->Set(numConnections,2,weights->Get(i,j));
				numConnections++;
			}
}

int NEURAL_NETWORK::FlipCoin(void) {

        return( Rand(0.0,1.0) > 0.5 );
}

double NEURAL_NETWORK::Incoming_Connections(int nodeIndex) {

	double incomingConnections = 0.0;

	for (int j=0;	j<numNodes;	j++)

		if ( weights->Get(nodeIndex,j) != 0 )

			incomingConnections++;

	return( incomingConnections);
}

void NEURAL_NETWORK::Initialize(void) {

	numNodes	= numMotors * NODES_PER_MOTOR;

	nodeValues 	= new MATRIX(1,numNodes,0.0);

	temp 		= new MATRIX(1,numNodes,0.0);

	inAFixedAttractor = false;

	numConnections	= 0;
	edgeList	= NULL;
}

void NEURAL_NETWORK::Mutate(int i, int j) {

	int originalValue = int(weights->Get(i,j));

	switch ( originalValue ) {

	case -1:
		if ( FlipCoin() )
			weights->Set(i,j,0);
		else
			weights->Set(i,j,+1);
		break;

	case 0:
		if ( FlipCoin() )
			weights->Set(i,j,-1);
		else
			weights->Set(i,j,+1);
		break;

	case +1:
		if ( FlipCoin() )
			weights->Set(i,j,-1);
		else
			weights->Set(i,j,0);
		break;
	}
}

int NEURAL_NETWORK::Mutate_Biased(double mutationProbability) {

	int mutationOccurred = false;

	for (int i=0;i<numNodes;i++) {

		if ( Rand(0,1) < mutationProbability ) {

/*
			Connection_Remove(i);
			Connection_Add(i);
			mutationOccurred = true;
*/

			double incomingConnections = Incoming_Connections(i);

			double probabilityOfRemovingAConnection =

				(40*incomingConnections) /
 
				( (40*incomingConnections) + (numNodes-incomingConnections) );

			if ( Rand(0,1) < probabilityOfRemovingAConnection )

				Connection_Remove(i);

			else
				Connection_Add(i);

			mutationOccurred = true;


			if ( FlipCoin() ) {

				double incomingConnections = Incoming_Connections(i);

				double probabilityOfRemovingAConnection =

					(40*incomingConnections) /
	 
					( (40*incomingConnections) + (numNodes-incomingConnections) );

				if ( Rand(0,1) < probabilityOfRemovingAConnection )

					Connection_Remove(i);

				else
					Connection_Add(i);

				mutationOccurred = true;
			}
		}
	}

	return( mutationOccurred );
}

double NEURAL_NETWORK::Rand(double min, double max) {

        double zeroToOne = ((double)rand()) / RAND_MAX;
        double returnVal;

        returnVal = (zeroToOne * (max-min)) + min;
        return returnVal;
}

int NEURAL_NETWORK::RandInt(int min, int max) {

        if ( min == max )
                return( min );
        else
                return( (rand() % (max-min+1)) + min );
}

double NEURAL_NETWORK::Scale(double value, double min1, double max1,
                                           double min2, double max2) {

        if ( min1 < 0 )
                value = value - min1;
        else
                value = value + min1;

        return( (value*(max2-min2)/(max1-min1)) + min2 );

}

void NEURAL_NETWORK::Weights_Initialize_Randomly(void) {

	for (int i=0;	i<numNodes;	i++) {

		for (int j=0;	j<numNodes;	j++) {

			double valueType = Rand(0,1);

			if ( valueType < 0.334 )

				weights->Set(i,j,-1);

			else if ( valueType < 0.667 )

				weights->Set(i,j,0);

			else
				weights->Set(i,j,+1);
		}
	}
}

void NEURAL_NETWORK::Weights_Initialize_NumIncomingConnections(int nic) {

	for (int i=0;	i<numNodes;	i++)

		for (int k=0;	k<nic;	k++)

			Connection_Add(i);
}

#endif

