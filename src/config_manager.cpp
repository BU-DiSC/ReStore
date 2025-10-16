#include "config_manager.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <filesystem>
#include <unistd.h>

ConfigManager::ConfigManager() {
    // Initialize default configurations for different workloads
    initializeWorkloadDefaults();
}

void ConfigManager::initializeWorkloadDefaults() {
    // MSR_hm_1 defaults
    workload_defaults["MSR_hm_1"] = {
        {"max_capacity_tier1", "5000"},
        {"max_capacity_tier2", "15000"},
        {"max_capacity_tier3", "60000"},
        {"read_time_tier1", "30"},
        {"read_time_tier2", "200"},
        {"read_time_tier3", "500"},
        {"total_num_pages", "51733"},
        {"total_num_reqs", "2308560"},
        {"temp_incr_alpha", "0.1"},
        {"temp_incr_buffersize", "50000"},
        {"temp_drop_thrd", "10000"},
        {"temp_drop_freqs", "20000"},
        {"RL_update_freqs", "2000"},
        {"RL_init_rounds", "200"},
        {"a_b_update_freq_s1", "5"},
        {"num_elements_to_consider_s1", "500"},
        {"a_b_update_freq_s2", "5"},
        {"num_elements_to_consider_s2", "1000"}
    };
    
    // MSR_wdev_0 defaults
    workload_defaults["MSR_wdev_0"] = {
        {"max_capacity_tier1", "5000"},
        {"max_capacity_tier2", "15000"},
        {"max_capacity_tier3", "60000"},
        {"read_time_tier1", "30"},
        {"read_time_tier2", "200"},
        {"read_time_tier3", "500"},
        {"total_num_pages", "79677"},
        {"total_num_reqs", "695995"},
        {"temp_incr_alpha", "0.1"},
        {"temp_incr_buffersize", "50000"},
        {"temp_drop_thrd", "10000"},
        {"temp_drop_freqs", "20000"},
        {"RL_update_freqs", "2000"},
        {"RL_init_rounds", "200"},
        {"a_b_update_freq_s1", "5"},
        {"num_elements_to_consider_s1", "500"},
        {"a_b_update_freq_s2", "5"},
        {"num_elements_to_consider_s2", "1000"}
    };
    
    // MSR_mds_0 defaults
    workload_defaults["MSR_mds_0"] = {
        {"max_capacity_tier1", "80000"},
        {"max_capacity_tier2", "240000"},
        {"max_capacity_tier3", "900000"},
        {"read_time_tier1", "36"},
        {"read_time_tier2", "206"},
        {"read_time_tier3", "520"},
        {"total_num_pages", "802284"},
        {"total_num_reqs", "2703897"},
        {"temp_incr_alpha", "0.05"},
        {"temp_incr_buffersize", "800000"},
        {"temp_drop_thrd", "80000"},
        {"temp_drop_freqs", "100000"},
        {"RL_update_freqs", "500"},
        {"RL_init_rounds", "100"},
        {"a_b_update_freq_s1", "5"},
        {"num_elements_to_consider_s1", "1000"},
        {"a_b_update_freq_s2", "5"},
        {"num_elements_to_consider_s2", "1000"}
    };
}

bool ConfigManager::loadWorkloadDefaults(const std::string& workload, std::map<std::string, std::string>& config) {
    auto it = workload_defaults.find(workload);
    if (it == workload_defaults.end()) {
        std::cerr << "Warning: No default configuration found for workload: " << workload << std::endl;
        return false;
    }
    
    config = it->second;
    config["workload"] = workload;
    return true;
}

bool ConfigManager::loadConfig(const std::string& filename, std::map<std::string, std::string>& config) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        return false;
    }
    
    std::string line;
    while (std::getline(file, line)) {
        // Simple key=value format
        size_t eq_pos = line.find('=');
        if (eq_pos != std::string::npos) {
            std::string key = line.substr(0, eq_pos);
            std::string value = line.substr(eq_pos + 1);
            config[key] = value;
        }
    }
    
    file.close();
    return true;
}

bool ConfigManager::saveConfig(const std::string& filename, const std::map<std::string, std::string>& config) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        return false;
    }
    
    for (const auto& [key, value] : config) {
        file << key << "=" << value << std::endl;
    }
    
    file.close();
    return true;
}


std::vector<std::string> ConfigManager::getAvailablePolicies() const {
    return {"RL", "LFU", "LRU", "LRFU", "EXD", "TEMP", "static", "ideal", "LogiReg", "XGBfast"};
}

bool ConfigManager::setDevicePaths(const std::string& fast_device, const std::string& middle_device, const std::string& slow_device) {
    // Validate device paths exist
    if (access(fast_device.c_str(), F_OK) != 0) {
        std::cerr << "Warning: Fast device path does not exist: " << fast_device << std::endl;
    }
    if (access(middle_device.c_str(), F_OK) != 0) {
        std::cerr << "Warning: Middle device path does not exist: " << middle_device << std::endl;
    }
    if (access(slow_device.c_str(), F_OK) != 0) {
        std::cerr << "Warning: Slow device path does not exist: " << slow_device << std::endl;
    }
    
    // Store device paths in a special configuration section
    device_paths["fast_device"] = fast_device;
    device_paths["middle_device"] = middle_device;
    device_paths["slow_device"] = slow_device;
    
    return true;
}

std::map<std::string, std::string> ConfigManager::getDevicePaths() const {
    return device_paths;
}

