#pragma once
#include <string>
#include <vector>
#include <filesystem>
#include <fstream>

struct Edge
{
    size_t index; // neighbour index
    int weight;
    std::string sequencePart;
};

class Instance
{
public:
    Instance(std::filesystem::path filepath);
    size_t outputLength(const std::vector<size_t>& solution) const;

private:
    void extractInstanceInfo();
    int bestMatch(const std::string o1, const std::string o2);

public:
    void buildAdjMatrix();
    void buildAdjList();

    enum ErrorType
    {
        NONE,
        NEGATIVE_RANDOM,
        NEGATIVE_REPEAT,
        POSITIVE_RANDOM,
        POSITIVE_WRONG_ENDING
    };

    std::filesystem::path filepath;
    ErrorType errorType = NONE;
    size_t numErrors = 0;
    size_t n = 0; // dna sequence length
    size_t s = 0; // number of oligonucleotides in original spectrum
    size_t l = 0; // oligonucleotide length
    size_t bestSolutionSize = 0;
    std::string name{};
    std::vector<std::string> oligonucleotides{};
    std::vector<std::vector<int>> adjMatrix;
    std::vector<std::vector<Edge>> adjList;
};