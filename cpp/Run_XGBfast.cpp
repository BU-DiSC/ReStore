#include <algorithm>   // std::min, std::shuffle, std::sort, std::unique, std::min_element
#include <atomic>      // std::atomic
#include <chrono>      // std::chrono
#include <cmath>       // std::abs, std::cos, std::exp, std::llround, std::log, std::round, std::sqrt
#include <cstddef>     // std::size_t
#include <cstdint>     // std::int_fast64_t
#include <cstdlib>     // std::exit, std::quick_exit
#include <ctime>       // std::localtime, std::strftime, std::time, std::time_t, std::tm
#include <deque>       // std::deque
#include <filesystem>  // std::filesystem
#include <fstream>     // std::ifstream, std::ofstream
#include <functional>  // std::function
#include <future>      // std::future
#include <iomanip>     // std::setprecision, std::setw
#include <ios>         // std::fixed
#include <iostream>    // std::cout
#include <limits>      // std::numeric_limits
#include <map>         // std::map
#include <mutex>       // std::mutex, std::scoped_lock
#include <numeric>
#include <random>      // std::mt19937_64, std::random_device, std::uniform_int_distribution
#include <sstream>     // std::ostringstream, std::istringstream
#include <stdexcept>   // std::runtime_error
#include <string>      // std::string, std::to_string
#include <string_view> // std::string_view
#include <thread>      // std::this_thread, std::thread
#include <tuple>       // std::ignore
#include <typeinfo>
#include <unordered_map>
#include <unordered_set>
#include <utility>     // std::as_const, std::forward, std::move, std::pair
#include <vector>      // std::vector

#if defined(__APPLE__)
#include <exception> // std::terminate
#endif


// Include the header files for the thread pool library and its utilities.
#include "BS_thread_pool.hpp"
//#include "BS_thread_pool_utils.hpp"

// Include ReStore driver
#include "ReStore_driver.hpp"

// Use the FastForest for quicker XGBoost
#include <fastforest.h>
// need to be compiled by: g++-12 -O3 -std=c++20 -pthread -g -o Run_XGBfast-driver Run_XGBfast-driver.cpp -I$HOME/.local/include -L$HOME/.local/lib64 -lfastforest
// also might need to add runtime linking: export LD_LIBRARY_PATH=$HOME/.local/lib64:$LD_LIBRARY_PATH


// Helper function to trim whitespace from both ends of a string
std::string trim(const std::string &str) {
    size_t start = str.find_first_not_of(" \t\n\r\f\v");
    size_t end = str.find_last_not_of(" \t\n\r\f\v");
    return (start == std::string::npos) ? "" : str.substr(start, end - start + 1);
}

// Function to move a page between tiers
void movePage(int page_id, std::unordered_set<int>& from, std::unordered_set<int>& to) {
    // Tier update
    from.erase(page_id); // O(1) average
    to.insert(page_id); // O(1) average
}

// XGB class
// Page structure to store page id and previous accesses records for XGB prediction
// (optinal) also store the recency/frequency for downgrade criteria
struct Page {
    int id;
    std::deque<int> last_five_request_rounds; // Efficient for adding and removing at both ends
    std::vector<float> XGB_features;         // Use float for compatibility
    int last_request_round;

    bool operator>(const Page& other) const {
        return last_request_round > other.last_request_round;
    }
};


class XGBCache {
public:
    std::unordered_map<int, Page> page_map;
    // std::unordered_set<int> cache;
    std::priority_queue<Page, std::vector<Page>, std::greater<>> min_heap;


    XGBCache(int capacity, const std::string& model_path) : capacity(capacity) {
        try {
            fastForest = fastforest::load_txt(model_path, features);
        } catch (const std::exception& e) {
            std::cerr << "Failed to load FastForest model: " << e.what() << std::endl;
            exit(1);
        }

        // std::cout << "FastForest model loaded successfully from " << model_path << std::endl;
    }

