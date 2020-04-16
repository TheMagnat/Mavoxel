#ifndef VECTOR_H
#define VECTOR_H

#ifdef __cplusplus
extern "C"{
#endif 


struct vector {

	int* data;
	unsigned int size;

};
typedef struct vector vector;



void createVec(vector* vec, int size);
void deleteVec(vector* vec);
//Fill vec with value
void fillVec(vector* vec, int value);
//Fill vec with each slot index
void fillVecWithIndex(vector* vec);
//Fill vec with random value that don't exced maxValue
void fillVecWithRandom(vector* vec, int maxValue);
//print the vec. Separate each value with the string separator
void printVec(vector* vec, char* separator);
//shuffle each value in the vector
void shuffleVec(vector* vec);
//Sort the vector
void sort(vector* vec);

#ifdef __cplusplus
}
#endif 


#endif /* VECTOR_H */
