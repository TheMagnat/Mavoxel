
#include "Neural.hpp"

#include <cmath>
#include <iostream>

Neural::Neural() {

}

Neural::Neural(std::pair<unsigned int, unsigned int> matLen, std::pair<double, double> bound)
: nbNeur_(matLen.first * matLen.second) {

	matLen_[0] = matLen.first;
	matLen_[1] = matLen.second;
	

	bound_[0] = bound.first;
	bound_[1] = bound.second;

}

void Neural::init(std::pair<unsigned int, unsigned int> matLen, std::pair<double, double> bound){
	
	nbNeur_ = matLen.first * matLen.second;

	matLen_[0] = matLen.first;
	matLen_[1] = matLen.second;

	bound_[0] = bound.first;
	bound_[1] = bound.second;

}
		
Neural::~Neural(){
	///delete neural and learning
}

void Neural::generate(std::string filename, unsigned int nbLine, unsigned int nbData, std::vector<std::string> allLabels){

	size_t i;

	allLabels_ = allLabels;


	std::unique_ptr<char*[]> ptr = std::make_unique<char*[]>(allLabels_.size());

	// tempo.labels = (char**) malloc(allLabels_.size() * sizeof(char*));

	for(i = 0; i < allLabels_.size(); ++i){
		
		ptr.get()[i] = allLabels_[i].data();

	}

	dataLabel tempo;

	tempo.size 		= allLabels_.size();
	tempo.labels 	= ptr.get();

	generateLearningMat(&allIris, filename.data(), nbLine, nbData, tempo);
	generateNeuralMat(&neurMat, &allIris, nbNeur_, matLen_, bound_);

	//Note : The unique_ptr ptr will delete the allocated memory here.
}

void Neural::initOneByOne(){

	counter = 0;

	std::cout << "size : " << allIris.size << std::endl;

	createVec(&indexVector, allIris.size);
	fillVecWithIndex(&indexVector);

}

void Neural::learnOneByOne(){

	std::cout << "shuffle\n";
	shuffleVec(&indexVector);

	for(int i = 0; i < 50; ++i)
		splitedLearning(&neurMat, &allIris, &indexVector, &counter, 2000);


	std::cout << "count : " << counter << std::endl; 

}

void Neural::learn(){

	startLearning(&neurMat, &allIris);

}

void Neural::print(){

	unsigned int i;

	visualize(&neurMat);

	printf("\n");
	for(i = 0; i < allLabels_.size(); ++i){
		printf("%d = %s\n", i+1, allLabels_[i].data());
	}

}

void Neural::printLearn(){

	unsigned int i;

	for(i = 0; i < allIris.size; ++i){
		printf("Iris = %f, %f, %f, %f\n", allIris.data[i].data[0], allIris.data[i].data[1], allIris.data[i].data[2], allIris.data[i].data[3]);
	}

}

float Neural::getNeuronLen(size_t index) const {

	float result(0);
	
	// std::cout << "Index: " << index << std::endl;
	for(unsigned int i(0); i < neurMat.data[index].size; ++i){

		// std::cout << "i: " << i << " value: " << neurMat.data[index].data[i] << std::endl;
		result += neurMat.data[index].data[i] * neurMat.data[index].data[i];

	}

	return sqrt(result);

}

float Neural::getNeuronSpecialAdd(size_t index, int arg) const {

	float result(0);
	
	for(unsigned int i(0); i < neurMat.data[index].size; ++i){

		result += ((i+1)*(arg*0.1)) * neurMat.data[index].data[i];

	}

	return result/10.0f;

}

float Neural::getNeuronData(size_t index, size_t dataIndex) const {

	if(dataIndex > 3) dataIndex = 3;

	return neurMat.data[index].data[dataIndex];

}

float getRangedValue(size_t val1, size_t max1, size_t max2){
	return (float)val1 * (float)max2 / (float)max1;
}

float Neural::callGoodMethod(size_t index, int method, int methodArg) const {

	switch(method){

		default:
		case 0:
			return getNeuronLen(index);

		case 1:
			return getNeuronData(index, methodArg);

		case 2:
			return getNeuronSpecialAdd(index, methodArg);

	}

}

double cubicInterpolate (double p[4], double x) {
	return p[1] + 0.5 * x*(p[2] - p[0] + x*(2.0*p[0] - 5.0*p[1] + 4.0*p[2] - p[3] + x*(3.0*(p[1] - p[2]) + p[3] - p[0])));
}

