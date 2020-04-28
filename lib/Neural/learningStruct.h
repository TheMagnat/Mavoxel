#ifndef LEARNING_STRUCT_H
#define LEARNING_STRUCT_H

#ifdef __cplusplus
extern "C"{
#endif 


/*
	This structure is a string vector that contain the labels of the data we're learning.
*/
struct dataLabel{
	char** labels;
	unsigned int size;
};
typedef struct dataLabel dataLabel;


/*
	This structure is a vector that contain all the attribute of the data,
	it's norme and id.
*/
struct learningVec {

	double* data;
	unsigned int size;

	double norme;

	unsigned int id;

};
typedef struct learningVec learningVec;


/*
	This structure is a vector of vector the learning vector (so a matrice).
*/
struct learningMat{
	learningVec* data;
	unsigned int size;
};
typedef struct learningMat learningMat;


/*
	This function is the main function for generating the learning matrice.
	The learning matrice is generated in 'mat'
	it call openAndReadLearningData and normalizeLearningMat.
*/
void generateLearningMat(learningMat* mat, char* fileName, unsigned int nbLine, unsigned int nbData, dataLabel allLabels);

/**
 * This function delete all the memory allocated to the mat variable.
*/
void deleteLearningMat(learningMat* mat);

/*
	This function open and read the learning file and will fill the learning matrice (learnMat) with it.
*/
void openAndReadLearningData(char* filename, learningMat* learnMat, unsigned int nbLine, unsigned int nbData, dataLabel allLabels);

/*
	This function take one line of the data file (str) and fill 1 learning vector (toFill) with it.
*/
void pickData(char* str, learningVec* toFill, unsigned int nbData, dataLabel allLabels);

/*
	This function normalize the learning matrice (mat).
*/
void normalizeLearningMat(learningMat* mat);

#ifdef __cplusplus
}
#endif 



#endif /* LEARNING_STRUCT_H */
