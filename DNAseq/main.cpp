#include <iostream>
#include <thread>
#include <mutex>

#include "Instance.h"
#include "LocalSearch.h"

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
        // use AntColony and LocalSearch
        return 1;
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

void loadInstance(std::filesystem::path path, std::vector<Instance>* tests, std::mutex* mutex)
{
    Instance&& instance = Instance{ path };
    mutex->lock();
    std::cout << "Loaded " << path << std::endl;
    tests->push_back(instance);
    mutex->unlock();
}

int main()
{
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
        break;// DEBUG: hard coded to test only one instance
    }

    for (auto& worker : workers)
    {
        worker.join();
    }

    Solution solution{ 0, 1, 2, 3, 4 };
    LocalSearch ls = LocalSearch{ tests[0], solution };
    ls.run();

    STSP_Sequencer perfectSequencer;
    Our_Sequencer ourSequencer{};

    Tester::test(ourSequencer, tests);
    Tester::compare(perfectSequencer, ourSequencer, tests);

    return 0;
}

