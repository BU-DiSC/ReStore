#include <algorithm>   // std::min, std::shuffle, std::sort, std::unique, std::min_element
#include <atomic>      // std::atomic
#include <chrono>      // std::chrono
#include <cmath>       // std::abs, std::cos, std::exp, std::llround, std::log, std::round, std::sqrt
#include <cstddef>     // std::size_t
#include <cstdint>     // std::int_fast64_t
#include <cstdlib>     // std::exit, std::quick_exit
#include <ctime>       // std::localtime, std::strftime, std::time, std::time_t, std::tm
#include <deque>       // std::deque
#include <execution>
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
#include <utility>     // std::as_const, std::forward, std::move, std::pair
#include <vector>      // std::vector

#if defined(__APPLE__)
#include <exception> // std::terminate
#endif

// Include the header files for the thread pool library and its utilities.
#include "BS_thread_pool.hpp"
//#include "BS_thread_pool_utils.hpp"


// Read/Write from/to Tier
void R_n_W(int pageNum, std::string action, int tier_num,
          int read_time_tier1, float asym_tier1,
          int read_time_tier2, float asym_tier2,
          int read_time_tier3, float asym_tier3
          ){
    // R/W in Tier1
    if (tier_num == 1){
        if (action == "Read"){
            // std::cout << "reading page " << pageNum << " from Tier 1 ...\n";
            // Sleep for 0.01 seconds to simulate the read action
            std::this_thread::sleep_for(std::chrono::microseconds(read_time_tier1));
            // std::cout << "Read Done." << std::endl;
        }
        else if (action == "Write"){
            // std::cout << "writing page " << pageNum << " to Tier 1 ...\n";
            // Sleep for 0.03 seconds to simulate the write action
            // write_time_tier1 = read_time_tier1*asym_tier1
            std::this_thread::sleep_for(std::chrono::microseconds(static_cast<int>(read_time_tier1*asym_tier1)));
            // std::cout << "Write Done." << std::endl;
        }
        else {
            std::cerr << "Unknown action !" << std::endl;
        }
    }
    // R/W in Tier2
    else if (tier_num == 2){
        if (action == "Read"){
            // std::cout << "reading page " << pageNum << " from Tier 2 ...\n";
            // Sleep for 0.05 seconds to simulate the read action
            std::this_thread::sleep_for(std::chrono::microseconds(read_time_tier2));
            // std::cout << "Read Done." << std::endl;
        }
        else if (action == "Write"){
            // std::cout << "writing page " << pageNum << " to Tier 2 ...\n";
            // Sleep for 0.1 seconds to simulate the write action
            // write_time_tier2 = read_time_tier2*asym_tier2
            std::this_thread::sleep_for(std::chrono::microseconds(static_cast<int>(read_time_tier2*asym_tier2)));
            // std::cout << "Write Done." << std::endl;
        }
        else {
            std::cerr << "Unknown action !" << std::endl;
        }
    }
    // R/W in Tier3
    else if (tier_num == 3){
        if (action == "Read"){
            // std::cout << "reading page " << pageNum << " from Tier 3 ...\n";
            // Sleep for 0.2 seconds to simulate the read action
            std::this_thread::sleep_for(std::chrono::microseconds(read_time_tier3));
            // std::cout << "Read Done." << std::endl;
        }
        else if (action == "Write"){
            // std::cout << "writing page " << pageNum << " to Tier 3 ...\n";
            // Sleep for 0.3 seconds to simulate the write action
            // write_time_tier3 = read_time_tier3*asym_tier3
            std::this_thread::sleep_for(std::chrono::microseconds(static_cast<int>(read_time_tier3*asym_tier3)));
            // std::cout << "Write Done." << std::endl;
        }
        else {
            std::cerr << "Unknown action ! \n" << std::endl;
        }    
    }
}

// Helper function to trim whitespace from both ends of a string
std::string trim(const std::string &str) {
    size_t start = str.find_first_not_of(" \t\n\r\f\v");
    size_t end = str.find_last_not_of(" \t\n\r\f\v");
    return (start == std::string::npos) ? "" : str.substr(start, end - start + 1);
}


// Page movement between tiers
void PageMigrate(
    int keyToMove,
    std::unordered_map<int, double>& from_map,
    std::unordered_map<int, double>& to_map
    ) {
    // check if the page is in from_map
    auto it = from_map.find(keyToMove);
    if (it == from_map.end()) {
        throw std::runtime_error("Page not in from_map!");
    }
    to_map.insert(*it);
    from_map.erase(it);
    // std::cout << "Page " << keyToMove << " migrated.";
}


// shorter version of PageMigrate()
/*
{
    auto& source = (map1.count(keyToMove) > 0) ? map1 : map2;
    auto& destination = (map1.count(keyToMove) > 0) ? map2 : map1;

    auto it = source.find(keyToMove);
    
    if (it != source.end()) {
        destination.insert(*it);
        source.erase(it);
        std::cout << "Page " << keyToMove << " moved.\n";
    } else {
        std::cout << "Page " << keyToMove << " not found in either map.\n";
    }
    
    return {map1, map2};
}
*/

// Temperature structure for min_heap_temp
struct Page {
    int id;
    double temperature;
    std::deque<int> req_rounds;  // record the rounds of being requested, within certain time window(temp_incr_buffersize)
    int last_request_round;   // last round being requested, use for temperature drop
    int last_tempdrop_round;  // last round temperature drop is executed

