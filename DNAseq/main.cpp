#include <iostream>
#include <filesystem>
#include <fstream>
#include <vector>
#include <string>
#include <set>
#include <algorithm>
#include <limits>
#include <stack>
#include <array>

#define STRINGIFY(x) #x

#define EXPAND(x) STRINGIFY(x)

#ifdef PROJECT_PATH
    std::string projectPath = EXPAND(PROJECT_PATH);
#else
    std::string projectPath{ "path/to/project" };
#endif

#define FIXED_FLOAT(x) std::fixed << std::setprecision(3) << (x)

struct Edge
{
	size_t index; // neighbour index
	int weight;
	std::string sequencePart;
	// example: T[AAAA] & [AAAA]C -> weight = 0, sequencePart = "C" 
	// example: TC[T] & [T]GG -> weight = 1, sequencePart = "GG" 
	// example: ATCA[AG-CAAC] & [AGACTAC]TC -> weight = ?, sequencePart = "TC" 
};

class Instance
{
public:
    Instance(std::filesystem::path filepath)
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
        buildAdjList();
    }

private:
    void extractInstanceInfo()
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
            throw std::runtime_error{"unexpected filename"};

        s = std::stoi(name.substr(dotPos + 1, numErrorsOffset - dotPos - 1));
        n = s + l - 1;
        numErrors = std::stoi(name.substr(numErrorsOffset+1, name.size() - numErrorsOffset));

        if (minusPos != std::string::npos && numErrors >= 40)
            errorType = NEGATIVE_RANDOM;
        else if (minusPos != std::string::npos)
            errorType = NEGATIVE_REPEAT;
        else if (plusPos != std::string::npos && numErrors >= 80)
            errorType = POSITIVE_RANDOM;
        else if (plusPos != std::string::npos)
            errorType = POSITIVE_WRONG_ENDING;
    }

    // nucleotide compatibility
    inline int compat(char n1, char n2)
    {
        if (n1 == n2)
            return 1;
        else
            return -1;
    }

    // TODO: implement levenshtein distance
    // returns pair: <weight, sequencePart>
    std::pair<int, std::string> bestMatch(const std::string o1, const std::string o2)
    {
        size_t n = o1.size() + 1;
        size_t m = o2.size() + 1;

        int g = -2; // space cost

        std::vector<std::vector<int>> M(n, std::vector<int>(m));
        
        for (size_t i = 0; i < n; ++i)
        {
            for (size_t j = 0; j < m; ++j)
            {
                if (i == 0 || j == 0)
                {
                    M[i][j] = 0;
                }
                else
                {
                    M[i][j] = std::max({
                        M[i - 1][j - 1] + compat(o1[i-1], o2[j-1]),
                        M[i - 1][j] + g,
                        M[i][j - 1] + g
                        });
                }
            }
        }

        // find best distance and index (semi-global match)
        std::vector<int>& lastRow = M[M.size() - 1];
        int bestDistance = lastRow[0];
        size_t bestDistanceIndex = 0;
        for (size_t i = 0; i < lastRow.size() - 1; ++i) // don't check last element 
        {
            int distance = lastRow[i];
            if (distance > bestDistance)
            {
                bestDistance = distance;
                bestDistanceIndex = i;
            }   
        }

        std::string sequencePart = o2.substr(bestDistanceIndex, lastRow.size() - bestDistanceIndex - 1);

        return std::make_pair(bestDistance, sequencePart);
    }

public:
    void buildAdjMatrix()
    {
        for (size_t i = 0; i < oligonucleotides.size(); ++i)
        {
            adjMatrix.push_back(std::vector<int>{});

            for (size_t j = 0; j < oligonucleotides.size(); ++j)
            {
                if (i != j)
                {
                    int weight = bestMatch(oligonucleotides[i], oligonucleotides[j]).first;
                    adjMatrix[i].push_back(weight);
                }
                else
                {
                    adjMatrix[i].push_back(std::numeric_limits<int>::min());
                }
            }
        }
    }

    void buildAdjList()
    {
        for (size_t i = 0; i < oligonucleotides.size(); ++i)
        {
            adjList.push_back(std::vector<Edge>{});

            for (size_t j = 0; j < oligonucleotides.size(); ++j)
            {
                if (i != j)
                {
					std::pair<int, std::string> match = bestMatch(oligonucleotides[i], oligonucleotides[j]);
                    if (match.first < l)
                    {
                        adjList[i].push_back(Edge{j, match.first, match.second});
                    }
                }
            }

            std::sort(adjList[i].begin(), adjList[i].end(),
                [](const Edge& e1, const Edge& e2) { return e1.weight > e2.weight; });
        }
    }

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
    size_t s = 0; // number of oligonucleotides
    size_t l = 0; // oligonucleotide length
    std::string name{};
    std::vector<std::string> oligonucleotides{};
    std::vector<std::vector<int>> adjMatrix;
    std::vector<std::vector<Edge>> adjList;
};

