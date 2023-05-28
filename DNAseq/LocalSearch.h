#pragma once
#include "Instance.h"

using Solution = std::vector<size_t>;

struct RankedSolution
{
	Solution solution;
	int value;
};

using Neighbours = std::vector<RankedSolution>;

class LocalSearch
{
public:
	LocalSearch(const Instance& instance, Solution solution);
	Solution run(size_t tabuSize = 30, size_t numIterations = 100, size_t k = 2);


private:
	Neighbours generateNeighbours(size_t k);
	size_t getBestNeighbour(const Neighbours& neighbours);
	int solutionValue(const Solution& solution);

	const Instance* instance;
	Solution bestSolution;
	int bestValue;
	std::vector<Solution> tabuList;
};

