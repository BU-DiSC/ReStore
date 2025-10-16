#include "simulator.h"
#include <iostream>
#include <cstdlib>
#include <sstream>
#include <filesystem>
#include <chrono>
#include <thread>
#include <vector>
#include <unistd.h>

ReStoreSimulator::ReStoreSimulator() {
    initializePolicyRunners();
}

// Static helper function to build command with working directory
std::string ReStoreSimulator::buildCommandWithWorkdir(const std::string& executable, const std::map<std::string, std::string>& config) {
    std::ostringstream args;
    
    // Add working directory as an argument
    char* cwd = getcwd(nullptr, 0);
    if (cwd) {
        args << "-workdir=" << cwd << " ";
        free(cwd);
    }
    
    for (const auto& [key, value] : config) {
        args << "-" << key << "=" << value << " ";
    }
    
    return executable + " " + args.str();
}

void ReStoreSimulator::initializePolicyRunners() {
    policy_runners["RL"] = std::make_unique<RLRunner>();
    policy_runners["LFU"] = std::make_unique<LFURunner>();
    policy_runners["LRU"] = std::make_unique<LRURunner>();
    policy_runners["LRFU"] = std::make_unique<LRFURunner>();
    policy_runners["EXD"] = std::make_unique<EXDRunner>();
    policy_runners["TEMP"] = std::make_unique<TEMPRunner>();
    policy_runners["static"] = std::make_unique<StaticRunner>();
    policy_runners["ideal"] = std::make_unique<IdealRunner>();
    policy_runners["LogiReg"] = std::make_unique<LogiRegRunner>();
    policy_runners["XGBfast"] = std::make_unique<XGBfastRunner>();
}

int ReStoreSimulator::run(const std::string& policy, const std::map<std::string, std::string>& config) {
    if (!isPolicyAvailable(policy)) {
        std::cerr << "Error: Policy '" << policy << "' is not available." << std::endl;
        return 1;
    }
    
    std::cout << "Running ReStore simulation with policy: " << policy << std::endl;
    std::cout << "Workload: " << config.at("workload") << std::endl;
    
    auto start_time = std::chrono::high_resolution_clock::now();
    
    int result = policy_runners[policy]->run(config);
    
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::seconds>(end_time - start_time);
    
    if (result == 0) {
        std::cout << "Simulation completed successfully in " << duration.count() << " seconds." << std::endl;
    } else {
        std::cout << "Simulation failed with exit code: " << result << std::endl;
    }
    
    return result;
}

bool ReStoreSimulator::isPolicyAvailable(const std::string& policy) const {
    return policy_runners.find(policy) != policy_runners.end();
}

std::vector<std::string> ReStoreSimulator::getAvailablePolicies() const {
    std::vector<std::string> policies;
    for (const auto& [policy, _] : policy_runners) {
        policies.push_back(policy);
    }
    return policies;
}

std::string ReStoreSimulator::buildCommandLineArgs(const std::string& policy, const std::map<std::string, std::string>& config) {
    std::ostringstream args;
    
    for (const auto& [key, value] : config) {
        args << "-" << key << "=" << value << " ";
    }
    
    return args.str();
}

int ReStoreSimulator::executePolicy(const std::string& policy, const std::map<std::string, std::string>& config) {
    std::string executable = policy_runners[policy]->getExecutableName();
    std::string args = buildCommandLineArgs(policy, config);
    
    std::string command = "./" + executable + " " + args;
    
    std::cout << "Executing: " << command << std::endl;
    
    return std::system(command.c_str());
}

// Policy Runner Implementations

int RLRunner::run(const std::map<std::string, std::string>& config) {
    std::string command = ReStoreSimulator::buildCommandWithWorkdir("Run_RL", config);
    std::cout << "Executing: " << command << std::endl;
    
    return std::system(command.c_str());
}

int LFURunner::run(const std::map<std::string, std::string>& config) {
    std::string command = ReStoreSimulator::buildCommandWithWorkdir("Run_LFU", config);
    std::cout << "Executing: " << command << std::endl;
    
    return std::system(command.c_str());
}

int LRURunner::run(const std::map<std::string, std::string>& config) {
    std::string command = ReStoreSimulator::buildCommandWithWorkdir("Run_LRU", config);
    std::cout << "Executing: " << command << std::endl;
    
    return std::system(command.c_str());
}

int LRFURunner::run(const std::map<std::string, std::string>& config) {
    std::string command = ReStoreSimulator::buildCommandWithWorkdir("Run_LRFU", config);
    std::cout << "Executing: " << command << std::endl;
    
    return std::system(command.c_str());
}

int EXDRunner::run(const std::map<std::string, std::string>& config) {
    std::string command = ReStoreSimulator::buildCommandWithWorkdir("Run_EXD", config);
    std::cout << "Executing: " << command << std::endl;
    
    return std::system(command.c_str());
}

int TEMPRunner::run(const std::map<std::string, std::string>& config) {
    std::string command = ReStoreSimulator::buildCommandWithWorkdir("Run_TEMP", config);
    std::cout << "Executing: " << command << std::endl;
    
    return std::system(command.c_str());
}

int StaticRunner::run(const std::map<std::string, std::string>& config) {
    std::string command = ReStoreSimulator::buildCommandWithWorkdir("Run_static", config);
    std::cout << "Executing: " << command << std::endl;
    
    return std::system(command.c_str());
}

int IdealRunner::run(const std::map<std::string, std::string>& config) {
    std::string command = ReStoreSimulator::buildCommandWithWorkdir("Run_ideal", config);
    std::cout << "Executing: " << command << std::endl;
    
    return std::system(command.c_str());
}

int LogiRegRunner::run(const std::map<std::string, std::string>& config) {
    std::string command = ReStoreSimulator::buildCommandWithWorkdir("Run_LogiReg", config);
    std::cout << "Executing: " << command << std::endl;
    
    return std::system(command.c_str());
}

int XGBfastRunner::run(const std::map<std::string, std::string>& config) {
    std::string command = ReStoreSimulator::buildCommandWithWorkdir("Run_XGBfast", config);
    std::cout << "Executing: " << command << std::endl;
    
    return std::system(command.c_str());
}

