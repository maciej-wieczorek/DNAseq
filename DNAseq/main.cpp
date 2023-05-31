#include <iostream>
#include <thread>
#include <mutex>

#include "AntColony.h"
#include "Instance.h"
#include "LocalSearch.h"
#include "Logger.h"

#define STRINGIFY(x) #x

#define EXPAND(x) STRINGIFY(x)

#ifdef PROJECT_PATH
    std::string projectPath = EXPAND(PROJECT_PATH);
#else
    std::string projectPath{ "path/to/project" };
#endif

#define FIXED_FLOAT(x) std::fixed << std::setprecision(3) << (x)

class Sequencer
{
public:
    virtual ~Sequencer() {};
    virtual size_t run(const Instance& instance) = 0; // returns number of oligonucleotides used
    virtual std::string getName() const = 0;
};

class Our_Sequencer : public Sequencer
{
public:
    virtual size_t run(const Instance& instance) override
    {
        // use AntColony and LocalSearch
        AntColony antColony(instance, AntColony::Parameters(100, 100, 1.f, 1.f, 0.1f));
        std::vector<int> result = antColony.Run();

        Solution lsInput = Solution{ result.begin(), result.end() };
        LocalSearch localSearch(instance, lsInput);
        Solution improvedResult = localSearch.run();

        for (size_t vertex : improvedResult) {
            std::cout << vertex << " ";
        }
        std::cout << std::endl;

        size_t outputLength = instance.outputLength(improvedResult);
        std::cout << "length: " << outputLength << "/" << instance.n << std::endl;

        return result.size();
    }

    virtual std::string getName() const override
    {
        return "Our Sequencer";
    }

private:
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
            float acc = used / (float)instance.bestSolutionSize;
            LOG_INFO("{} acc: {}/{} = {}", instance.name, used, instance.bestSolutionSize, acc);
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

void loadInstance(std::filesystem::path path, std::vector<Instance>* tests, std::mutex* mutex)
{
    Instance&& instance = Instance{ path };
    mutex->lock();
    LOG_INFO("Loaded {}", path.string());
    tests->push_back(instance);
    mutex->unlock();
}

int main() {
    srand(time(nullptr));
    Logger::Init();

    LOG_INFO("Logger initialized.");

    // example logs:
    // int variable = 200;
    // LOG_TRACE("message {} / {} ({})", 123, variable, "text");
    // LOG_INFO("message {} / {} ({})", 123, variable, "text");
    // LOG_WARN("message {} / {} ({})", 123, variable, "text");
    // LOG_ERROR("message {} / {} ({})", 123, variable, "text");
    // LOG_CRITICAL("message {} / {} ({})", 123, variable, "text");

#ifdef PROJECT_PATH
    projectPath.erase(0, 1); // erase the first quote
    projectPath.erase(projectPath.size() - 2); // erase the last quote and the dot
#endif // PROJECT_PATH

    std::vector<Instance> tests;
    std::filesystem::path path{ projectPath + "/tests" };

    std::vector<std::thread> workers;
    std::mutex mutex;
    for (const auto& entry : std::filesystem::directory_iterator(path))
    {
        workers.push_back(std::thread(loadInstance, entry.path(), &tests, &mutex));
         break; // DEBUG: hard coded to test only one instance
    }

    for (auto& worker : workers)
    {
        worker.join();
    }

    Our_Sequencer ourSequencer{};
    Tester::test(ourSequencer, tests);

    return 0;
}