/**
 * This method will generate a height vector with the choosen method.
 * 
*/
std::vector<float> Neural::generateHeight(size_t len, size_t row, bool linear, int method, int methodArg,  float heightMultiplicator) const {
	

	std::vector<float> heightResult;

	for(size_t y(0); y < row; ++y){

		for(size_t x(0); x < len; ++x){
			
			float xRatio, yRatio;
			size_t trueX, trueY;

			if(x == len-1){
				trueX = neurMat.width-2;
				xRatio = 1;
			}
			else{
				xRatio = getRangedValue(x, len-1, neurMat.width-1);
				// std::cout << "xRatio: " << xRatio << std::endl;
				trueX = (unsigned int)xRatio;
				xRatio = xRatio - trueX; //This work because getRangedValue will allaway return a positive value
			}


			if(y == row-1){
				trueY = neurMat.height-2;
				yRatio = 1;
			}
			else{

				yRatio = getRangedValue(y, row-1, neurMat.height-1);
				// std::cout << "yRatio: " << yRatio << std::endl;
				trueY = (unsigned int)yRatio;
				yRatio = yRatio - trueY;
			}


			float height(0);

			if(linear){
				///LINEAR
				//0 0
				height += (1 - xRatio) * (1 - yRatio) 	* callGoodMethod(trueX + trueY * neurMat.width, method, methodArg);

				//1 0
				height += xRatio * (1 - yRatio) 		*  callGoodMethod(trueX+1 + trueY * neurMat.width, method, methodArg);
				//0 1
				
				height += (1 - xRatio) * yRatio 		*  callGoodMethod(trueX + (trueY+1) * neurMat.width, method, methodArg);
				//1 1
				
				height += xRatio * yRatio				*  callGoodMethod(trueX+1 + (trueY+1) * neurMat.width, method, methodArg);

			///LINEAR
			}
			else{
			///COSINE
				double allHeight[4];
				
				allHeight[0] =callGoodMethod(trueX + trueY * neurMat.width, method, methodArg);
				allHeight[1] =callGoodMethod(trueX+1 + trueY * neurMat.width, method, methodArg);
				allHeight[2] =callGoodMethod(trueX + (trueY+1) * neurMat.width, method, methodArg);
				allHeight[3] =callGoodMethod(trueX+1 + (trueY+1) * neurMat.width, method, methodArg);
				
				float inter1, inter2, inter3, inter4;
				float m;

				//0 0 -> 1 0
				m = (1-cos(xRatio *M_PI))/2.0;
				inter1 = (1-m)*allHeight[0] + m*allHeight[1];
				//0 1 -> 1 1
				m = (1-cos(xRatio *M_PI))/2.0;
				inter2 = (1-m)*allHeight[2] + m*allHeight[3];
				//0 0 -> 0 1
				m = (1-cos(yRatio *M_PI))/2.0;
				inter3 = (1-m)*allHeight[0] + m*allHeight[2];
				//1 0 -> 1 1
				m = (1-cos(yRatio *M_PI))/2.0;
				inter4 = (1-m)*allHeight[1] + m*allHeight[3];

				float interA, interB;

				m = (1-cos(yRatio *M_PI))/2.0;
				interA = (1-m)*inter1 + m*inter2;

				m = (1-cos(xRatio *M_PI))/2.0;
				interB = (1-m)*inter3 + m*inter4;

				height = (interA + interB)/2.0;

			///COSINE
			}


			height *= heightMultiplicator;
			
			// std::cout << "X: " << x << " Y: " << y << " Height: " << height << std::endl;
			
			// height = cubicInterpolate(allHeight, 0.5);

			heightResult.emplace_back(height);

		}

	}

	return heightResult;

}

std::vector<glm::vec3> Neural::getColorVector(size_t len, size_t row) const {
	
	std::vector<glm::vec3> colors;

	std::vector<glm::vec3> allPossibleColors;

	allPossibleColors.resize(4);

	allPossibleColors[0] = glm::vec3(219.0f/255.0f, 206.0f/255.0f, 211.0f/255.0f);
	allPossibleColors[1] = glm::vec3(138.0f/255.0f, 46.0f/255.0f, 242.0f/255.0f);
	
	allPossibleColors[2] = glm::vec3(182.0f/255.0f, 143.0f/255.0f, 227.0f/255.0f);
	allPossibleColors[3] = glm::vec3(184.0f/255.0f, 51.0f/255.0f, 153.0f/255.0f);

	for(size_t y(0); y < row; ++y){

		for(size_t x(0); x < len; ++x){

			float xRatio, yRatio;
			size_t trueX, trueY;

			if(x == len-1){
				trueX = neurMat.width-2;
				xRatio = 1;
			}
			else{
				xRatio = getRangedValue(x, len-1, neurMat.width-1);
				// std::cout << "xRatio: " << xRatio << std::endl;
				trueX = (unsigned int)xRatio;
				xRatio = xRatio - trueX; //This work because getRangedValue will allaway return a positive value
			}


			if(y == row-1){
				trueY = neurMat.height-2;
				yRatio = 1;
			}
			else{

				yRatio = getRangedValue(y, row-1, neurMat.height-1);
				// std::cout << "yRatio: " << yRatio << std::endl;
				trueY = (unsigned int)yRatio;
				yRatio = yRatio - trueY;
			}

			glm::vec3 color(0.0f);
			
			//0 0
			color += (1 - xRatio) * (1 - yRatio)* allPossibleColors[neurMat.data[trueX + trueY * neurMat.width].id];

			//1 0
			color += xRatio * (1 - yRatio) 		*  allPossibleColors[neurMat.data[trueX+1 + trueY * neurMat.width].id];
			//0 1
			
			color += (1 - xRatio) * yRatio 		*  allPossibleColors[neurMat.data[trueX + (trueY+1) * neurMat.width].id];
			//1 1
			
			color += xRatio * yRatio			*  allPossibleColors[neurMat.data[trueX+1 + (trueY+1) * neurMat.width].id];

			// std::cout << "x: " << trueX << " y: " << trueY << " index: " << trueX + trueY * neurMat.width << std::endl;

			//allPossibleColors[neurMat.data[trueX + trueY * neurMat.width].id]
			colors.emplace_back(color);
			// std::cout << "emplaced\n";
		}

		
	}

	return colors;

}