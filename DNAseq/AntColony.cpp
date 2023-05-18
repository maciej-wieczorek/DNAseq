#include "AntColony.h"
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <algorithm>
#include <map>

AntColony::AntColony(const Instance& instance, const Parameters& parameters)
	: m_Instance(instance), m_Parameters(parameters) {
	m_Pheromone = new float* [m_Instance.s + 1];
	for (int i = 0; i < m_Instance.s + 1; i++) {
		m_Pheromone[i] = new float[m_Instance.s];
		for (int j = 0; j < m_Instance.s; j++) {
			m_Pheromone[i][j] = 1.f;
		}
	}
}

AntColony::~AntColony() {
	for (int i = 0; i < m_Instance.s + 1; i++) {
		delete[] m_Pheromone[i];
	}
	delete[] m_Pheromone;
}

std::vector<int> AntColony::Run() {
	for (int i = 0; i < m_Parameters.Iterations; i++) {
		Iteration();
	}
	return Result();
}

void AntColony::Iteration() {
	int** ants = new int* [m_Instance.s + 1];
	for (int i = 0; i < m_Instance.s + 1; i++) {
		ants[i] = new int[m_Instance.s];
		for (int j = 0; j < m_Instance.s; j++) {
			ants[i][j] = 0;
		}
	}

	for (int i = 0; i < m_Parameters.Ants; i++) {
		std::vector<int> path = GeneratePath();
		int curretnt = m_Instance.s;
		for (int next : path) {
			ants[curretnt][next]++;
			curretnt = next;
		}
	}

	UpdatePheromone(ants);

	std::cout << "Pheromone:" << std::endl;
	for (int i = 0; i < m_Instance.s + 1; i++) {
		for (int j = 0; j < m_Instance.s; j++) {
			std::cout << std::setw(6) << std::setprecision(3) << m_Pheromone[i][j] << " ";
		}
		std::cout << std::endl;
	}
	std::cout << std::endl;
}
std::vector<int> AntColony::GeneratePath() {
	std::map<int, float> options;
	for (int i = 0; i < m_Instance.s; i++) {
		options[i] = 0.f;
	}

	std::vector<int> path;
	int current = m_Instance.s;
	while (!options.empty()) {
		float sum = 0.f;
		for (auto& next : options) {
			next.second =
				powf(m_Pheromone[current][next.first], m_Parameters.Alpha)
				* (current < m_Instance.s ? powf(1.f / (float)m_Instance.adjMatrix[current][next.first], m_Parameters.Beta) : 1.f);
			sum += next.second;
		}

		int nextIndex = -1;
		float random = (float)rand() / RAND_MAX * sum;
		for (auto& next : options) {
			if (random <= next.second) {
				nextIndex = next.first;
			}
			random -= next.second;
		}
		if (nextIndex < 0) {
			nextIndex = options.rbegin()->first;
		}
		path.push_back(nextIndex);
		options.erase(nextIndex);
		current = nextIndex;
	}

	return path;
}
void AntColony::UpdatePheromone(int** ants) {
	for (int i = 0; i < m_Instance.s + 1; i++) {
		for (int j = 0; j < m_Instance.s; j++) {
			m_Pheromone[i][j] =
				(1.f - m_Parameters.Evaporation) * m_Pheromone[i][j]
				+ (float)ants[i][j] / (i < m_Instance.s ? (float)m_Instance.adjMatrix[i][j] : 1.f);
		}
	}
}

std::vector<int> AntColony::Result() {
	std::vector<int> result;
	int currentIndex = m_Instance.s;
	for (int i = 0; i < m_Instance.s; i++) {
		int nextIndex = -1;
		for (int i = 0; i < m_Instance.s; i++) {
			if (i == currentIndex) {
				continue;
			}
			if (std::find(result.begin(), result.end(), i) != result.end()) {
				continue;
			}
			if (nextIndex < 0) {
				nextIndex = i;
				continue;
			}
			if (m_Pheromone[currentIndex][i] > m_Pheromone[currentIndex][nextIndex]) {
				nextIndex = i;
			}
		}
		result.push_back(nextIndex);
	}
	return result;
}