    bool operator>(const Page& other) const {
        return temperature > other.temperature;
    }
};

class TEMPCache {
public:
    std::unordered_map<int, Page> page_map;
    // std::unordered_set<int> cache;
    std::priority_queue<Page, std::vector<Page>, std::greater<>> min_heap;

    TEMPCache(int capacity,
              double temp_incr_alpha,
              int temp_incr_buffersize) : capacity(capacity),
                                          alpha(temp_incr_alpha),
                                          time_window(temp_incr_buffersize) {}

    // insert one by one instead of preload
    // void preloadPages(const std::unordered_set<int>& pages) {
    //     for (int page_id : pages) {
    //         if (page_map.size() >= capacity) {
    //             // If cache is full during preload, we stop adding more pages.
    //             break;
    //         }
    //         if (page_map.find(page_id) == page_map.end()) {
    //             Page new_page = {page_id, 0, 0};
    //             page_map[page_id] = new_page;
    //             // cache.insert(page_id);
    //             min_heap.push(new_page);
    //         }
    //     }
    // }

    void requestPage(int page_id, int current_round) {
        if (page_map.find(page_id) != page_map.end()) {
            auto& page = page_map[page_id];  // Store the reference to avoid multiple lookups
            // Page is already in cache, update temperature and recency
            page.req_rounds.push_back(current_round);
            // update the req_round deque so that it only contain rounds within time window
            while (page.req_rounds.front() < current_round - time_window){
                page.req_rounds.pop_front();
            }
            // calculate the new temperature of the page
            int req_time = page.req_rounds.size();
            double new_temp = 1 - 0.5 / exp(alpha * req_time);
            // update page temperature
            page.temperature = new_temp;
            // and its request record
            page.last_request_round = current_round;
            // push the new Page into heap
            min_heap.push(page);
        } else {
            // Page is not in cache
            std::cerr << "Page is not in Tier!" << std::endl;
        }
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
    double alpha;
    int time_window;
};


// Temperature increase (integrated in TEMPCache class)
double temp_increase(int PageKey, const std::unordered_map<int, int>& list_num_req, double temp_incr_alpha){

    int req_time = list_num_req.find(PageKey)->second;   // get historical number of request of this page
    double temp = 1 - 0.5 / exp(temp_incr_alpha * req_time);        // calculate the new temperature  // alpha=0.05

    return temp;
}

// Temperature drop (version for drop at each round)
// also return the avg_temp of each Tier
std::tuple<double, double, double> 
decrease_temperature(std::unordered_map<int, double>& Tier1,
                     std::unordered_map<int, double>& Tier2,
                     std::unordered_map<int, double>& Tier3,
                     std::unordered_map<int, int>& list_last_req,
                     int current_round,
                     int temp_drop_thrd,
                     double temp_drop_scale) {
    
    double sum_temp_T1 = 0.0;
    for (auto& keyValue : Tier1) {
        //idle time
        int idle_time = current_round - list_last_req[keyValue.first];
        // if it has been temp_drop_thrd rounds after last request of a page, drop it's temperature
        if ( idle_time > 0 && idle_time % temp_drop_thrd == 0 ) {
            // lowest temperature is 0.0
            keyValue.second = std::max(keyValue.second - temp_drop_scale, 0.0);
        }
        sum_temp_T1 += keyValue.second;
    }

    double sum_temp_T2 = 0.0;
    for (auto& keyValue : Tier2) {
        //idle time
        int idle_time = current_round - list_last_req[keyValue.first];
        // if it has been temp_drop_thrd rounds after last request of a page, drop it's temperature
        if ( idle_time > 0 && idle_time % temp_drop_thrd == 0 ) {
            // lowest temperature is 0.0
            keyValue.second = std::max(keyValue.second - temp_drop_scale, 0.0);
        }
        sum_temp_T2 += keyValue.second;
    }

    double sum_temp_T3 = 0.0;
    for (auto& keyValue : Tier3) {
        //idle time
        int idle_time = current_round - list_last_req[keyValue.first];
        // if it has been temp_drop_thrd rounds after last request of a page, drop it's temperature
        if ( idle_time > 0 && idle_time % temp_drop_thrd == 0 ) {
            // lowest temperature is 0.0
            keyValue.second = std::max(keyValue.second - temp_drop_scale, 0.0);
        }
        sum_temp_T3 += keyValue.second;
    }

    return std::make_tuple(sum_temp_T1/Tier1.size(), sum_temp_T2/Tier2.size(), sum_temp_T3/Tier3.size());
}

// Temperature drop (lazy evaluation: check every 500 rounds)
// also return the avg_temp of each Tier
std::tuple<double, double, double> 
decrease_temperature_lz(TEMPCache& LC_T1,
                        TEMPCache& LC_T2,
                        TEMPCache& LC_T3,
                        int current_round,
                        int temp_drop_thrd,
                        double temp_drop_scale) {
    
    double sum_temp_T1 = 0.0;
    std::priority_queue<Page, std::vector<Page>, std::greater<>> new_min_heap_T1;
    for (auto& elem : LC_T1.page_map) {
        int page_id = elem.first;
        auto& page = elem.second;
        int last_req_round = page.last_request_round;
        int& last_drop_round = page.last_tempdrop_round;
        // Idle time since the last request
        int idle_time = current_round - last_req_round;
        // If it has been temp_drop_thrd rounds after the last request of a page, drop its temperature
        if (idle_time > temp_drop_thrd) {
            // Calculate the correct drop ratio
            int rounds_since_last_drop = current_round - last_drop_round;
            int drop_ratio = rounds_since_last_drop / temp_drop_thrd;
            // Lowest temperature is 0.0
            page.temperature = std::max(page.temperature - temp_drop_scale * drop_ratio, 0.0);
            // Update last drop round
            last_drop_round = current_round - (rounds_since_last_drop % temp_drop_thrd);
        }
        // push all pages into the new heap
        new_min_heap_T1.push(elem.second);
        sum_temp_T1 += page.temperature;
    }
    // update the LC_T1.min_heap
    LC_T1.min_heap = std::move(new_min_heap_T1);

    double sum_temp_T2 = 0.0;
    std::priority_queue<Page, std::vector<Page>, std::greater<>> new_min_heap_T2;
    for (auto& elem : LC_T2.page_map) {
        int page_id = elem.first;
        auto& page = elem.second;
        int last_req_round = page.last_request_round;
        int& last_drop_round = page.last_tempdrop_round;
        // Idle time since the last request
        int idle_time = current_round - last_req_round;
        // If it has been temp_drop_thrd rounds after the last request of a page, drop its temperature
        if (idle_time > temp_drop_thrd) {
            // Calculate the correct drop ratio
            int rounds_since_last_drop = current_round - last_drop_round;
            int drop_ratio = rounds_since_last_drop / temp_drop_thrd;
            // Lowest temperature is 0.0
            page.temperature = std::max(page.temperature - temp_drop_scale * drop_ratio, 0.0);
            // Update last drop round
            last_drop_round = current_round - (rounds_since_last_drop % temp_drop_thrd);
        }
        // push all pages into the new heap
        new_min_heap_T2.push(elem.second);
        sum_temp_T2 += page.temperature;
    }
    // update the LC_T2.min_heap
    LC_T2.min_heap = std::move(new_min_heap_T2);

    double sum_temp_T3 = 0.0;
    for (auto& elem : LC_T3.page_map) {
        int page_id = elem.first;
        auto& page = elem.second;
        int last_req_round = page.last_request_round;
        int& last_drop_round = page.last_tempdrop_round;
        // Idle time since the last request
        int idle_time = current_round - last_req_round;
        // If it has been temp_drop_thrd rounds after the last request of a page, drop its temperature
        if (idle_time > temp_drop_thrd) {
            // Calculate the correct drop ratio
            int rounds_since_last_drop = current_round - last_drop_round;
            int drop_ratio = rounds_since_last_drop / temp_drop_thrd;
            // Lowest temperature is 0.0
            page.temperature = std::max(page.temperature - temp_drop_scale * drop_ratio, 0.0);
            // Update last drop round
            last_drop_round = current_round - (rounds_since_last_drop % temp_drop_thrd);
            // no need to push into min_heap for Tier3
            // LC_T3.min_heap.push(page);
        }
        sum_temp_T3 += page.temperature;
    }

    return std::make_tuple(sum_temp_T1/LC_T1.page_map.size(),
                           sum_temp_T2/LC_T2.page_map.size(),
                           sum_temp_T3/LC_T3.page_map.size());
}


// Calculate state variables (<s1,s2>)

// double cal_avg_temp(const std::unordered_map<int, double>& map) {
//     // s1: average temperature

//     // // Use std::accumulate to sum the values and std::map's size to get the count
//     // double sum = std::accumulate(map.begin(), map.end(), 0.0,
//     //                             [](double partialSum, const auto& pair) {
//     //                                 return partialSum + pair.second;
//     //                             });

//     // Use std::reduce with execution policy for parallel accumulation
//     double sum = std::reduce(std::execution::par, map.begin(), map.end(), 0.0,
//                              [](double partialSum, const auto& pair) {
//                                  return partialSum + pair.second;
//                              });

// new cal_s1 function using TEMPCache.page_map
double cal_avg_temp(const std::unordered_map<int, Page>& page_map) {
    // Use std::reduce with execution policy for parallel accumulation
    double sum = std::reduce(std::execution::par, page_map.begin(), page_map.end(), 0.0,
                             [](double partialSum, const auto& pair) {
                                 return partialSum + pair.second.temperature;  // Access Page.temperature
                             });

    double s1 = 0.0;  // Initialize s1 here, if empty Tier then s1 = 0

    if (!page_map.empty()) {
        s1 = sum / page_map.size();
    }

    return s1;
}

double cal_s2(int current_threads, int k_thrd) {
    double s2 = std::exp(static_cast<double>(current_threads - k_thrd));
    return s2;
}



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
    // temperature increase/drop hyperparameters
    int temp_incr_buffersize = 1e4;
    double temp_incr_alpha = 0.05;
    int temp_drop_thrd = 1000; //total_num_pages/10;
    double temp_drop_scale = 0.02;
    int temp_drop_freqs = 5*temp_drop_thrd;//3000;
    // set the lower bound for upgrade criteria, change according to page access distribution
    int min_access_toT1 = 3; 
    int min_access_toT2 = 1; 

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
            asym_tier1 = std::stod(arg.substr(12)); // Extracts and converts the substring after "-asym_tier1=" to a double
        } else if (arg.find("-asym_tier2=") == 0) {
            asym_tier2 = std::stod(arg.substr(12)); // Extracts and converts the substring after "-asym_tier2=" to a double
        } else if (arg.find("-asym_tier3=") == 0) {
            asym_tier3 = std::stod(arg.substr(12)); // Extracts and converts the substring after "-asym_tier3=" to a double
        } else if (arg.find("-num_threads_tier3=") == 0) {
            num_threads_tier3 = static_cast<int>(std::stod(arg.substr(19))); // Extracts and converts the substring after "-num_threads_tier3=" to an integer
        } 
        
          else if (arg.find("-temp_incr_buffersize=") == 0) {
            temp_incr_buffersize = static_cast<int>(std::stod(arg.substr(22))); // Extracts and converts the substring after "-temp_incr_buffersize=" to an integer
        } else if (arg.find("-temp_incr_alpha=") == 0) {
            temp_incr_alpha = std::stod(arg.substr(17)); // Extracts and converts the substring after "-temp_incr_alpha=" to a double
        } else if (arg.find("-temp_drop_thrd=") == 0) {
            temp_drop_thrd = static_cast<int>(std::stod(arg.substr(16))); // Extracts and converts the substring after "-temp_drop_thrd=" to an integer
        } else if (arg.find("-temp_drop_freqs=") == 0) {
            temp_drop_freqs = static_cast<int>(std::stod(arg.substr(17))); // Extracts and converts the substring after "-temp_drop_freqs=" to an integer
        } else if (arg.find("-temp_drop_scale=") == 0) {
            temp_drop_scale = std::stod(arg.substr(17)); // Extracts and converts the substring after "-temp_drop_scale=" to a double
        } else if (arg.find("-min_access_toT1=") == 0) {
            min_access_toT1 = static_cast<int>(std::stod(arg.substr(17))); // Extracts and converts the substring after "-min_access_toT1=" to an integer
        } else if (arg.find("-min_access_toT2=") == 0) {
            min_access_toT2 = static_cast<int>(std::stod(arg.substr(17))); // Extracts and converts the substring after "-min_access_toT2=" to an integer
        } 
    }

    // define workload path
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
    std::string log_path = folder_name + "/output_" + workload + "_TEMP.log";//-" + std::to_string(temp_drop_freqs) + "-" + std::to_string(RL_update_freqs) + ".log";
    std::ofstream logFile(log_path);

    auto cout_buff = std::cout.rdbuf(); 
    std::cout.rdbuf(logFile.rdbuf());

    // // Open txt file for tiers' content in each step
    // std::string outtier_path = folder_name + "/Tier123_" + workload + "_RL.txt";
    // std::ofstream outTier(outtier_path);


    // Define thread pools, with different concurrency
    BS::thread_pool pool1(num_threads_tier1);
    BS::thread_pool pool2(num_threads_tier2);
    BS::thread_pool pool3(num_threads_tier3);

    // Check the type of pool
    // const std::type_info& typeInfo = typeid(pool1);
    // std::cout << "Type name of pool: " << typeInfo.name() << std::endl;


    // Declare maps with int keys (page no.) and double values (temperature)
    // // For pages in each tier
    // std::unordered_map<int, double> Tier1;  // max capacity 10%
    // std::unordered_map<int, double> Tier2;  // max capacity 30%
    // std::unordered_map<int, double> Tier3;  // max capacity 100%

    // // min_heap for fast extraction of lowest temperature page
    // std::priority_queue<Page, std::vector<Page>, std::greater<>> min_heap_T1;
    // std::priority_queue<Page, std::vector<Page>, std::greater<>> min_heap_T2;
    // std::priority_queue<Page, std::vector<Page>, std::greater<>> min_heap_T3;

    //TEMPCache class for each Tier
    TEMPCache LC_T1(max_capacity_tier1, temp_incr_alpha, temp_incr_buffersize);
    TEMPCache LC_T2(max_capacity_tier2, temp_incr_alpha, temp_incr_buffersize);
    TEMPCache LC_T3(max_capacity_tier3, temp_incr_alpha, temp_incr_buffersize);

    // Initiation rule can be changed here.
    if (workload == "YCSB" || workload == "TPCC" || workload == "TPCE") {
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
            LC_T1.addPage(Page{key, 0.5, {}, 0, 0});
        }
        // Insert next max_capacity_tier2 ids into Tier2
        for (size_t i = max_capacity_tier1; i < max_capacity_tier1 + max_capacity_tier2 && i < allKeys.size(); ++i) {
            int key = allKeys[i];
            LC_T2.addPage(Page{key, 0.5, {}, 0, 0});
        }
        // Insert remaining ids into Tier3
        for (size_t i = max_capacity_tier1 + max_capacity_tier2; i < max_capacity_tier3 && i < allKeys.size(); ++i) {
            int key = allKeys[i];
            LC_T3.addPage(Page{key, 0.5, {}, 0, 0});
            // clear the useless min_hap for Tier3 to free space
            LC_T3.min_heap.pop();
        }
    }
    else {
        // initial policy: fill each tier first
        for (int key = 0; key < max_capacity_tier1; ++key) {
            LC_T1.addPage(Page{key, 0.5, {}, 0, 0});
        }
        for (int key = max_capacity_tier1; key < max_capacity_tier1 + max_capacity_tier2; ++key) {
            LC_T2.addPage(Page{key, 0.5, {}, 0, 0});
        }
        for (int key = max_capacity_tier1 + max_capacity_tier2; key < total_num_pages; ++key) {
            LC_T3.addPage(Page{key, 0.5, {}, 0, 0});
            // clear the useless min_hap for Tier3 to free space
            LC_T3.min_heap.pop();
        }
    }


    // initiate avg_temp variables
    double avg_temp_T1 = 0.5;
    double avg_temp_T2 = 0.5;
    double avg_temp_T3 = 0.5;


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
        // std::cout << "Begin request " << i << ", " << action << " page " << n_page << std::endl;


        // Locate current Tier the page is in
        auto it1 = LC_T1.page_map.find(n_page);
        auto it2 = LC_T2.page_map.find(n_page);
        auto it3 = LC_T3.page_map.find(n_page);


        // auto start_time_if = std::chrono::high_resolution_clock::now();

        if (it1 != LC_T1.page_map.end()) {
            // std::cout << "page " << n_page << " currently in Tier1" << std::endl;
            ++page_hit_Tier1;

            // auto start_time_action = std::chrono::high_resolution_clock::now();
            // Execute action
            int tier_num = 1;
            std::future<void> my_future = pool1.submit_task(
                [n_page, action, tier_num,
                read_time_tier1, asym_tier1,
                read_time_tier2, asym_tier2,
                read_time_tier3, asym_tier3]{
                R_n_W(n_page, action, tier_num,
                      read_time_tier1, asym_tier1,
                      read_time_tier2, asym_tier2,
                      read_time_tier3, asym_tier3);
                });
            // my_future.wait_for(std::chrono::microseconds(1));
            //my_future.wait();
            //std::cout << action << " done.\n"<< std::endl;

            // Access the key
            // int page_id = it1->first;
            // Access the Page object
            Page& page = it1->second;
            // record old temperature
            double old_temp = page.temperature;
            // Update its temperature
            LC_T1.requestPage(n_page, i);
            // record the temperature changes for calculating new avg_temp
            double temp_delta = page.temperature - old_temp;
            // update the avg_temp_T1
            avg_temp_T1 = (avg_temp_T1 * LC_T1.page_map.size() + temp_delta) / LC_T1.page_map.size();

            // auto end_time_action = std::chrono::high_resolution_clock::now();
            // auto duration_action = std::chrono::duration_cast<std::chrono::microseconds>(end_time_action - start_time_action);
            // std::cout << "Action and update temp takes " << duration_action.count() << " microseconds" << std::endl;

            // no need for migration if page already in Tier1

        }

        else if (it2 != LC_T2.page_map.end()) {
            // std::cout << "page " << n_page << " currently in Tier2" << std::endl;
            ++page_hit_Tier2;

            // auto start_time_action = std::chrono::high_resolution_clock::now();
            // Execute action
            int tier_num = 2;
            std::future<void> my_future = pool2.submit_task(
                [n_page, action, tier_num,
                read_time_tier1, asym_tier1,
                read_time_tier2, asym_tier2,
                read_time_tier3, asym_tier3]{
                R_n_W(n_page, action, tier_num,
                      read_time_tier1, asym_tier1,
                      read_time_tier2, asym_tier2,
                      read_time_tier3, asym_tier3);
                });
            //my_future.wait();
            //std::cout << action << " done.\n"<< std::endl;

            // Access the key
            // int page_id = it2->first;
            // Access the Page object
            Page& page = it2->second;
            // record old temperature
            double old_temp = page.temperature;
            // Update its temperature
            LC_T2.requestPage(n_page, i);
            // record the temperature changes for calculating new avg_temp
            double temp_delta = page.temperature - old_temp;
            // update the avg_temp_T2
            avg_temp_T2 = (avg_temp_T2 * LC_T2.page_map.size() + temp_delta) / LC_T2.page_map.size();

            // auto end_time_action = std::chrono::high_resolution_clock::now();
            // auto duration_action = std::chrono::duration_cast<std::chrono::microseconds>(end_time_action - start_time_action);
            // std::cout << "Action and update temp takes " << duration_action.count() << " microseconds" << std::endl;

            // Decide migration by temperature
            // if current page's temperature is higher than the avg_temp at upper tier, upgrade
            // if avg_temp_T1 < 0.5, use temp_increase(2) instead
            if (page.temperature > std::max(avg_temp_T1, 1-0.5/exp(temp_incr_alpha*min_access_toT1))){
                // auto start_time_migr = std::chrono::high_resolution_clock::now();
                // std::cout << "Page " << n_page << " should be moved from Tier2 to Tier1\n"<< std::endl;
                ++num_migr_t2t1;
                // migrate page from Tier2 to Tier1
                LC_T1.addPage(page);
                LC_T2.evictPage(page);
                // update avg_temp_T1&2
                avg_temp_T1 = (avg_temp_T1 * (LC_T1.page_map.size() - 1) + page.temperature) / LC_T1.page_map.size();
                avg_temp_T2 = (avg_temp_T2 * (LC_T2.page_map.size() + 1) - page.temperature) / LC_T2.page_map.size();

                // if Tier 1 is full
                while (LC_T1.page_map.size() > max_capacity_tier1){
                    // Find the page number with lowest temperature
                    Page lt_page_t1  = LC_T1.min_heap.top();
                    // std::cout << "lt page id: " << lt_page_t1.id << ", temp: " << lt_page_t1.temperature << std::endl;
                    // auto it = Tier1.find(lt_page_t1.id);
                    // if (it != Tier1.end()) {
                    //     std::cout << "Tier1[lt_page_t1.id]: " << it->second << std::endl;
                    // } else {
                    //     std::cout << "lt_page_t1.id not found in Tier1." << std::endl;
                    // }
                    // skip non_existed page or non-matched page
                    while ((LC_T1.page_map.find(lt_page_t1.id) == LC_T1.page_map.end())
                            || (lt_page_t1.temperature != LC_T1.page_map[lt_page_t1.id].temperature)) {
                        if (LC_T1.min_heap.empty()) {
                            std::cout << "min_heap_T1 is empty. Exiting loop." << std::endl;
                            break;
                        }
                        // std::cout << "Popping from min_heap_T1" << std::endl;
                        LC_T1.min_heap.pop();
                        if (!LC_T1.min_heap.empty()) {
                            lt_page_t1 = LC_T1.min_heap.top();
                            // std::cout << "New top of min_heap_T1: id = " << lt_page_t1.id << ", temperature = " << lt_page_t1.temperature << std::endl;
                        } else {
                            std::cout << "min_heap_T1 is empty after pop. Exiting loop." << std::endl;
                            break;
                        }
                    }
                    // Downgrade lt page to Tier2
                    LC_T2.addPage(lt_page_t1);
                    LC_T1.evictPage(lt_page_t1);
                    // update avg_temp_T1&2
                    avg_temp_T1 = (avg_temp_T1 * (LC_T1.page_map.size() + 1) - lt_page_t1.temperature) / LC_T1.page_map.size();
                    avg_temp_T2 = (avg_temp_T2 * (LC_T2.page_map.size() - 1) + lt_page_t1.temperature) / LC_T2.page_map.size();

                    // new task should be submitted here to simulate page downgrade
                    // read from Tier1
                    std::future<void> my_future = pool1.submit_task(
                        [read_time_tier1]{
                        // page moving time, or replace with R_n_W func
                        // sleep for ms to simulate page movement from tier1
                        std::this_thread::sleep_for(std::chrono::microseconds(read_time_tier1));
                        });
                    // write to Tier2
                    std::future<void> my_future2 = pool2.submit_task(
                        [read_time_tier2, asym_tier2]{
                        // page moving time, or replace with R_n_W func
                        // sleep for ms to simulate page movement to tier2
                        std::this_thread::sleep_for(std::chrono::microseconds(static_cast<int>(read_time_tier2*asym_tier2)));
                        // std::cout << "Page downgrading done.\n" << std::endl;
                        });
                }
                // new task due to the migration
                // read from Tier2
                std::future<void> my_future2 = pool2.submit_task(
                    [read_time_tier2]{
                    // page moving time, or replace with R_n_W func
                     // sleep for ms to simulate page movement from tier2 to tier1
                    std::this_thread::sleep_for(std::chrono::microseconds(read_time_tier2));
                    });
                // write to Tier1
                std::future<void> my_future1 = pool1.submit_task(
                    [read_time_tier1, asym_tier1]{
                    // page moving time, or replace with R_n_W func
                     // sleep for ms to simulate page movement from tier2 to tier1
                    std::this_thread::sleep_for(std::chrono::microseconds(static_cast<int>(read_time_tier1*asym_tier1)));
                    // std::cout << "Page upgrading done.\n" << std::endl;
                    });
                // auto end_time_migr = std::chrono::high_resolution_clock::now();
                // auto duration_migr = std::chrono::duration_cast<std::chrono::microseconds>(end_time_migr - start_time_migr);
                // std::cout << "migration takes " << duration_migr.count() << " microseconds" << std::endl;
            }
            else{
                // std::cout << "Page " << n_page << " should remain in Tier2.\n"<< std::endl;
            }
        }

        else if (it3 != LC_T3.page_map.end()) {
            // std::cout << "page " << n_page << " currently in Tier3" << std::endl;
            ++page_hit_Tier3;

            // auto start_time_action = std::chrono::high_resolution_clock::now();
            // Execute action
            int tier_num = 3;
            std::future<void> my_future = pool3.submit_task(
                [n_page, action, tier_num,
                read_time_tier1, asym_tier1,
                read_time_tier2, asym_tier2,
                read_time_tier3, asym_tier3]{
                R_n_W(n_page, action, tier_num,
                      read_time_tier1, asym_tier1,
                      read_time_tier2, asym_tier2,
                      read_time_tier3, asym_tier3);
                });
            //my_future.wait();
            // std::cout << action << " done.\n"<< std::endl;

            // Access the key
            // int page_id = it3->first;
            // Access the Page object
            Page& page = it3->second;
            // record old temperature
            double old_temp = page.temperature;
            // Update its temperature
            LC_T3.requestPage(n_page, i);  LC_T3.min_heap.pop(); // clean min_heap_T3 to free up space
            // record the temperature changes for calculating new avg_temp
            double temp_delta = page.temperature - old_temp;
            // update the avg_temp_T3
            avg_temp_T3 = (avg_temp_T3 * LC_T3.page_map.size() + temp_delta) / LC_T3.page_map.size();


            // auto end_time_action = std::chrono::high_resolution_clock::now();
            // auto duration_action = std::chrono::duration_cast<std::chrono::microseconds>(end_time_action - start_time_action);
            // std::cout << "Action and update temp takes " << duration_action.count() << " microseconds" << std::endl;

            // Decide migration by temperature
            // if current page's temperature is higher than the avg_temp at upper tier, upgrade
            // if avg_temp_T2 < 0.5, use temp_increase(2) instead
            if (page.temperature > std::max(avg_temp_T2, 1-0.5/exp(temp_incr_alpha*min_access_toT2))){
                // auto start_time_migr = std::chrono::high_resolution_clock::now();
                // std::cout << "Page " << n_page << " should be moved from Tier3 to Tier2\n"<< std::endl;
                ++num_migr_t3t2;
                // migrate page from Tier3 to Tier2
                LC_T2.addPage(page);
                LC_T3.evictPage(page);
                // update avg_temp_T2&3
                avg_temp_T2 = (avg_temp_T2 * (LC_T2.page_map.size() - 1) + page.temperature) / LC_T2.page_map.size();
                avg_temp_T3 = (avg_temp_T3 * (LC_T3.page_map.size() + 1) - page.temperature) / LC_T3.page_map.size();

                // if Tier 2 is full
                while (LC_T2.page_map.size() > max_capacity_tier2){
                    // Find the page number with lowest temperature
                    Page lt_page_t2  = LC_T2.min_heap.top();
                    // std::cout << "lt page id: " << lt_page_t2.id << ", temp: " << lt_page_t2.temperature << std::endl;
                    // auto it = Tier2.find(lt_page_t2.id);
                    // if (it != Tier2.end()) {
                    //     std::cout << "Tier2[lt_page_t2.id]: " << it->second << std::endl;
                    // } else {
                    //     std::cout << "lt_page_t2.id not found in Tier2." << std::endl;
                    // }
                    // skip non_existed page or non-matched page
                    while ((LC_T2.page_map.find(lt_page_t2.id) == LC_T2.page_map.end())
                            || (lt_page_t2.temperature != LC_T2.page_map[lt_page_t2.id].temperature)) {
                        if (LC_T2.min_heap.empty()) {
                            std::cout << "min_heap_T2 is empty. Exiting loop." << std::endl;
                            break;
                        }
                        // std::cout << "Popping from min_heap_T2" << std::endl;
                        LC_T2.min_heap.pop();
                        if (!LC_T2.min_heap.empty()) {
                            lt_page_t2 = LC_T2.min_heap.top();
                            // std::cout << "New top of min_heap_T2: id = " << lt_page_t2.id << ", temperature = " << lt_page_t2.temperature << std::endl;
                        } else {
                            std::cout << "min_heap_T2 is empty after pop. Exiting loop." << std::endl;
                            break;
                        }
                    }
                    // Downgrade lt page to Tier3
                    LC_T3.addPage(lt_page_t2);  LC_T3.min_heap.pop(); // clean min_heap_T3 to free up space
                    LC_T2.evictPage(lt_page_t2);
                    // update avg_temp_T2&3
                    avg_temp_T2 = (avg_temp_T2 * (LC_T2.page_map.size() + 1) - lt_page_t2.temperature) / LC_T2.page_map.size();
                    avg_temp_T3 = (avg_temp_T3 * (LC_T3.page_map.size() - 1) + lt_page_t2.temperature) / LC_T3.page_map.size();

                    // new task should be submitted here to simulate page downgrade
                    // read from Tier2
                    std::future<void> my_future = pool2.submit_task(
                        [read_time_tier2]{
                        // page moving time, or replace with R_n_W func
                        // sleep for ms to simulate page movement from tier2
                        std::this_thread::sleep_for(std::chrono::microseconds(read_time_tier2));
                        });
                    // write to Tier3
                    std::future<void> my_future2 = pool3.submit_task(
                        [read_time_tier3, asym_tier3]{
                        // page moving time, or replace with R_n_W func
                        // sleep for ms to simulate page movement to tier3
                        std::this_thread::sleep_for(std::chrono::microseconds(static_cast<int>(read_time_tier3*asym_tier3)));
                        // std::cout << "Page downgrading done.\n" << std::endl;
                        });
                }
                // new task due to the migration (combining upgrade&downgrade as one task right now)
                // read from Tier3
                std::future<void> my_future3 = pool3.submit_task(
                    [read_time_tier3]{
                    // page moving time, or replace with R_n_W func
                     // sleep for ms to simulate page movement from tier3 to tier2
                    std::this_thread::sleep_for(std::chrono::microseconds(read_time_tier3));
                    });
                // write to Tier2
                std::future<void> my_future2 = pool2.submit_task(
                    [read_time_tier2, asym_tier2]{
                    // page moving time, or replace with R_n_W func
                     // sleep for ms to simulate page movement from tier3 to tier2
                    std::this_thread::sleep_for(std::chrono::microseconds(static_cast<int>(read_time_tier2*asym_tier2)));
                    // std::cout << "Page upgrading done.\n" << std::endl;
                    });
                // auto end_time_migr = std::chrono::high_resolution_clock::now();
                // auto duration_migr = std::chrono::duration_cast<std::chrono::microseconds>(end_time_migr - start_time_migr);
                // std::cout << "migration takes " << duration_migr.count() << " microseconds" << std::endl;
            }
            else{
                // std::cout << "Page " << n_page << " should remain in Tier3.\n"<< std::endl;
            }
        }
        
        else {
            std::cerr << "page not exist in Tiers!\n" << std::endl;
        }

        // auto end_time_if = std::chrono::high_resolution_clock::now();
        // auto duration_if = std::chrono::duration_cast<std::chrono::microseconds>(end_time_if - start_time_if);
        // std::cout << "If in Tier1/2/3 takes " << duration_if.count() << " microseconds" << std::endl;

        // auto start_time_temp = std::chrono::high_resolution_clock::now();
        // // Check for Temperature drop 
        // std::tie(avg_temp_T1, avg_temp_T2, avg_temp_T3) = decrease_temperature(Tier1, Tier2, Tier3, list_last_req, i, temp_drop_thrd, temp_drop_scale);
        // Check after every 500 rounds
        if ( i > 0 && i % temp_drop_freqs ==0 ){
            // auto start_time_temp = std::chrono::high_resolution_clock::now();
            std::tie(avg_temp_T1, avg_temp_T2, avg_temp_T3) =  decrease_temperature_lz(LC_T1, LC_T2, LC_T3, i, temp_drop_thrd, temp_drop_scale);
            // auto end_time_temp = std::chrono::high_resolution_clock::now();
            // auto duration_temp = std::chrono::duration_cast<std::chrono::microseconds>(end_time_temp - start_time_temp);
            // std::cout << "drop temperatures takes " << duration_temp.count() << " microseconds" << std::endl;
        }
        // auto end_time_temp = std::chrono::high_resolution_clock::now();
        // auto duration_temp = std::chrono::duration_cast<std::chrono::microseconds>(end_time_temp - start_time_temp);
        // std::cout << "drop temperatures takes " << duration_temp.count() << " microseconds" << std::endl;


        //std::cout << "Running on pool " << rv << ", results:" << my_future.get() << '\n';
        // std::cout << "Finish request " << i+1 << "\n" << std::endl;

        // End the timer
        // auto end_time_i = std::chrono::high_resolution_clock::now();
        // // Calculate the duration
        // auto duration_i = std::chrono::duration_cast<std::chrono::microseconds>(end_time_i - start_time_i);
        // // Output the duration
        // std::cout << "request takes " << duration_i.count() << " microseconds\n" << std::endl;


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

        // std::cout << "Tier1 size: " << LC_T1.page_map.size() << ", min_heap size: " << LC_T1.min_heap.size() <<std::endl;
        // std::cout << "Tier2 size: " << LC_T2.page_map.size() << ", min_heap size: " << LC_T2.min_heap.size() <<std::endl;
        // std::cout << "Tier3 size: " << LC_T3.page_map.size() << ", min_heap size: " << LC_T3.min_heap.size() <<std::endl;
        
    }
    // End the timer of for loop
    auto end_time_for = std::chrono::high_resolution_clock::now();

    while (pool1.get_tasks_running() > 0 || pool2.get_tasks_running() > 0 || pool3.get_tasks_running() > 0 || 
           pool1.get_tasks_queued() > 0  || pool2.get_tasks_queued() > 0  || pool3.get_tasks_queued() > 0) {
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
    std::cout << "Total requests time: " << duration.count() << " microseconds\n" << std::endl;

    // counter of read/write
    std::cout << "There are " << num_reads << " reads, and " << num_write << " writes.\n" << std::endl;

    // page hits
    std::cout << "Number of page hits in Tier1: " << page_hit_Tier1 << std::endl;
    std::cout << "Number of page hits in Tier2: " << page_hit_Tier2 << std::endl;
    std::cout << "Number of page hits in Tier3: " << page_hit_Tier3 << std::endl;

    std::cout << "\nNumber of page upgraded from Tier2 to Tier1: " << num_migr_t2t1 << std::endl;
    std::cout << "Number of page upgraded from Tier3 to Tier2: " << num_migr_t3t2 << std::endl;
    

    // std::cout << "\nPages in Tier1: " <<std::endl;
    // for(const auto& elem : Tier1){
    // std::cout << elem.first << ", ";
    // }
    // std::cout << "\nAnd their temperatures: " <<std::endl;
    // for(const auto& elem : Tier1){
    // std::cout << elem.second << ", ";
    // }

    // std::cout << std::endl;
    // std::cout << "\nPages in Tier2: " <<std::endl;
    // for(const auto& elem : Tier2){
    // std::cout << elem.first << ", ";
    // }
    // std::cout << "\nAnd their temperatures: " <<std::endl;
    // for(const auto& elem : Tier2){
    // std::cout << elem.second << ", ";
    // }

    /*// Write the captured output to the log file
    std::cout << outputStream.str();

    // Restore the original std::cout buffer
    std::cout.rdbuf(originalCoutBuffer);*/
    std::cout.rdbuf(cout_buff);

    // Close the log file and tier file
    logFile.close();
    // outTier.close();

}