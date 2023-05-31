#include "LocalSearch.h"

LocalSearch::LocalSearch(const Instance& instance, Solution solution) :
	instance{ &instance }, bestSolution{ solution, cost(solution, &instance) }, currentSolution{ bestSolution }
{
}

Solution LocalSearch::run(size_t tabuSize, size_t numIterations, size_t k)
{
	if (!isValid(bestSolution, instance))
		throw std::exception{};

	tabuList = std::vector<RankedSolution>{};
	for (size_t i = 0; i < numIterations; ++i)
	{
		currentSolution = getBestNeighbour(k);

		if (currentSolution.solution.size() == 0)
			return bestSolution.solution;

		tabuList.push_back(currentSolution);

		if (currentSolution > bestSolution)
		{
			bestSolution = currentSolution;
		}
	}

	return bestSolution.solution;
}

Solution do2Opt(const Solution& path, size_t i, size_t j)
{
	Solution pathCopy{ path };
	std::reverse(pathCopy.begin() + i, pathCopy.begin() + j + 1);

	return pathCopy;
}

Solution addVertex(const Solution& path, size_t index, size_t vertex)
{
	Solution newPath{ path };
	newPath.insert(newPath.begin() + index, vertex);

	return newPath;
}

RankedSolution LocalSearch::getBestNeighbour(size_t k)
{
	RankedSolution bestNeighbour{};
	const size_t n = currentSolution.solution.size();

	std::vector<size_t> newVertices{};
	for (size_t i = 0; i < instance->adjMatrix.size(); ++i)
	{
		if (std::find(currentSolution.solution.begin(),
			currentSolution.solution.end(), i) == currentSolution.solution.end())
		{
			newVertices.push_back(i);
		}
	}

	// try to add new vertex on every position
	bool found = false;
	for (size_t v : newVertices)
	{
		for (size_t i = 0; i <= n; ++i)
		{
			Solution neighbourSolution = addVertex(currentSolution.solution, i, v);
			int c = cost(neighbourSolution, instance);
			RankedSolution neighbour = { std::move(neighbourSolution), c};
			if (isValid(neighbour, instance) && !isTabu(neighbour))
			{
				if (neighbour > bestNeighbour)
				{
					bestNeighbour = neighbour;
					found = true;
				}
			}
		}
	}

	if (found)
		return bestNeighbour;

	// try edge swapping
	for (int i = 0; i <= n - k; i++)
	{
		for (int j = i + 1; j <= n + 1 - k; j++)
		{
			const auto& dist = instance->adjMatrix;
			int costDelta = dist[i][(i + 1) % n] - dist[j][(j + 1) % n] + dist[i][j] + dist[(i + 1) % n][(j + 1) % n];

			Solution neighbourSolution = do2Opt(currentSolution.solution, i, j);
			int c = cost(neighbourSolution, instance);
			RankedSolution neighbour = { std::move(neighbourSolution), c};
			if (isValid(neighbour, instance) && !isTabu(neighbour))
			{
				if (neighbour > bestNeighbour)
					bestNeighbour = neighbour;
			}
		}
	}

	return bestNeighbour;
}

bool LocalSearch::isTabu(const RankedSolution& solution)
{
	for (const auto& s : tabuList)
	{
		if (s == solution)
			return true;
	}

	return false;
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

size_t outputLength(const RankedSolution& solution, const Instance* instance)
{
	return solution.cost + instance->l;
}

bool isValid(const RankedSolution& solution, const Instance* instance)
{
	return outputLength(solution, instance) <= instance->n;
}
