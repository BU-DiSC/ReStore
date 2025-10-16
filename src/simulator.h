#ifndef SIMULATOR_H
#define SIMULATOR_H

#include <string>
#include <map>
#include <memory>
#include <vector>

// Forward declarations for policy implementations
class PolicyRunner;

class ReStoreSimulator {
private:
    std::map<std::string, std::unique_ptr<PolicyRunner>> policy_runners;
    
    void initializePolicyRunners();
    std::string buildCommandLineArgs(const std::string& policy, const std::map<std::string, std::string>& config);
    int executePolicy(const std::string& policy, const std::map<std::string, std::string>& config);

public:
    // Static helper function for building commands with workdir
    static std::string buildCommandWithWorkdir(const std::string& executable, const std::map<std::string, std::string>& config);
    
public:
    ReStoreSimulator();
    ~ReStoreSimulator() = default;
    
    int run(const std::string& policy, const std::map<std::string, std::string>& config);
    bool isPolicyAvailable(const std::string& policy) const;
    std::vector<std::string> getAvailablePolicies() const;
};

// Abstract base class for policy runners
class PolicyRunner {
public:
    virtual ~PolicyRunner() = default;
    virtual int run(const std::map<std::string, std::string>& config) = 0;
    virtual std::string getExecutableName() const = 0;
};

// Concrete policy runner implementations
class RLRunner : public PolicyRunner {
public:
    int run(const std::map<std::string, std::string>& config) override;
    std::string getExecutableName() const override { return "Run_RL"; }
};

class LFURunner : public PolicyRunner {
public:
    int run(const std::map<std::string, std::string>& config) override;
    std::string getExecutableName() const override { return "Run_LFU"; }
};

class LRURunner : public PolicyRunner {
public:
    int run(const std::map<std::string, std::string>& config) override;
    std::string getExecutableName() const override { return "Run_LRU"; }
};

class LRFURunner : public PolicyRunner {
public:
    int run(const std::map<std::string, std::string>& config) override;
    std::string getExecutableName() const override { return "Run_LRFU"; }
};

class EXDRunner : public PolicyRunner {
public:
    int run(const std::map<std::string, std::string>& config) override;
    std::string getExecutableName() const override { return "Run_EXD"; }
};

class TEMPRunner : public PolicyRunner {
public:
    int run(const std::map<std::string, std::string>& config) override;
    std::string getExecutableName() const override { return "Run_TEMP"; }
};

class StaticRunner : public PolicyRunner {
public:
    int run(const std::map<std::string, std::string>& config) override;
    std::string getExecutableName() const override { return "Run_static"; }
};

class IdealRunner : public PolicyRunner {
public:
    int run(const std::map<std::string, std::string>& config) override;
    std::string getExecutableName() const override { return "Run_ideal"; }
};

class LogiRegRunner : public PolicyRunner {
public:
    int run(const std::map<std::string, std::string>& config) override;
    std::string getExecutableName() const override { return "Run_LogiReg"; }
};

class XGBfastRunner : public PolicyRunner {
public:
    int run(const std::map<std::string, std::string>& config) override;
    std::string getExecutableName() const override { return "Run_XGBfast"; }
};

#endif // SIMULATOR_H