    void preloadPages(const std::unordered_set<int>& pages) {
        for (int page_id : pages) {
            if (page_map.size() >= capacity) {
                // If cache is full during preload, we stop adding more pages.
                break;
            }
            if (page_map.find(page_id) == page_map.end()) {
                Page new_page = {page_id, {}, {}, 0};
                page_map[page_id] = new_page;
                // cache.insert(page_id);
                min_heap.push(new_page);
            }
        }
    }


    void requestPage(int page_id, int current_round) {
        if (page_map.find(page_id) != page_map.end()) {
            // Page is already in cache, update last_requests and XGB_features

            auto& page = page_map.at(page_id);  // Use reference to avoid multiple lookups

            // Compute XGB_features before updating last_five_request_rounds
            // clear the XGB_features first
            page.XGB_features.clear();
            page.XGB_features.resize(5, 1e9f);  // Fill with 1e9 (large enough) to mimic NaN

            size_t num_rounds = page.last_five_request_rounds.size();
            size_t start_index = 5 - num_rounds; // Start inserting at the right position

            for (size_t i = 0; i < num_rounds; ++i) {
                int gap = current_round - page.last_five_request_rounds[num_rounds - 1 - i];
                page.XGB_features[start_index + i] = static_cast<float>(gap);
            }

            // Update last_five_request_rounds after computing features
            if (page.last_five_request_rounds.size() >= 5) {
                page.last_five_request_rounds.pop_front();
            }
            page.last_five_request_rounds.push_back(current_round);
            
            // Update the last_request_round
            page.last_request_round = current_round;

            // Push current page into the min_heap to update the min_heap records
            min_heap.push(page);
        } else {
            // Page is not in cache
            std::cerr << "Page is not in Tier!" << std::endl;
        }
    }

    float predict_access_future(int page_id) const {
        if (page_map.find(page_id) == page_map.end()) {
            std::cerr << "Page ID not found in cache!" << std::endl;
            return -1.0f;  // Return a sentinel value or handle error
        }

        const Page& page = page_map.at(page_id);

        float score = 1./(1. + std::exp(-fastForest(page.XGB_features.data())));
        
        return score;
    }

    void addPage(Page new_page) {
        // Add new page to cache
        int page_id = new_page.id;
        page_map[page_id] = new_page;
        // cache.insert(page_id);
        min_heap.push(new_page);
    }

    void evictPage(Page ev_page) {
        // Cache is full, evict the least frequently used page
        // min_heap.pop();
        page_map.erase(ev_page.id);
        // cache.erase(ev_page.id);
    }

    // const std::unordered_set<int>& getCache() const {
    //     return cache;
    // }

private:
    void updateHeap(int page_id) {
        // This is a simplified example. In a real implementation, you may need to
        // remove the page from the heap and re-insert it to maintain heap properties.
        // Here, we simulate this by pushing a new copy and relying on lazy deletion.
        min_heap.push(page_map[page_id]);
    }

    int capacity;
    fastforest::FastForest fastForest;
    std::vector<std::string> features{"f0",  "f1",  "f2",  "f3",  "f4"};
};




