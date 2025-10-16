#ifndef CONFIG_MANAGER_H
#define CONFIG_MANAGER_H

#include <string>
#include <map>
#include <vector>

class ConfigManager {
private:
    std::map<std::string, std::map<std::string, std::string>> workload_defaults;
    std::map<std::string, std::string> device_paths;
    
    void initializeWorkloadDefaults();
    
public:
    ConfigManager();
    
    bool loadWorkloadDefaults(const std::string& workload, std::map<std::string, std::string>& config);
    bool loadConfig(const std::string& filename, std::map<std::string, std::string>& config);
    bool saveConfig(const std::string& filename, const std::map<std::string, std::string>& config);
    
    std::vector<std::string> getAvailablePolicies() const;
    
    // Device configuration methods
    bool setDevicePaths(const std::string& fast_device, const std::string& middle_device, const std::string& slow_device);
    std::map<std::string, std::string> getDevicePaths() const;
};

#endif // CONFIG_MANAGER_H

