#include "LocalSearch.h"

LocalSearch::LocalSearch(const Instance& instance, Solution solution) :
	instance{ &instance }, bestSolution{ solution }, bestValue{ 0 }
{
	bestValue = solutionValue(bestSolution);
}

Solution LocalSearch::run(size_t tabuSize, size_t numIterations, size_t k)
{
	tabuList = std::vector<Solution>{};
	for (size_t i = 0; i < numIterations; ++i)
	{
		Neighbours neighbours = generateNeighbours(k);
		const RankedSolution& bestNeighbour = neighbours[getBestNeighbour(neighbours)];

		if (bestNeighbour.value > bestValue)
		{
			bestSolution = bestNeighbour.solution;
			bestValue = bestNeighbour.value;
			tabuList.push_back(bestSolution);
		}
		else
		{
			return bestSolution;
		}
	}
	return bestSolution;
}

Solution do2Opt(const Solution& path, size_t i, size_t j)
{
	Solution pathCopy{ path };
	std::reverse(std::begin(pathCopy) + i + 1, std::begin(pathCopy) + j + 1);
	return pathCopy;
}

Neighbours LocalSearch::generateNeighbours(size_t k)
{
	// jak zawsze jest improvement to nigdy nie ma potworzen, po co tabu?
	// nie istniejace polaczenie mozna zamienic na istniejace
	// nie oceniaj rozwiazan po koszcie tylko po liczbie miast (nie do konca)
	// test czy valid
	Neighbours neighbours{};
	const size_t n = instance->s;

	for (int i = 0; i <= n - k; i++)
	{
		for (int j = i + 1; j <= n + 1 - k; j++)
		{
			const auto& dist = instance->adjMatrix;
			int valueDelta = dist[i][(i + 1) % n] - dist[j][(j + 1) % n] + dist[i][j] + dist[(i + 1) % n][(j + 1) % n];

			Solution neighbour = do2Opt(bestSolution, i, j);
			neighbours.push_back(RankedSolution{ neighbour, bestValue + valueDelta });
		}
	}
	return neighbours;
}

size_t LocalSearch::getBestNeighbour(const Neighbours& neighbours)
{
	int bestVal = std::numeric_limits<int>::max();
	size_t bestIdx = 0;
	for (size_t i = 0; i < neighbours.size(); ++i)
	{
		if (neighbours[i].value < bestVal)
		{
			bestVal = neighbours[i].value;
			bestIdx = i;
		}
	}

	return bestIdx;
}

int LocalSearch::solutionValue(const Solution& solution)
{
	// jak rozwiazanie jest krotsze, ale maly koszt to powinno byc lepsze od dluzszego
	return solution.size();
}

int cost(const Solution& solution, const Instance* instance)
{
	int value{};
	for (size_t i = 0; i < solution.size() - 1; ++i)
	{
		size_t v1 = solution[i];
		size_t v2 = solution[i + 1];
		value += instance->adjMatrix[v1][v2];
	}

	return value;
}

size_t outputLength(const Solution& solution, const Instance* instance)
{
	return cost(solution, instance) + instance->l;
}

bool isValid(const Solution& solution, const Instance* instance)
{
	outputLength(solution, instance) <= instance->n;
}