int main(
    int argc, char *argv[]
    ){
    // set environmental arguments here
    // read/write time of each tier
    int read_time_tier1 = 30;   float asym_tier1 = 1.5;
    int read_time_tier2 = 200;  float asym_tier2 = 2.0;
    int read_time_tier3 = 500;  float asym_tier3 = 4.0;
    // capacity of each tier
    int max_capacity_tier1 = 100;
    int max_capacity_tier2 = 400;
    int max_capacity_tier3 = 2e4; //large enough for all pages
    // Concurrency (available number of threads)
    int num_threads_tier1 = 8;
    int num_threads_tier2 = 4;
    int num_threads_tier3 = 2;
    // concurrency threshold (k_read/write)
    int k_thrd_tier1 = 9;
    int k_thrd_tier2 = 5;
    int k_thrd_tier3 = 3;
    // total number of pages
    int total_num_pages = 1e4;
    // total number of requests
    int total_num_reqs = 1e6;
    // use real workload
    std::string workload = "10hf80_1e4_rw1_1e6";
    // choose XGBoost model (which percentage of pages used for training)
    int train_percentage = 5;

    // Command line argument parsing
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg.find("-workload=") == 0) {
            workload = arg.substr(10); // Extracts the substring after "-workload=" and assigns it to workload
        } else if (arg.find("-total_num_pages=") == 0) {
            total_num_pages = static_cast<int>(std::stod(arg.substr(17))); // Extracts and converts the substring after "-total_num_pages=" to an integer
        } else if (arg.find("-total_num_reqs=") == 0) {
            total_num_reqs = static_cast<int>(std::stod(arg.substr(16))); // Extracts and converts the substring after "-total_num_reqs=" to an integer
        } else if (arg.find("-max_capacity_tier1=") == 0) {
            max_capacity_tier1 = static_cast<int>(std::stod(arg.substr(20))); // Extracts and converts the substring after "-max_capacity_tier1=" to an integer
        } else if (arg.find("-max_capacity_tier2=") == 0) {
            max_capacity_tier2 = static_cast<int>(std::stod(arg.substr(20))); // Extracts and converts the substring after "-max_capacity_tier2=" to an integer
        } else if (arg.find("-max_capacity_tier3=") == 0) {
            max_capacity_tier3 = static_cast<int>(std::stod(arg.substr(20))); // Extracts and converts the substring after "-max_capacity_tier3=" to an integer
        } else if (arg.find("-read_time_tier1=") == 0) {
            read_time_tier1 = static_cast<int>(std::stod(arg.substr(17))); // Extracts and converts the substring after "-read_time_tier1=" to an integer
        } else if (arg.find("-read_time_tier2=") == 0) {
            read_time_tier2 = static_cast<int>(std::stod(arg.substr(17))); // Extracts and converts the substring after "-read_time_tier2=" to an integer
        } else if (arg.find("-read_time_tier3=") == 0) {
            read_time_tier3 = static_cast<int>(std::stod(arg.substr(17))); // Extracts and converts the substring after "-read_time_tier3=" to an integer
        } else if (arg.find("-asym_tier1=") == 0) {
            asym_tier1 = std::stod(arg.substr(12)); // Extracts and converts the substring after "-asym_tier1=" to an double
        } else if (arg.find("-asym_tier2=") == 0) {
            asym_tier2 = std::stod(arg.substr(12)); // Extracts and converts the substring after "-asym_tier2=" to an double
        } else if (arg.find("-asym_tier3=") == 0) {
            asym_tier3 = std::stod(arg.substr(12)); // Extracts and converts the substring after "-asym_tier3=" to an double
        } else if (arg.find("-num_threads_tier1=") == 0) {
            num_threads_tier1 = static_cast<int>(std::stod(arg.substr(19))); // Extracts and converts the substring after "-num_threads_tier1=" to an integer
        } else if (arg.find("-num_threads_tier2=") == 0) {
            num_threads_tier2 = static_cast<int>(std::stod(arg.substr(19))); // Extracts and converts the substring after "-num_threads_tier2=" to an integer
        } else if (arg.find("-num_threads_tier3=") == 0) {
            num_threads_tier3 = static_cast<int>(std::stod(arg.substr(19))); // Extracts and converts the substring after "-num_threads_tier3=" to an integer
        }
        else if (arg.find("-train_percentage=") == 0) {
            train_percentage = static_cast<int>(std::stod(arg.substr(18))); // Extracts and converts the substring after "-train_percentage=" to an integer
        }
    }

    // workload path
    std::string workload_path = std::string("workload_") + workload + ".txt";

    // Check if the folder for results exists
    std::string folder_name = std::string("Results_") + workload + "/capacity_tests_" 
                              + std::to_string(read_time_tier1) + "-" + std::to_string(read_time_tier2) + "-" + std::to_string(read_time_tier3)
                              + "/" + std::to_string(max_capacity_tier1) + "-" + std::to_string(max_capacity_tier2);
    if (!std::filesystem::exists(folder_name)) {
        // If the folder does not exist, create it
        if (std::filesystem::create_directory(folder_name)) {
            std::cout << "Folder created successfully: " << folder_name << std::endl;
        } else {
            std::cerr << "Failed to create folder: " << folder_name << std::endl;
        }
    } else {
        std::cout << "Folder already exists: " << folder_name << std::endl;
    }

    // Open the log file for writing
    std::string log_path = folder_name + "/output_" + workload + "_XGBfast-driver_" + std::to_string(train_percentage) + "%.log";
    std::ofstream logFile(log_path);

    auto cout_buff = std::cout.rdbuf(); 
    std::cout.rdbuf(logFile.rdbuf());

    // // Open txt file for tiers' content in each step
    // std::string outtier_path = folder_name + "Tier123_" + workload_path + "_XGB.txt";
    // std::ofstream outTier(outtier_path);


    // Define driver of Tiers
    Tier tier1_dr(max_capacity_tier1, num_threads_tier1, read_time_tier1, asym_tier1);
    Tier tier2_dr(max_capacity_tier2, num_threads_tier2, read_time_tier2, asym_tier2);
    Tier tier3_dr(max_capacity_tier3, num_threads_tier3, read_time_tier3, asym_tier3);


    // XGB class for Tier1, Tier2, Tier3
    std::string xgb_model_path = "ML_models/XGBoost_models/xgboost_model_" + workload + "_" + std::to_string(train_percentage) + "%.txt";
    XGBCache LC_T1(max_capacity_tier1, xgb_model_path);
    XGBCache LC_T2(max_capacity_tier2, xgb_model_path);
    XGBCache LC_T3(max_capacity_tier3, xgb_model_path);
    // create reference of page_map
    auto& Tier1 = LC_T1.page_map;
    auto& Tier2 = LC_T2.page_map;
    auto& Tier3 = LC_T3.page_map;


    // Initiation rule can be changed here.
    if (workload == "YCSB" || workload == "TPCC" || workload == "TPCE" || workload.find("MSR") != std::string::npos) {
        // initial policy for workload_YCSB: fill with specific page ids
        std::string page_ids_file = std::string("workload_") + workload + ".allpageids";  // Order file to read from
        std::ifstream IDs(page_ids_file);
        std::vector<int> allKeys;
        if (!IDs.is_open()) {
            std::cerr << "Error opening file: " << page_ids_file << std::endl;
            return 1;
        }
        // Read only the first line from .pageids
        std::string pageids;
        if (std::getline(IDs, pageids)) {
            std::stringstream ss(pageids);
            std::string token;
            while (std::getline(ss, token, ',')) {
                try {
                    token = trim(token); // Trim any leading/trailing whitespace
                    if (!token.empty()) {
                        allKeys.push_back(std::stoi(token));
                    }
                } catch (const std::invalid_argument& e) {
                    std::cerr << "Invalid argument: " << token << " is not a valid integer." << std::endl;
                } catch (const std::out_of_range& e) {
                    std::cerr << "Out of range: " << token << " is out of integer range." << std::endl;
                }
            }
        }
        IDs.close();
        //double check if num of pageids are the same
        if (allKeys.size() != total_num_pages){
            std::cerr << "allKeys length doesn't match total_num_pages !" << std::endl;
            return 1;
        }
        // Insert first max_capacity_tier1 ids into Tier1
        for (size_t i = 0; i < max_capacity_tier1 && i < allKeys.size(); ++i) {
            int key = allKeys[i];
            LC_T1.addPage(Page{key, {}, {}, 0});
        }
        // Insert next max_capacity_tier2 ids into Tier2
        for (size_t i = max_capacity_tier1; i < max_capacity_tier1 + max_capacity_tier2 && i < allKeys.size(); ++i) {
            int key = allKeys[i];
            LC_T2.addPage(Page{key, {}, {}, 0});
        }
        // Insert remaining ids into Tier3
        for (size_t i = max_capacity_tier1 + max_capacity_tier2; i < allKeys.size(); ++i) {
            int key = allKeys[i];
            LC_T3.addPage(Page{key, {}, {}, 0});
            // clean LC_T3.min_heap to free space
            LC_T3.min_heap.pop();
        }
        // // Insert 1 ids into Tier1 (to avoid empty error)
        // for (size_t i = 0; i < 1 && i < allKeys.size(); ++i) {
        //     int key = allKeys[i];
        //     LC_T1.addPage(Page{key, {}, {}, 0});
        // }
        // // Insert 1 ids into Tier2 (to avoid empty error)
        // for (size_t i = 1; i < 2 && i < allKeys.size(); ++i) {
        //     int key = allKeys[i];
        //     LC_T2.addPage(Page{key, {}, {}, 0});
        // }
        // // Insert all ids into Tier3
        // for (size_t i = 2; i < allKeys.size(); ++i) {
        //     int key = allKeys[i];
        //     LC_T3.addPage(Page{key, {}, {}, 0});
        //     // clean LC_T3.min_heap to free space
        //     LC_T3.min_heap.pop();
        // }
    }
    else {
        // initial policy: fill each tier first
        for (int key = 0; key < max_capacity_tier1; ++key) {
            LC_T1.addPage(Page{key, {}, {}, 0});
        }
        for (int key = max_capacity_tier1; key < max_capacity_tier1 + max_capacity_tier2; ++key) {
            LC_T2.addPage(Page{key, {}, {}, 0});
        }
        for (int key = max_capacity_tier1 + max_capacity_tier2; key < total_num_pages; ++key) {
            LC_T3.addPage(Page{key, {}, {}, 0});
            // clean LC_T3.min_heap to free space
            LC_T3.min_heap.pop();
        }
    }
    



    // Counter of Read/Write requests
    int num_reads = 0;
    int num_write = 0;

    // Counter of page hit
    int page_hit_Tier1 = 0;
    int page_hit_Tier2 = 0;
    int page_hit_Tier3 = 0;

    // Counter of page migration
    int num_migr_t2t1 = 0;
    int num_migr_t3t2 = 0;

    // Counter of runtime
    long long total_runtime_XGB = 0; // Variable to accumulate durations


    // Open the workload file
    std::ifstream file(workload_path);
    if (!file) {
        std::cerr << "Failed to open the file: " << workload_path << std::endl;
        return 1; // Exit if the file cannot be opened
    }

    // Start the timer
    auto start_time = std::chrono::high_resolution_clock::now();
    // Begin loop of requests
    int i = 0;  //round counter
    std::string line;
    while (std::getline(file, line)) {
        // Increment i
        ++i;
        // Check if we've processed the desired number of requests
        if (i > total_num_reqs) {
            break;  // Exit the while loop if reached total_num_reqs limit
        }

        auto start_time_i = std::chrono::high_resolution_clock::now();

        // load PageNum and action from lines
        std::istringstream iss(line);
        char act;
        int n_page;
        std::string action;
        if (iss >> act >> n_page) {
            if (act == 'R'){
                num_reads++;
                action = "Read";

            }
            else if (act == 'W'){
                num_write++;
                action = "Write";
            }
            else {
                std::cerr << "Invalid action: " << act << std::endl;
            }
        } 
        else {
            std::cerr << "Invalid line: " << line << std::endl;
        }

        // Locate current Tier the page is in
        auto it1 = Tier1.find(n_page);
        auto it2 = Tier2.find(n_page);
        auto it3 = Tier3.find(n_page);

        if (it1 != Tier1.end()) {
            // std::cout << "page " << n_page << " currently in Tier1\n" << std::endl;
            ++page_hit_Tier1;
            
            // Execute action
            tier1_dr.exec(n_page, action);

            // Update page's Metadata

            LC_T1.requestPage(n_page, i);

            // no need for migration if page already in Tier1
        }

        else if (it2 != Tier2.end()) {
            // std::cout << "page " << n_page << " currently in Tier2\n" << std::endl;
            ++page_hit_Tier2;

            // Execute action
            tier2_dr.exec(n_page, action);

            // Update page's Metadata
            LC_T2.requestPage(n_page, i);

            // Decide migration by XGB
            // count time
            auto start_time_XGB = std::chrono::high_resolution_clock::now();
            // Calculate the prediction on future access
            float pred_XGB = LC_T2.predict_access_future(n_page);
            // If XGB prediction is 1 (>0.5), then upgrade current page to Tier1
            if (pred_XGB > 0.5){
                ++num_migr_t2t1;
                // std::cout << "Page " << n_page << " should be moved from Tier2 to Tier1\n"<< std::endl;
                Page current_page = LC_T2.page_map[n_page];
                LC_T1.addPage(current_page);
                LC_T2.evictPage(current_page);
                // if Tier 1 is full
                while (Tier1.size() > max_capacity_tier1){
                    // Downgrade LRU page to Tier2
                    Page lru_page_t1  = LC_T1.min_heap.top();
                    // skip non-existed page or non-matched weight page
                    while ((Tier1.find(lru_page_t1.id) == Tier1.end()) ||
                        (lru_page_t1.last_request_round != LC_T1.page_map.at(lru_page_t1.id).last_request_round)) {
                        LC_T1.min_heap.pop();
                        lru_page_t1  = LC_T1.min_heap.top();
                    }
                    LC_T2.addPage(lru_page_t1);
                    LC_T1.evictPage(lru_page_t1);
                    // new task should be submitted here to simulate page downgrade
                    // read from Tier1
                    tier1_dr.exec(lru_page_t1.id, "Read");
                    // write to Tier2
                    tier2_dr.exec(lru_page_t1.id, "Write");
                }
                // new task for the migration
                // read from Tier2
                tier2_dr.exec(current_page.id, "Read");
                // write to Tier1
                tier1_dr.exec(current_page.id, "Write");
            // check whether Tiers = LC_Ti.cache
            }
            else{
                // std::cout << "Page " << n_page << " should remain in Tier2.\n"<< std::endl;
            }
            auto end_time_XGB = std::chrono::high_resolution_clock::now();
            // Calculate the duration
            auto duration_XGB = std::chrono::duration_cast<std::chrono::nanoseconds>(end_time_XGB - start_time_XGB);
            // std::cout << "XGBCache.predict takes " << duration_XGB.count() << " nanoseconds" << std::endl;
            total_runtime_XGB += duration_XGB.count(); // Accumulate duration in nanoseconds
        }

        else if (it3 != Tier3.end()) {
            // std::cout << "page " << n_page << " currently in Tier3\n" << std::endl;
            ++page_hit_Tier3;

            // Execute action
            tier3_dr.exec(n_page, action);

            // Update page's Metadata
            LC_T3.requestPage(n_page, i);
            // clean LC_T3.min_heap to free space
            LC_T3.min_heap.pop();
            
            // Decide migration by XGB
            // count time
            auto start_time_XGB = std::chrono::high_resolution_clock::now();
            // Calculate the prediction on future access
            float pred_XGB = LC_T3.predict_access_future(n_page);
            // If XGB prediction is 1 (>0.5), then upgrade current page to Tier2
            if (pred_XGB > 0.5){
                ++num_migr_t3t2;
                // std::cout << "Page " << n_page << " should be moved from Tier3 to Tier2\n"<< std::endl;
                Page current_page = LC_T3.page_map[n_page];
                LC_T2.addPage(current_page);
                LC_T3.evictPage(current_page);
                // if Tier 2 is full
                while (Tier2.size() > max_capacity_tier2){
                    // Downgrade LRU page to Tier3
                    Page lru_page_t2  = LC_T2.min_heap.top();
                    // skip non-existed page or non-matched weight page
                    while ((Tier2.find(lru_page_t2.id) == Tier2.end()) ||
                        (lru_page_t2.last_request_round != LC_T2.page_map.at(lru_page_t2.id).last_request_round)) {
                        LC_T2.min_heap.pop();
                        lru_page_t2  = LC_T2.min_heap.top();
                    }
                    LC_T3.addPage(lru_page_t2);
                    LC_T2.evictPage(lru_page_t2);
                    // new task should be submitted here to simulate page downgrade
                    // read from Tier2
                    tier2_dr.exec(lru_page_t2.id, "Read");
                    // write to Tier3
                    tier3_dr.exec(lru_page_t2.id, "Write");
                }
                // new task for the migration
                // read from Tier3
                tier3_dr.exec(current_page.id, "Read");
                // write to Tier2
                tier2_dr.exec(current_page.id, "Write");
            // check whether Tiers = LC_Ti.cache
            }
            else{
                // std::cout << "Page " << n_page << " should remain in Tier2.\n"<< std::endl;
            }
            auto end_time_XGB = std::chrono::high_resolution_clock::now();
            // Calculate the duration
            auto duration_XGB = std::chrono::duration_cast<std::chrono::nanoseconds>(end_time_XGB - start_time_XGB);
            // std::cout << "XGBCache.predict takes " << duration_XGB.count() << " nanoseconds" << std::endl;
            total_runtime_XGB += duration_XGB.count(); // Accumulate duration in nanoseconds
        }

        else {
            std::cout << "page not exist in Tiers!\n" << std::endl;
        }


        //std::cout << "Running on pool " << rv << ", results:" << my_future.get() << '\n';
        // std::cout << "Finish request " << i+1 << "\n" << std::endl;

        // outTier << "Page distribution after request " << i+1 << std::endl;
        // // Write Tier1 to the file
        // outTier << "Tier1:" << std::endl;
        // for (const auto& entry : Tier1) {
        //     outTier << entry.first << " " << entry.second << ", ";
        // }
        // outTier << std::endl;
        // // Write Tier2 to the file
        // outTier << "Tier2:" << std::endl;
        // for (const auto& entry : Tier2) {
        //     outTier << entry.first << " " << entry.second << ", ";
        // }
        // outTier << std::endl;
        // // Write Tier3 to the file
        // outTier << "Tier3:" << std::endl;
        // for (const auto& entry : Tier3) {
        //     outTier << entry.first << " " << entry.second << ", ";
        // }
        // outTier << std::endl;
        
    }
    auto end_time_for = std::chrono::high_resolution_clock::now();

    while (tier1_dr.pool.get_tasks_running() > 0 || 
           tier2_dr.pool.get_tasks_running() > 0 || 
           tier3_dr.pool.get_tasks_running() > 0 || 
           tier1_dr.pool.get_tasks_queued() > 0  || 
           tier2_dr.pool.get_tasks_queued() > 0  || 
           tier3_dr.pool.get_tasks_queued() > 0) {
    // Check if tasks are still running in pools and if there are queued tasks
    // If tasks are still running/queued, continue to wait
    // If not, proceed to end the timer
    }

    // End the timer
    auto end_time = std::chrono::high_resolution_clock::now();

    // Calculate the duration
    auto duration_for = std::chrono::duration_cast<std::chrono::microseconds>(end_time_for - start_time);
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);

    // Output the duration
    std::cout << "For loop time: " << duration_for.count() << " microseconds" << std::endl;
    std::cout << "Total XGB runtime: " << total_runtime_XGB << " nanoseconds" << std::endl;
    std::cout << "Total requests time: " << duration.count() << " microseconds\n" << std::endl;

    // counter of read/write
    std::cout << "There are " << num_reads << " reads, and " << num_write << " writes.\n" << std::endl;

    // page hits
    std::cout << "Number of page hits in Tier1: " << page_hit_Tier1 << std::endl;
    std::cout << "Number of page hits in Tier2: " << page_hit_Tier2 << std::endl;
    std::cout << "Number of page hits in Tier3: " << page_hit_Tier3 << std::endl;

    // number of migrations
    std::cout << "\nNumber of page migrated from Tier2 to Tier1: " << num_migr_t2t1 << std::endl;
    std::cout << "Number of page migrated from Tier3 to Tier2: " << num_migr_t3t2 << std::endl;

    std::cout << "\nAverage runtime per call: " << total_runtime_XGB/(page_hit_Tier2+page_hit_Tier3) << " nanoseconds" << std::endl;
    std::cout << "Average runtime per migration: " << total_runtime_XGB/(num_migr_t2t1+num_migr_t3t2) << " nanoseconds" << std::endl;
    

    std::cout.rdbuf(cout_buff); 
    // Close the log file and tier file
    logFile.close();
    file.close();
    // outTier.close();

}
