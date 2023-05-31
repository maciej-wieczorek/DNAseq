#include "AntColony.h"

#include <cmath>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <map>
#include <set>

#include "Logger.h"

AntColony::AntColony(const Instance& instance, const Parameters& parameters)
	: m_Instance(instance), m_Parameters(parameters) {
	int size = m_Instance.oligonucleotides.size();

	m_Pheromone = new float* [size + 1];
	for (int i = 0; i < size + 1; i++) {
		m_Pheromone[i] = new float[size];
		for (int j = 0; j < size; j++) {
			m_Pheromone[i][j] = 1.f;
		}
	}
}

AntColony::~AntColony() {
	for (int i = 0; i < m_Instance.oligonucleotides.size() + 1; i++) {
		delete m_Pheromone[i];
	}
	delete m_Pheromone;
}

std::vector<int> AntColony::Run() {
	int** ants;
	for (int i = 0; i < m_Parameters.Iterations; i++) {
		Iteration();

		LOG_TRACE("ant colony: {} / {}", i + 1, m_Parameters.Iterations);
	}

	LOG_TRACE("pheromone:{}", PheromeneToString());

	return Result();
}

void AntColony::Iteration() {
	int size = m_Instance.oligonucleotides.size();

	// pheromene deposited
	float** pheromoneDeposited = new float* [size + 1];
	for (int i = 0; i < size + 1; i++) {
		pheromoneDeposited[i] = new float[size];
		for (int j = 0; j < size; j++) {
			pheromoneDeposited[i][j] = 0;
		}
	}

	// generate ants path
	for (int a = 0; a < m_Parameters.Ants; a++) {
		std::set<int> availableVertices;
		for (int i = 0; i < size; i++) {
			availableVertices.insert(i);
		}

		int pathLength = 0;
		int currentVertex = size;
		std::vector<int> path;
		path.push_back(currentVertex);

		// generate ant path
		while (true) {
			std::map<int, float> weights;
			float weightsSum = 0;

			// calculate weights for available edges
			std::set<int> toRemove;
			for (int vertex : availableVertices) {
				// distance from current vertex and available vertex
				int distance = currentVertex == size ? m_Instance.l : m_Instance.adjMatrix[currentVertex][vertex];

				if (pathLength + distance > m_Instance.n) {
					toRemove.insert(vertex);
					continue;
				}

				// calculate weight of edge
				float weight = std::pow(m_Pheromone[currentVertex][vertex], m_Parameters.Alpha) * std::pow(1.f / (float)distance, m_Parameters.Beta);
				weights.insert(std::pair<int, float>(vertex, weight));
				weightsSum += weight;
			}

			// remove not available edges
			for (int vertex : toRemove) {
				availableVertices.erase(vertex);
			}

			// end when there are no more available edges
			if (availableVertices.empty()) {
				break;
			}

			// select random edge
			int nextVertex = -1;
			float random = (float)rand() / (float)RAND_MAX * weightsSum;
			for (std::pair<int, float> weight : weights) {
				random -= weight.second;
				if (random < 0.f) {
					nextVertex = weight.first;
					break;
				}
			}
			if (nextVertex == -1) {
				nextVertex = weights.rbegin()->first;
			}

			path.push_back(nextVertex);
			pathLength += currentVertex == size ? m_Instance.l : m_Instance.adjMatrix[currentVertex][nextVertex];
			availableVertices.erase(nextVertex);
			currentVertex = nextVertex;
		}

		// calculate added pheromone
		for (int i = 0; i < path.size() - 1; i++) {
			pheromoneDeposited[path[i]][path[i + 1]] += (float)path.size() / (float)m_Instance.n;
		}
	}

	// update pheromone
	for (int i = 0; i < size + 1; i++) {
		for (int j = 0; j < size; j++) {
			m_Pheromone[i][j] = (1.f - m_Parameters.Evaporation) * m_Pheromone[i][j] + pheromoneDeposited[i][j];
		}
	}

}

std::vector<int> AntColony::Result() {
	int size = m_Instance.oligonucleotides.size();

	int pathLength = 0;
	int currentVertex = size;
	std::vector<int> result;

	std::set<int> availableVertices;
	for (int i = 0; i < size; i++) {
		availableVertices.insert(i);
	}

	while (true) {
		std::set<int> toRemove;
		int nextVertex = 0;

		for (int vertex : availableVertices) {
			// distance from current vertex and available vertex
			int distance = currentVertex == size ? m_Instance.l : m_Instance.adjMatrix[currentVertex][vertex];

			if (pathLength + distance > m_Instance.n) {
				toRemove.insert(vertex);
				continue;
			}

			if (m_Pheromone[currentVertex][vertex] > m_Pheromone[currentVertex][nextVertex]) {
				nextVertex = vertex;
			}
		}

		// remove not available edges
		for (int vertex : toRemove) {
			availableVertices.erase(vertex);
		}

		// end when there are no more available edges
		if (availableVertices.empty()) {
			break;
		}

		result.push_back(nextVertex);
		pathLength += currentVertex == size ? m_Instance.l : m_Instance.adjMatrix[currentVertex][nextVertex];
		availableVertices.erase(nextVertex);
		currentVertex = nextVertex;
	}

	return result;
}

std::string AntColony::PheromeneToString() const {
	int size = m_Instance.oligonucleotides.size();

	std::stringstream stream;

	stream << '\n';
	for (int i = 0; i < size + 1; i++) {
		for (int j = 0; j < size; j++) {
			stream << std::setw(15) << std::fixed << std::setprecision(3) << m_Pheromone[i][j];
		}
		stream << '\n';
	}

	return stream.str();
}
