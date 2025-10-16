#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <cstdlib>
#include <memory>
#include "config_manager.h"
#include "simulator.h"

class ReStoreCLI {
private:
    std::map<std::string, std::string> config;
    std::string config_file;
    
public:
    ReStoreCLI() : config_file("restore_config.json") {}
    
    int run(int argc, char* argv[]) {
        if (argc < 2) {
            printUsage();
            return 1;
        }
        
        std::string command = argv[1];
        
        if (command == "init") {
            return handleInit(argc, argv);
        } else if (command == "run" || command == "simulate") {
            return handleRun(argc, argv);
        } else if (command == "list") {
            return handleList(argc, argv);
        } else if (command == "config") {
            return handleConfig(argc, argv);
        } else if (command == "build") {
            return handleBuild(argc, argv);
        } else {
            std::cerr << "Unknown command: " << command << std::endl;
            printUsage();
            return 1;
        }
    }
    
private:
    void printUsage() {
        std::cout << "ReStore - Multi-Tiered Storage System Simulator\n\n";
        std::cout << "Usage:\n";
        std::cout << "  restore init -workload <workload> [options]     Initialize configuration for a workload\n";
        std::cout << "  restore init -devices <fast> <middle> <slow>    Configure device paths\n";
        std::cout << "  restore run -policy <policy> -workload <workload> [options]  Run simulation\n";
        std::cout << "  restore list policies                          List available policies\n";
        std::cout << "  restore config [get|set] <key> [value]         Manage configuration\n";
        std::cout << "  restore build [options]                        Build/rebuild executables\n\n";
        std::cout << "Examples:\n";
        std::cout << "  restore init -workload my_workload\n";
        std::cout << "  restore init -devices /dev/nvme0n1 /dev/nvme1n1 /dev/nvme2n1\n";
        std::cout << "  restore run -policy LFU -workload my_workload\n";
        std::cout << "  restore run -policy RL -workload my_workload -max_capacity_tier1 5000\n";
        std::cout << "  restore build -fast_device /dev/sda -middle_device /dev/sdb -slow_device /dev/sdc\n";
    }
    
    int handleInit(int argc, char* argv[]) {
        std::map<std::string, std::string> args = parseArgs(argc, argv);
        
        ConfigManager configMgr;
        
        // Check if this is device configuration
        if (args.find("devices") != args.end()) {
            // Handle device configuration
            if (argc < 5) {
                std::cerr << "Error: Device configuration requires 3 device paths\n";
                std::cerr << "Usage: restore init -devices <fast_device> <middle_device> <slow_device>\n";
                return 1;
            }
            
            std::string fast_device = argv[3];
            std::string middle_device = argv[4];
            std::string slow_device = argv[5];
            
            if (!configMgr.setDevicePaths(fast_device, middle_device, slow_device)) {
                std::cerr << "Error: Failed to set device paths\n";
                return 1;
            }
            
            // Save device configuration
            std::map<std::string, std::string> device_config = configMgr.getDevicePaths();
            if (!configMgr.saveConfig("device_config.json", device_config)) {
                std::cerr << "Error: Failed to save device configuration\n";
                return 1;
            }
            
            std::cout << "Device paths configured:\n";
            std::cout << "  Fast device: " << fast_device << std::endl;
            std::cout << "  Middle device: " << middle_device << std::endl;
            std::cout << "  Slow device: " << slow_device << std::endl;
            std::cout << "Configuration saved to: device_config.json" << std::endl;
            std::cout << "\nRun 'restore build' to rebuild with new device paths.\n";
            
            return 0;
        }
        
        // Handle workload configuration
        if (args.find("workload") == args.end()) {
            std::cerr << "Error: -workload is required for init command\n";
            return 1;
        }
        
        std::string workload = args["workload"];
        
        // Load default configuration for the workload
        if (!configMgr.loadWorkloadDefaults(workload, config)) {
            std::cerr << "Error: Failed to load defaults for workload: " << workload << std::endl;
            return 1;
        }
        
        // Override with any provided arguments
        for (const auto& [key, value] : args) {
            if (key != "workload") {
                config[key] = value;
            }
        }
        
        // Save configuration
        if (!configMgr.saveConfig(config_file, config)) {
            std::cerr << "Error: Failed to save configuration\n";
            return 1;
        }
        
        std::cout << "Configuration initialized for workload: " << workload << std::endl;
        std::cout << "Configuration saved to: " << config_file << std::endl;
        
        return 0;
    }
    
    int handleRun(int argc, char* argv[]) {
        std::map<std::string, std::string> args = parseArgs(argc, argv);
        
        if (args.find("policy") == args.end() || args.find("workload") == args.end()) {
            std::cerr << "Error: Both -policy and -workload are required for run command\n";
            return 1;
        }
        
        std::string policy = args["policy"];
        std::string workload = args["workload"];
        
        // Load configuration
        ConfigManager configMgr;
        if (!configMgr.loadConfig(config_file, config)) {
            std::cerr << "Warning: Could not load config file, using defaults\n";
        }
        
        // Override with command line arguments
        for (const auto& [key, value] : args) {
            config[key] = value;
        }
        
        // Create and run simulator
        ReStoreSimulator simulator;
        return simulator.run(policy, config);
    }
    
