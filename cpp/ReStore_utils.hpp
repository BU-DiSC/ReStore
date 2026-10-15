// ReStore_utils.hpp - Small helpers shared by the policy executables (Run_*.cpp)
#ifndef RESTORE_UTILS_H
#define RESTORE_UTILS_H

#include <filesystem>
#include <iostream>
#include <string>
#include <system_error>

// Locate workload_<name><ext> (e.g. ".txt", ".allpageids", ".pageids") under <workdir>/workloads/.
// Files may sit directly in workloads/ or in any subfolder (e.g. workloads/synthetic/,
// workloads/traces/MSR_Cambridge/). If nothing is found, the flat path is returned so the
// caller's "cannot open" message points at the expected location.
inline std::string find_workload_file(const std::string& workdir, const std::string& workload, const std::string& ext) {
    namespace fs = std::filesystem;
    const std::string fname = "workload_" + workload + ext;
    const fs::path root = fs::path(workdir) / "workloads";
    const fs::path flat = root / fname;

    std::error_code ec;
    if (fs::exists(flat, ec)) return flat.string();
    if (fs::is_directory(root, ec)) {
        for (auto it = fs::recursive_directory_iterator(root, fs::directory_options::skip_permission_denied, ec);
             !ec && it != fs::recursive_directory_iterator(); it.increment(ec)) {
            if (it->path().filename() == fname && it->is_regular_file(ec)) return it->path().string();
        }
    }
    return flat.string();
}

// Restores std::cout's original buffer when it goes out of scope. Declare it right after
// redirecting std::cout into a log file, so that every return path (including early error
// returns) restores std::cout before the log file stream is destroyed.
class CoutRestorer {
public:
    explicit CoutRestorer(std::streambuf* original) : original_(original) {}
    ~CoutRestorer() { std::cout.rdbuf(original_); }
    CoutRestorer(const CoutRestorer&) = delete;
    CoutRestorer& operator=(const CoutRestorer&) = delete;
private:
    std::streambuf* original_;
};

#endif // RESTORE_UTILS_H
