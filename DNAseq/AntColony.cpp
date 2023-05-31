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

	m_Pheromone = std::vector<std::vector<float>>(size + 1, std::vector<float>(size, 1.f));
	m_PheromoneDeposited = std::vector<std::vector<float>>(size + 1, std::vector<float>(size, 0.f));
	m_Weights = std::vector<float>(size, 1.f);
}

AntColony::~AntColony() {}

std::vector<int> AntColony::Run() {

	for (int i = 0; i < m_Parameters.Iterations; i++) {
		Iteration();

		LOG_TRACE("ant colony: {} / {}", i + 1, m_Parameters.Iterations);
	}

	// LOG_TRACE("pheromone:{}", PheromeneToString());

	return Result();
}

void AntColony::Iteration() {
	int size = m_Instance.oligonucleotides.size();

	// pheromene deposited
	for (int i = 0; i < size + 1; i++) {
		for (int j = 0; j < size; j++) {
			m_PheromoneDeposited[i][j] = (0.f);
		}
	}

	for (int a = 0; a < m_Parameters.Ants; a++) {
		int size = m_Instance.oligonucleotides.size();

		int pathLength = 0;
		int currentVertex = size;

		std::vector<int> path;
		path.push_back(currentVertex);

		for (int i = 0; i < size; i++) {
			m_Weights[i] = 1.f;
		}

		// generate ant path
		while (true) {
			//std::map<int, float> weights;
			float weightsSum = 0;

			// calculate weights for available edges
			int availableVertices = 0;

			for (int i = 0; i < size; i++) {
				if (m_Weights[i] < 0.f) {
					continue;
				}

				// distance from current vertex and available vertex
				int distance = currentVertex == size ? m_Instance.l : m_Instance.adjMatrix[currentVertex][i];

				if (pathLength + distance > m_Instance.n) {
					m_Weights[i] = -1.f;
					continue;
				}

				// calculate weight of edge
				float weight = std::powf(m_Pheromone[currentVertex][i], m_Parameters.Alpha) * std::powf(1.f / (float)distance, m_Parameters.Beta);
				m_Weights[i] = weight;
				weightsSum += weight;
				availableVertices++;
			}

			// end when there are no more available edges
			if (availableVertices == 0) {
				break;
			}

			// select random edge
			int nextVertex = -1;
			float random = (float)rand() / (float)RAND_MAX * weightsSum;
			for (int i = 0; i < size; i++) {
				if (m_Weights[i] < 0.f) {
					continue;
				}
				random -= m_Weights[i];
				if (random < 0.f) {
					nextVertex = i;
					break;
				}
			}
			if (nextVertex == -1) {
				for (int i = 0; i < size; i++) {
					if (m_Weights[i] >= 0.f) {
						nextVertex = i;
						break;
					}
				}
			}

			path.push_back(nextVertex);
			pathLength += currentVertex == size ? m_Instance.l : m_Instance.adjMatrix[currentVertex][nextVertex];
			m_Weights[nextVertex] = -1.f;
			currentVertex = nextVertex;
		}

		// calculate added pheromone
		for (int i = 0; i < path.size() - 1; i++) {
			m_PheromoneDeposited[path[i]][path[i + 1]] += (float)path.size() / (float)m_Instance.bestSolutionSize;
		}
	}


	// update pheromone
	for (int i = 0; i < size + 1; i++) {
		for (int j = 0; j < size; j++) {
			m_Pheromone[i][j] = (1.f - m_Parameters.Evaporation) * m_Pheromone[i][j] + m_PheromoneDeposited[i][j];
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
		int maxPheromone = 0;

		for (int vertex : availableVertices) {
			// distance from current vertex and available vertex
			int distance = currentVertex == size ? m_Instance.l : m_Instance.adjMatrix[currentVertex][vertex];

			if (pathLength + distance > m_Instance.n) {
				toRemove.insert(vertex);
				continue;
			}

			if (m_Pheromone[currentVertex][vertex] > maxPheromone) {
				nextVertex = vertex;
				maxPheromone = m_Pheromone[currentVertex][vertex];
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