class Sequencer
{
public:
    virtual ~Sequencer() {};
    virtual size_t run(const Instance& instance) = 0; // returns number of oligonucleotides used
    virtual std::string getName() const = 0;
};

class STSP_Sequencer : public Sequencer
{
public:
    virtual size_t run(const Instance& instance) override
    {
        // TODO: implement STSP Sequencer
        return 0.9f * instance.s;
    }

    virtual std::string getName() const override
    {
        return "STSP Sequencer";
    }
};

class Our_Sequencer : public Sequencer
{
public:
    virtual size_t run(const Instance& instance) override
    {
        // TODO: implement our Sequencer
        this->instance = &instance;
        visited.clear();
        sequence.clear();
        sequenceParts = std::stack<std::string>{};
        N = instance.n;
        maxGain = std::numeric_limits<size_t>::min();
        maxLevel = 0;
        
        visited.insert(0);
        sequenceParts.push(instance.oligonucleotides[0]);
        stsp(0, 0, instance.l);

        sequence = makeSequence(bestSequenceParts);

        return bestSequenceParts.size();
    }

    virtual std::string getName() const override
    {
        return "Our Sequencer";
    }

private:
    std::set<int> visited{};
    std::string sequence{};
    std::stack<std::string> sequenceParts{};
    std::stack<std::string> bestSequenceParts{};
    size_t N = 0;
    size_t maxGain = std::numeric_limits<size_t>::min();
    size_t maxLevel = 0;
    const Instance* instance;

    void stsp(size_t currPos, size_t gain, size_t length)
    { 
		// If all the selected nodes have been visited 
		if (length == N)
        { 
            if (gain > maxGain)
            {
                maxLevel = visited.size();
                maxGain = gain;
                bestSequenceParts = sequenceParts;
            }
			return; 
		} 

		// If the current cost exceeds the minimum cost found so far 
        /*
		if (cost + (N - visited.size()) * findMin(currPos) >= ans)
        { 
			return; 
		} 
        */

		// Iterate over all unvisited nodes 
        for (size_t i = 0; i < instance->adjList[currPos].size(); ++i)
        {
            Edge next = instance->adjList[currPos][i];
            if (visited.find(next.index) == visited.end())
            {
                visited.insert(next.index);
                sequenceParts.push(next.sequencePart);
                
                stsp(next.index, gain + instance->adjMatrix[currPos][next.index], length + next.sequencePart.size());

                sequenceParts.pop();
                visited.erase(next.index);
            }
        }
    } 

    std::string makeSequence(std::stack<std::string> parts)
    {
        std::vector<std::string> partsVec{};
        while (!parts.empty())
        {
            partsVec.push_back(parts.top());
            parts.pop();
        }

        std::string sequence{};
        for (int i = partsVec.size() - 1; i >= 0; --i)
        {
            sequence += partsVec[i];
        }

        return sequence;
    }

};

class Tester
{
    using Insts = std::vector<Instance>;

public:

    static void test(Sequencer& s, Insts& tests)
    {
        std::cout << "##### RUNNING TEST ON: " << s.getName() << " #####\n";
        for (const Instance& instance : tests)
        {
            size_t used = s.run(instance);
            float acc = used / (float)instance.s;
            std::cout << instance.name << ":\t" << FIXED_FLOAT(acc) << '\n';
        }
    }

    static void compare(Sequencer& s1, Sequencer& s2, const Insts& tests)
    {
        std::cout << "##### RUNNING COMPARISON BETWEEN: " << s1.getName()
                  << " AND " << s2.getName() << " #####\n";
        for (const Instance& instance : tests)
        {
            size_t u1 = s1.run(instance);
            size_t u2 = s2.run(instance);
            float acc1 = u1 / (float)instance.s;
            float acc2 = u2 / (float)instance.s;
            float diff = acc2 - acc1;
            std::cout << instance.name << ":\t" << FIXED_FLOAT(acc1) << '\t'
                      << FIXED_FLOAT(acc2) << '\t' << FIXED_FLOAT(diff) << '\n';
        }
    }
};

int main()
{
#ifdef PROJECT_PATH
    projectPath.erase(0, 1); // erase the first quote
    projectPath.erase(projectPath.size() - 2); // erase the last quote and the dot
#endif // PROJECT_PATH

    std::vector<Instance> tests;
    std::filesystem::path path{ projectPath + "/tests" };
    for (const auto& entry : std::filesystem::directory_iterator(path))
    {
        tests.push_back(Instance{entry.path()});
        break;// DEBUG: hard coded to test only one instance
    }

    STSP_Sequencer perfectSequencer;
    Our_Sequencer ourSequencer{};

    Tester::test(ourSequencer, tests);
    Tester::compare(perfectSequencer, ourSequencer, tests);

    return 0;
}

