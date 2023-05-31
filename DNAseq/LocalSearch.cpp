#include "LocalSearch.h"

LocalSearch::LocalSearch(const Instance& instance, Solution solution) :
	instance{ &instance }, bestSolution{ solution, cost(solution, &instance) }
{

}

Solution LocalSearch::run(size_t tabuSize, size_t numIterations, size_t k)
{
	if (!isValid(bestSolution.solution, instance))
		throw std::exception{};

	tabuList = std::vector<RankedSolution>{};
	for (size_t i = 0; i < numIterations; ++i)
	{
		RankedSolution bestNeighbour = getBestNeighbour(k);

		if (bestNeighbour > bestSolution)
		{
			bestSolution = bestNeighbour;
			tabuList.push_back(bestSolution);
		}
		else
		{
			return bestSolution.solution;
		}
	}
	return bestSolution.solution;
}

Solution do2Opt(const Solution& path, size_t i, size_t j)
{
	Solution pathCopy{ path };
	std::reverse(std::begin(pathCopy) + i + 1, std::begin(pathCopy) + j + 1);
	return pathCopy;
}

RankedSolution LocalSearch::getBestNeighbour(size_t k)
{
	// jak zawsze jest improvement to nigdy nie ma potworzen, po co tabu?
	// nie istniejace polaczenie mozna zamienic na istniejace
	// nie oceniaj rozwiazan po koszcie tylko po liczbie miast (nie do konca)
	// test czy valid
	RankedSolution bestNeighbour{};
	const size_t n = instance->adjMatrix.size();

	for (int i = 0; i <= n - k; i++)
	{
		for (int j = i + 1; j <= n + 1 - k; j++)
		{
			const auto& dist = instance->adjMatrix;
			int costDelta = dist[i][(i + 1) % n] - dist[j][(j + 1) % n] + dist[i][j] + dist[(i + 1) % n][(j + 1) % n];

			Solution neighbourSolution = do2Opt(bestSolution.solution, i, j);
			int c = cost(neighbourSolution, instance);
			RankedSolution neighbour = { std::move(neighbourSolution), c};
			if (isValid(neighbour.solution, instance) && !isTabu(neighbour))
			{
				if (neighbour > bestNeighbour)
					bestNeighbour = neighbour;
			}
		}
	}
	return bestNeighbour;
}

int LocalSearch::solutionValue(const Solution& solution)
{
	// jak rozwiazanie jest krotsze, ale maly koszt to powinno byc lepsze od dluzszego
	return solution.size();
}

bool LocalSearch::isTabu(const RankedSolution& solution)
{
	for (const auto& s : tabuList)
	{
		if (s == solution)
			return false;
	}

	return true;
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
	return outputLength(solution, instance) <= instance->n;
}
