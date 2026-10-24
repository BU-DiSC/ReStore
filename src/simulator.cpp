#include "simulator.h"
#include <iostream>
#include <cstdlib>
#include <sstream>
#include <filesystem>
#include <chrono>
#include <thread>
#include <vector>
#include <unistd.h>
#include <sys/wait.h>

ReStoreSimulator::ReStoreSimulator() {
    initializePolicyRunners();
}

// Prefer the policy executable that sits next to this `restore` binary (e.g. build/Run_RL),
// so the CLI works straight from the build directory; otherwise fall back to a PATH lookup.
static std::string resolvePolicyExecutable(const std::string& executable) {
    std::error_code ec;
    std::filesystem::path self = std::filesystem::read_symlink("/proc/self/exe", ec);
    if (!ec) {
        std::filesystem::path candidate = self.parent_path() / executable;
        if (std::filesystem::exists(candidate, ec)) {
            return candidate.string();
        }
    }
    return executable;
}

// Static helper function to build command with working directory
std::string ReStoreSimulator::buildCommandWithWorkdir(const std::string& executable, const std::map<std::string, std::string>& config) {
    std::ostringstream args;

    // Add working directory as an argument (a -workdir in config, if given, comes later and wins)
    char* cwd = getcwd(nullptr, 0);
    if (cwd) {
        args << "-workdir=\"" << cwd << "\" ";
        free(cwd);
    }

    // Tier device paths are passed to the policy executable as environment variables
    // (see tierDevicePath() in cpp/device_driver/ReStore_device.c); tiers without a device are simulated.
    static const std::map<std::string, std::string> device_env = {
        {"fast_device", "RESTORE_FAST_DEVICE"},
        {"middle_device", "RESTORE_MIDDLE_DEVICE"},
        {"slow_device", "RESTORE_SLOW_DEVICE"},
    };
    std::ostringstream env;

    for (const auto& [key, value] : config) {
        auto it = device_env.find(key);
        if (it != device_env.end()) {
            env << it->second << "=\"" << value << "\" ";
        } else {
            args << "-" << key << "=\"" << value << "\" ";
        }
    }

    return env.str() + "\"" + resolvePolicyExecutable(executable) + "\" " + args.str();
}

void ReStoreSimulator::initializePolicyRunners() {
    policy_runners["RL"] = std::make_unique<RLRunner>();
    policy_runners["RL_device"] = std::make_unique<RLDeviceRunner>();
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
    
    // std::system() returns a wait status, not an exit code: decode it so that crashes and
    // errors in the policy executable are reported (and propagated) as failures.
    int status = policy_runners[policy]->run(config);
    int result = 1;
    std::string failure;
    if (status == -1) {
        failure = "could not start the policy executable";
    } else if (WIFEXITED(status)) {
        result = WEXITSTATUS(status);
        if (result == 127) failure = "policy executable not found (is it built/installed?)";
        else if (result != 0) failure = "exit code " + std::to_string(result);
    } else if (WIFSIGNALED(status)) {
        result = 128 + WTERMSIG(status);
        failure = "terminated by signal " + std::to_string(WTERMSIG(status));
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::seconds>(end_time - start_time);

    if (result == 0) {
        std::cout << "Simulation completed successfully in " << duration.count() << " seconds." << std::endl;
    } else {
        std::cerr << "Simulation failed: " << failure << std::endl;
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

int RLDeviceRunner::run(const std::map<std::string, std::string>& config) {
    std::string command = ReStoreSimulator::buildCommandWithWorkdir("Run_RL_device", config);
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

