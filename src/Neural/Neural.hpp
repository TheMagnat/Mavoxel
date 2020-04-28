
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

		void setLearningParam(unsigned int maxIteration_, float maxAlpha_, unsigned int maxArea_);

		/**
		 * Init the objet like the second constructor
		*/
		void init(std::pair<unsigned int, unsigned int> matLen, std::pair<double, double> bound);

		/**
		 * To generate the learning matrix and the neurons matrix
		*/
		void generate(std::string filename, unsigned int nbLine, unsigned int nbData, std::vector<std::string> allLabels);

		/**
		 * Call this init function after generate
		*/
		void initAfterGenerate();

		/**
		 * Reset the neurons.
		*/
		void reset();

		/**
		 * Learn from 1 iteration by 1 iteration.
		*/
		int learnOneByOne();

		/**
		 * learn from 1 data by 1 data.
		*/
		int dataOneByOne();

		/**
		 * Finish the learning from the current state of the neurons.
		*/
		void finishAll();

		/**
		 * Labelize the neurons with their current data.
		*/
		void reLabelize();

		/**
		 * Do the learning from 0.
		 * Do not call this method if a one by one methode has been used.
		*/
		void learn();

		void printLabels();

		void printLearn() const;

		float getNeuronLen(size_t index) const;
		float getNeuronSpecialAdd(size_t index, int arg) const;
		float getNeuronData(size_t index, size_t dataIndex) const;

		float callGoodMethod(size_t index, int method, int methodArg) const;

		/**
		 * This method will generate a height vector with the choosen method.
		*/
		std::vector<float> generateHeight(size_t len, size_t row, bool linear = true, int method = 0, int methodArg = 0,  float heightMultiplacator = 1, float heightAdd = 0.0f) const;

		/**
 		* This method will generate a color vector.
		*/
		std::vector<glm::vec3> getColorVector(size_t len, size_t row, bool linear) const;

	private:

		std::vector<std::string> allLabels_;

		unsigned int nbNeur_;
		unsigned int matLen_[2];
		double bound_[2];

		neuralMat neurMat;
		learningMat allIris;

		//Learning Param
		unsigned int maxIteration;
		float maxAlpha;
		unsigned int maxArea;
		
		//This part is for the step by step learn function.
		bool dataOneBOne;

		unsigned int learnCounter;
		unsigned int dataCounter;
		vector indexVector;

		vector labelIndexVector;

};