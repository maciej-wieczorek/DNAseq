#pragma once
#include "Instance.h"

using Solution = std::vector<size_t>;

struct RankedSolution
{
	Solution solution;
	int cost;

	bool operator>(const RankedSolution& other) const
	{
		if (solution.size() > other.solution.size())
			return true;
		else if (solution.size() < other.solution.size())
			return false;
		else
			return (cost < other.cost);
	}

	bool operator==(const RankedSolution& other) const
	{
		if (solution.size() != other.solution.size())
			return false;

		for (size_t i = 0; i < solution.size(); ++i)
		{
			if (solution[i] != other.solution[i])
				return false;
		}

		return true;
	}
};

using Neighbours = std::vector<RankedSolution>;

class LocalSearch
{
public:
	LocalSearch(const Instance& instance, Solution solution);
	Solution run(size_t tabuSize = 30, size_t numIterations = 100, size_t k = 2);


private:
	RankedSolution getBestNeighbour(size_t k);
	bool isTabu(const RankedSolution& solution);

	const Instance* instance;
	RankedSolution bestSolution;
	RankedSolution currentSolution;
	std::vector<RankedSolution> tabuList;
};

int cost(const Solution& solution, const Instance* instance);
bool isValid(const RankedSolution& solution, const Instance* instance);

