
#ifndef NEURAL_STRUCT_H
#define NEURAL_STRUCT_H

#include "learningStruct.h"
#include "vector.h"

#ifdef __cplusplus
extern "C"{
#endif 


struct neuralVec {

	double* data;
	unsigned int size;

	int id;

	//double norme;

};
typedef struct neuralVec neuralVec;



struct neuralMat{
	neuralVec* data;
	unsigned int size;
	unsigned int width;
	unsigned int height;
};
typedef struct neuralMat neuralMat;


/*
	This function will generate the neural matrice (neurMat) based on the
	average of all the learning matrice and some small random value.

	size contain the number of neurone, matLen contain the width and the height of the neural matrice
	and bound the bounds of the random value to add to the average of the learning matrice.
*/
void generateNeuralMat(neuralMat* neurMat, learningMat* learnMat, unsigned int size, unsigned int matLen[2], double bound[2]);

/*
	This function start the learning on the neural matrice (neurMat) with the learning matrice (learnMat)
*/
void startLearning(neuralMat* neurMat, learningMat* learnMat);

/**
 * This do function do the learning on the neural matrice but step
 * by step using an unsigned int pointer to keep the iteration count.
 * 
 * @return 1 if it's finished and labeled, 0 otherwise.
 */
int splitedLearning(neuralMat* neurMat, learningMat* learnMat, vector* indexVector, unsigned int* currentIteration, unsigned int maxIteration);

/*
	This function spread the win of a neurone to it's neighbors in an area of size areaSize.
*/
void spread(neuralMat* neurMat, learningVec* learnVec, int winnerIndex, int areaSize, double alpha);

/*
	This function do one learning passage. It find the nearest neurone
	for each learning vec and call spread on it to spread it's win
*/
void learn(neuralMat* neurMat, learningMat* learnMat, vector* indexVec, int areaSize, double alpha);

/*
	This function set the id of the neural matrice after the learning phase
*/
void setId(neuralMat* neurMat, learningMat* learnMat);

/*
	This function print all the neural vector with each attribute.
*/
void printNeuralMat(neuralMat* neurMat);

/*
	This function print all the neural matrice in it's matrice form with each vector id.
*/
void visualize(neuralMat* neurMat);

#ifdef __cplusplus
}
#endif 


#endif /* NEURAL_STRUCT_H */
