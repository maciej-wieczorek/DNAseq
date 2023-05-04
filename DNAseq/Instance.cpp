#include "Instance.h"

Instance::Instance(std::filesystem::path filepath)
    : filepath{ filepath }
{
    std::ifstream file{ filepath };
    if (file.is_open())
    {
        while (file)
        {
            std::string oligonucleotide;
            std::getline(file, oligonucleotide);
            if (!oligonucleotide.empty())
                oligonucleotides.push_back(std::move(oligonucleotide));
        }
        file.close();
    }
    extractInstanceInfo();

    buildAdjMatrix();
    // buildAdjList();
}

void Instance::extractInstanceInfo()
{
    name = filepath.filename().string();
    l = oligonucleotides[0].size();

    size_t dotPos = name.find('.');
    size_t plusPos = name.find('+');
    size_t minusPos = name.find('-');
    size_t numErrorsOffset = 0;
    if (plusPos != std::string::npos)
        numErrorsOffset = plusPos;
    else if (minusPos != std::string::npos)
        numErrorsOffset = minusPos;
    else
        throw std::runtime_error{ "unexpected filename" };

    s = std::stoi(name.substr(dotPos + 1, numErrorsOffset - dotPos - 1));
    n = s + l - 1;
    numErrors = std::stoi(name.substr(numErrorsOffset + 1, name.size() - numErrorsOffset));

    if (minusPos != std::string::npos && numErrors >= 40)
        errorType = NEGATIVE_RANDOM;
    else if (minusPos != std::string::npos)
        errorType = NEGATIVE_REPEAT;
    else if (plusPos != std::string::npos && numErrors >= 80)
        errorType = POSITIVE_RANDOM;
    else if (plusPos != std::string::npos)
        errorType = POSITIVE_WRONG_ENDING;
}

int Instance::bestMatch(const std::string o1, const std::string o2)
{
    for (size_t i = 1; i < o1.size(); ++i)
    {
        bool found = true;
        for (size_t j = 0; i + j < o1.size(); ++j)
        {
            if (o1[i + j] != o2[j])
            {
                found = false;
                break;
            }
        }
        if (found)
            return i;
    }

    return o1.size();
}

void Instance::buildAdjMatrix()
{
    for (size_t i = 0; i < oligonucleotides.size(); ++i)
    {
        adjMatrix.push_back(std::vector<int>{});

        for (size_t j = 0; j < oligonucleotides.size(); ++j)
        {
            if (i != j)
            {
                int weight = bestMatch(oligonucleotides[i], oligonucleotides[j]);
                adjMatrix[i].push_back(weight);
            }
            else
            {
                adjMatrix[i].push_back(std::numeric_limits<int>::max());
            }
        }
    }
}

void Instance::buildAdjList()
{
    for (size_t i = 0; i < oligonucleotides.size(); ++i)
    {
        adjList.push_back(std::vector<Edge>{});

        for (size_t j = 0; j < oligonucleotides.size(); ++j)
        {
            if (i != j)
            {
                int weight = bestMatch(oligonucleotides[i], oligonucleotides[j]);
                adjList[i].push_back(Edge{ j, weight, "" });
            }
        }

        std::sort(adjList[i].begin(), adjList[i].end(),
            [](const Edge& e1, const Edge& e2) { return e1.weight > e2.weight; });
    }
}
