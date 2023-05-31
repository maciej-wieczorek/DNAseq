#pragma once
#include "Instance.h"
#include <thread>

class AntColony {

public:
	struct Parameters {
		int Iterations;
		int Ants;
		float Alpha;
		float Beta;
		float Evaporation;

		Parameters(int iterations, int ants, float alpha, float beta, float evaporation)
			: Iterations(iterations), Ants(ants), Alpha(alpha), Beta(beta), Evaporation(evaporation) {}
	};

public:
	AntColony(const Instance& instance, const Parameters& parameters);
	virtual ~AntColony();

	std::vector<int> Run();
	
private:
	void Iteration();
	void Ant(float** pheromoneDeposited, std::mutex* mutex);
	std::vector<int> Result();
	std::string PheromeneToString() const;
	
private:
	const Instance& m_Instance;
	const Parameters& m_Parameters;
	float** m_Pheromone;

};

