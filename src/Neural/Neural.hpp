
#include <Neural/neuralStruct.h>
#include <Neural/learningStruct.h>

#include <glm/glm.hpp>

#include <vector>
#include <string>
#include <memory>


class Neural {

	public:

		Neural();
		Neural(std::pair<unsigned int, unsigned int> matLen, std::pair<double, double> bound);
		
		~Neural();

		void init(std::pair<unsigned int, unsigned int> matLen, std::pair<double, double> bound);

		void generate(std::string filename, unsigned int nbLine, unsigned int nbData, std::vector<std::string> allLabels);

		void initOneByOne();
		void learnOneByOne();

		void dataOneByOne();

		void learn();

		void print();

		void printLearn();

		float getNeuronLen(size_t index) const;
		float getNeuronSpecialAdd(size_t index, int arg) const;
		float getNeuronData(size_t index, size_t dataIndex) const;

		float callGoodMethod(size_t index, int method, int methodArg) const;

		/**
		 * This method will generate a height vector with the choosen method.
		 * 
		*/
		std::vector<float> generateHeight(size_t len, size_t row, bool linear = true, int method = 0, int methodArg = 0,  float heightMultiplacator = 1) const;

		std::vector<glm::vec3> getColorVector(size_t len, size_t row) const;

	private:

		std::vector<std::string> allLabels_;

		unsigned int nbNeur_;
		unsigned int matLen_[2];
		double bound_[2];

		neuralMat neurMat;
		learningMat allIris;

		
		//This part is for the step by step learn function.
		unsigned int learnCounter;
		unsigned int dataCounter;
		vector indexVector;

};