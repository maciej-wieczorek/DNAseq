#include <iostream>
#include <filesystem>
#include <fstream>
#include <vector>
#include <string>
#include <type_traits>

#define STRINGIFY(x) #x

#define EXPAND(x) STRINGIFY(x)

#ifdef PROJECT_PATH
    std::string projectPath = EXPAND(PROJECT_PATH);
#else
    std::string projectPath{ "path/to/project" };
#endif

#define FIXED_FLOAT(x) std::fixed << std::setprecision(3) << (x)

class Instance
{
public:
    Instance(std::filesystem::path filepath)
        : filepath{ filepath }
    {
        extractInstanceInfo();
    }

    void extractInstanceInfo()
    {
        name = filepath.filename().string();
        l = 10;

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
};

class Sequencer
{
public:
    virtual size_t run(const Instance& instance) const = 0; // returns number of oligonucleotides used
    virtual std::string getName() const = 0;
};

class STSP_Sequencer : public Sequencer
{
public:
    virtual size_t run(const Instance& instance) const override
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
    virtual size_t run(const Instance& instance) const override
    {
        // TODO: implement our Sequencer
        return 1;
    }

    virtual std::string getName() const override
    {
        return "Our Sequencer";
    }
};

class Tester
{
    using Insts = std::vector<Instance>;

public:

    static void test(const Sequencer& s, const Insts& tests)
    {
        std::cout << "##### RUNNING TEST ON: " << s.getName() << " #####\n";
        for (const Instance& instance : tests)
        {
            size_t used = s.run(instance);
            float acc = used / (float)instance.s;
            std::cout << instance.name << ":\t" << FIXED_FLOAT(acc) << '\n';
        }
    }

    static void compare(const Sequencer& s1, const Sequencer& s2, const Insts& tests)
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
        Instance instance{entry.path()};

        std::ifstream file{ entry.path() };
        if (file.is_open())
        {
            while (file)
            {
                std::string oligonucleotide;
                std::getline(file, oligonucleotide);
                if (oligonucleotide.size() == instance.l)
                    instance.oligonucleotides.push_back(std::move(oligonucleotide));
            }
            file.close();
        }

        tests.push_back(std::move(instance));
    }

    STSP_Sequencer perfectSequencer;
    Our_Sequencer ourSequencer{};

    Tester::test(ourSequencer, tests);
    Tester::compare(perfectSequencer, ourSequencer, tests);

    return 0;
}