    int handleList(int argc, char* argv[]) {
        if (argc < 3) {
            std::cerr << "Usage: restore list policies\n";
            return 1;
        }
        
        std::string type = argv[2];
        
        if (type == "policies") {
            std::cout << "Available policies:\n";
            std::cout << "  RL      - Reinforcement Learning\n";
            std::cout << "  LFU     - Least Frequently Used\n";
            std::cout << "  LRU     - Least Recently Used\n";
            std::cout << "  LRFU    - Least Recently/Frequently Used\n";
            std::cout << "  EXD     - EXponential Decay\n";
            std::cout << "  TEMP    - Temperature-based\n";
            std::cout << "  static  - Static allocation\n";
            std::cout << "  ideal   - Ideal (oracle) policy\n";
            std::cout << "  LogiReg - Logistic Regression\n";
            std::cout << "  XGBfast - XGBoost (fastforest)\n";
        } else {
            std::cerr << "Unknown list type: " << type << std::endl;
            std::cerr << "Available options: policies\n";
            return 1;
        }
        
        return 0;
    }
    
    int handleConfig(int argc, char* argv[]) {
        if (argc < 4) {
            std::cerr << "Usage: restore config <get|set> <key> [value]\n";
            return 1;
        }
        
        std::string action = argv[2];
        std::string key = argv[3];
        
        ConfigManager configMgr;
        configMgr.loadConfig(config_file, config);
        
        if (action == "get") {
            auto it = config.find(key);
            if (it != config.end()) {
                std::cout << it->second << std::endl;
            } else {
                std::cout << "Key not found: " << key << std::endl;
                return 1;
            }
        } else if (action == "set") {
            if (argc < 5) {
                std::cerr << "Error: Value required for set operation\n";
                return 1;
            }
            config[key] = argv[4];
            configMgr.saveConfig(config_file, config);
            std::cout << "Set " << key << " = " << argv[4] << std::endl;
        } else {
            std::cerr << "Unknown config action: " << action << std::endl;
            return 1;
        }
        
        return 0;
    }
    
    int handleBuild(int argc, char* argv[]) {
        std::map<std::string, std::string> args = parseArgs(argc, argv);
        
        std::cout << "Building ReStore with device configuration..." << std::endl;
        
        // Load device configuration if available
        ConfigManager configMgr;
        std::map<std::string, std::string> device_config;
        if (configMgr.loadConfig("device_config.json", device_config)) {
            std::cout << "Using device configuration from device_config.json" << std::endl;
        } else {
            std::cout << "No device configuration found, using defaults" << std::endl;
            device_config["fast_device"] = "/dev/nvme0n1";
            device_config["middle_device"] = "/dev/nvme1n1";
            device_config["slow_device"] = "/dev/nvme2n1";
        }
        
        // Override with command line arguments
        if (args.find("fast_device") != args.end()) {
            device_config["fast_device"] = args["fast_device"];
        }
        if (args.find("middle_device") != args.end()) {
            device_config["middle_device"] = args["middle_device"];
        }
        if (args.find("slow_device") != args.end()) {
            device_config["slow_device"] = args["slow_device"];
        }
        
        std::cout << "Device paths:" << std::endl;
        std::cout << "  Fast: " << device_config["fast_device"] << std::endl;
        std::cout << "  Middle: " << device_config["middle_device"] << std::endl;
        std::cout << "  Slow: " << device_config["slow_device"] << std::endl;
        
        // Build command - change to source directory first
        std::ostringstream build_cmd;
        build_cmd << "cd /home/ubuntu/ReStore/ && mkdir -p build && cd build && cmake ..";
        build_cmd << " -DFAST_DEVICE_PATH=\"" << device_config["fast_device"] << "\"";
        build_cmd << " -DMIDDLE_DEVICE_PATH=\"" << device_config["middle_device"] << "\"";
        build_cmd << " -DSLOW_DEVICE_PATH=\"" << device_config["slow_device"] << "\"";
        build_cmd << " -DCMAKE_BUILD_TYPE=Release && make -j$(nproc)";
        
        std::cout << "Executing: " << build_cmd.str() << std::endl;
        
        int result = std::system(build_cmd.str().c_str());
        
        if (result == 0) {
            std::cout << "Build completed successfully!" << std::endl;
        } else {
            std::cerr << "Build failed with exit code: " << result << std::endl;
        }
        
        return result;
    }
    
    std::map<std::string, std::string> parseArgs(int argc, char* argv[]) {
        std::map<std::string, std::string> args;
        
        for (int i = 2; i < argc; i++) {
            std::string arg = argv[i];
            if (arg[0] == '-') {
                size_t eq_pos = arg.find('=');
                if (eq_pos != std::string::npos) {
                    std::string key = arg.substr(1, eq_pos - 1);
                    std::string value = arg.substr(eq_pos + 1);
                    args[key] = value;
                } else if (i + 1 < argc && argv[i + 1][0] != '-') {
                    std::string key = arg.substr(1);
                    args[key] = argv[++i];
                }
            }
        }
        
        return args;
    }
};

int main(int argc, char* argv[]) {
    ReStoreCLI cli;
    return cli.run(argc, argv);
}

