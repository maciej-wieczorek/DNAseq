#pragma once
#include "Instance.h"

class AntColony {

public:
	struct Parameters {
		int Iterations; // number of iterations
		int Ants; // number of ants
		float Alpha; // pheromone influence
		float Beta; // distance influence
		float Evaporation; // pheromeno evaporation rate

		Parameters(int iterations, int ants, float alpha, float beta, float evaporation)
			: Iterations(iterations), Ants(ants), Alpha(alpha), Beta(beta), Evaporation(evaporation) {}
	};

public:
	AntColony(const Instance& instance, const Parameters& parameters);
	virtual ~AntColony();

	std::vector<int> Run();
	
private:
	void Iteration();
	std::vector<int> Result();
	std::string PheromeneToString() const;
	
private:
	const Instance& m_Instance;
	const Parameters& m_Parameters;
	std::vector<std::vector<float>> m_Pheromone;
	std::vector<std::vector<float>> m_PheromoneDeposited;
	std::vector<float> m_Weights;

};

