#include <algorithm>   // std::min, std::shuffle, std::sort, std::unique, std::min_element
#include <atomic>      // std::atomic
#include <chrono>      // std::chrono
#include <cmath>       // std::abs, std::cos, std::exp, std::llround, std::log, std::round, std::sqrt
#include <cstddef>     // std::size_t
#include <cstdint>     // std::int_fast64_t
#include <cstdlib>     // std::exit, std::quick_exit
#include <ctime>       // std::localtime, std::strftime, std::time, std::time_t, std::tm
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

// Function to move a page between tiers
void movePage(int page_id, std::unordered_set<int>& from, std::unordered_set<int>& to) {
    // Tier update
    from.erase(page_id); // O(1) average
    to.insert(page_id); // O(1) average
}

// LRU class
// Page structure to store page id and last request round
struct Page {
    int id;
    int last_request_round;

    bool operator>(const Page& other) const {
        return last_request_round > other.last_request_round;
    }
};

class LRUCache {
public:
    std::unordered_map<int, Page> page_map;
    // std::unordered_set<int> cache;
    std::priority_queue<Page, std::vector<Page>, std::greater<>> min_heap;

    LRUCache(int capacity) : capacity(capacity) {}

    void preloadPages(const std::unordered_set<int>& pages) {
        for (int page_id : pages) {
            if (page_map.size() >= capacity) {
                // If cache is full during preload, we stop adding more pages.
                break;
            }
            if (page_map.find(page_id) == page_map.end()) {
                Page new_page = {page_id, 0};
                page_map[page_id] = new_page;
                // cache.insert(page_id);
                min_heap.push(new_page);
            }
        }
    }

    void requestPage(int page_id, int current_round) {
        if (page_map.find(page_id) != page_map.end()) {
            // Page is already in cache, update recency
            page_map[page_id].last_request_round = current_round;
            min_heap.push(page_map[page_id]);
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
        page_map.erase(ev_page.id);
        // cache.erase(ev_page.id);
        min_heap.pop();
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
    // Use k_lru*recency_LRU to simulate LRU-K
    double k_lru = 1.0; 

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
            asym_tier1 = std::stod(arg.substr(17)); // Extracts and converts the substring after "-asym_tier1=" to an double
        } else if (arg.find("-asym_tier2=") == 0) {
            asym_tier2 = std::stod(arg.substr(17)); // Extracts and converts the substring after "-asym_tier2=" to an double
        } else if (arg.find("-asym_tier3=") == 0) {
            asym_tier3 = std::stod(arg.substr(17)); // Extracts and converts the substring after "-asym_tier3=" to an double
        }
        else if (arg.find("-k_lru=") == 0) {
            asym_tier3 = std::stod(arg.substr(7)); // Extracts and converts the substring after "-k_lru=" to an double
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
    std::string log_path = folder_name + "/output_" + workload + "_LRU-OSA.log";
    std::ofstream logFile(log_path);

    auto cout_buff = std::cout.rdbuf(); 
    std::cout.rdbuf(logFile.rdbuf());

    // // Open txt file for tiers' content in each step
    // std::string outtier_path = folder_name + "Tier123_" + workload_path + "_LRU.txt";
    // std::ofstream outTier(outtier_path);

    // Define thread pools, with different concurrency
    BS::thread_pool pool1(num_threads_tier1);
    BS::thread_pool pool2(num_threads_tier2);
    BS::thread_pool pool3(num_threads_tier3);

    // LFU class for Tier1, Tier2, Tier3
    LRUCache LC_T1(max_capacity_tier1);
    LRUCache LC_T2(max_capacity_tier2);
    LRUCache LC_T3(max_capacity_tier3);
    // create reference of page_map
    auto& Tier1 = LC_T1.page_map;
    auto& Tier2 = LC_T2.page_map;
    auto& Tier3 = LC_T3.page_map;    


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
            LC_T1.addPage(Page{key, 0});
        }
        // Insert next max_capacity_tier2 ids into Tier2
        for (size_t i = max_capacity_tier1; i < max_capacity_tier1 + max_capacity_tier2 && i < allKeys.size(); ++i) {
            int key = allKeys[i];
            LC_T2.addPage(Page{key, 0});
        }
        // Insert remaining ids into Tier3
        for (size_t i = max_capacity_tier1 + max_capacity_tier2; i < allKeys.size(); ++i) {
            int key = allKeys[i];
            LC_T3.addPage(Page{key, 0});
            // clean LC_T3.min_heap to free space
            LC_T3.min_heap.pop();
        }
    }
    else {
        // initial policy: fill each tier first
        for (int key = 0; key < max_capacity_tier1; ++key) {
            LC_T1.addPage(Page{key, 0});
        }
        for (int key = max_capacity_tier1; key < max_capacity_tier1 + max_capacity_tier2; ++key) {
            LC_T2.addPage(Page{key, 0});
        }
        for (int key = max_capacity_tier1 + max_capacity_tier2; key < total_num_pages; ++key) {
            LC_T3.addPage(Page{key, 0});
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
                //return curr_thrds_tier1-1;  //after finishing the action, current threads -= 1
                });
            //my_future.wait();
            // my_future.wait_for(std::chrono::microseconds(1));
            //std::cout << action << " done.\n"<< std::endl;

            // Update its recency
            LC_T1.requestPage(n_page, i);
            
            // no need for migration if page already in Tier1
        }

        else if (it2 != Tier2.end()) {
            // std::cout << "page " << n_page << " currently in Tier2\n" << std::endl;
            ++page_hit_Tier2;

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
                //return curr_thrds_tier2-1;  //after finishing the action, current threads -= 1
                });
            //my_future.wait();
            //std::cout << action << " done.\n"<< std::endl;

            // Update page's recency after decision

            // Decide migration by LRU
            // Compare the recency of current page with LRU pages in Tier1
            Page current_page = LC_T2.page_map[n_page];
            Page lru_page_t1  = LC_T1.min_heap.top();
            // skip non_existed page or non-matched LRU page
            while ((Tier1.find(lru_page_t1.id) == Tier1.end()) || 
                   (lru_page_t1.last_request_round != LC_T1.page_map[lru_page_t1.id].last_request_round)) {
                LC_T1.min_heap.pop();
                lru_page_t1  = LC_T1.min_heap.top();
            }
            // Use OSA for upgrade, i.e. upgrade one single access (with no constrain)
            // if (current_page.last_request_round > int(k_lru * lru_page_t1.last_request_round + max_capacity_tier1)){
            if (1 == 1){
                ++num_migr_t2t1;
                // std::cout << "Page " << n_page << " should be moved from Tier2 to Tier1\n"<< std::endl;
                // Update page's recency 
                LC_T2.requestPage(n_page, i);
                // movePage(n_page, Tier2, Tier1);
                LC_T1.addPage(current_page);
                LC_T2.evictPage(current_page);
                // if Tier 1 is full
                while (Tier1.size() > max_capacity_tier1){
                    // Downgrade LFU page to Tier2
                    LC_T1.evictPage(lru_page_t1);
                    LC_T2.addPage(lru_page_t1);
                    // new task should be submitted here to simulate page downgrade
                    // read from Tier1
                    std::future<void> my_future = pool1.submit_task(
                        [read_time_tier1]{
                        // page moving time, or replace with R_n_W func
                        // sleep for ms to simulate page movement from tier1
                        std::this_thread::sleep_for(std::chrono::microseconds(read_time_tier1));
                        //std::cout << "Page migration done.\n" << std::endl;
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
                    //std::cout << "Page migration done.\n" << std::endl;
                    });
                // write to Tier1
                std::future<void> my_future1 = pool1.submit_task(
                    [read_time_tier1, asym_tier1]{
                    // page moving time, or replace with R_n_W func
                     // sleep for ms to simulate page movement from tier2 to tier1
                    std::this_thread::sleep_for(std::chrono::microseconds(static_cast<int>(read_time_tier1*asym_tier1)));
                    // std::cout << "Page upgrading done.\n" << std::endl;
                    });
            // check whether Tiers = LC_Ti.cache
            }
            else{
                // std::cout << "Page " << n_page << " should remain in Tier2.\n"<< std::endl;
                // Update page's recency 
                LC_T2.requestPage(n_page, i);
            }
        }

        else if (it3 != Tier3.end()) {
            // std::cout << "page " << n_page << " currently in Tier3\n" << std::endl;
            ++page_hit_Tier3;

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
                //return curr_thrds_tier3-1;  //after finishing the action, current threads -= 1
                });
            //my_future.wait();
            //std::cout << action << " done.\n"<< std::endl;

            // Update page's recency after decision

            // Decide migration by LRU
            // Compare the recency of current page with LRU pages in Tier2
            Page current_page = LC_T3.page_map[n_page];
            Page lru_page_t2  = LC_T2.min_heap.top();
            // skip non_existed page or non-matched LRU page
            while ((Tier2.find(lru_page_t2.id) == Tier2.end()) || 
                   (lru_page_t2.last_request_round != LC_T2.page_map[lru_page_t2.id].last_request_round)) {
                LC_T2.min_heap.pop();
                lru_page_t2  = LC_T2.min_heap.top();
            }
            // Use OSA for upgrade, i.e. upgrade one single access (with no constrain)
            // if (current_page.last_request_round > int(k_lru * lru_page_t2.last_request_round + max_capacity_tier2)){
            if (1 == 1){    
                ++num_migr_t3t2;
                // std::cout << "Page " << n_page << " should be moved from Tier3 to Tier2\n"<< std::endl;
                // Update page's recency 
                LC_T3.requestPage(n_page, i);
                // clean LC_T3.min_heap to free space
                // LC_T3.min_heap.pop();
                // movePage(n_page, Tier3, Tier2);
                LC_T2.addPage(current_page);
                LC_T3.evictPage(current_page);
                // if Tier 2 is full
                while (Tier2.size() > max_capacity_tier2){
                    // Downgrade LFU page to Tier3
                    LC_T2.evictPage(lru_page_t2);
                    LC_T3.addPage(lru_page_t2);
                    // clean LC_T3.min_heap to free space
                    LC_T3.min_heap.pop();
                    // new task should be submitted here to simulate page downgrade
                    // read from Tier2
                    std::future<void> my_future = pool2.submit_task(
                        [read_time_tier2]{
                        // page moving time, or replace with R_n_W func
                        // sleep for ms to simulate page movement from tier2
                        std::this_thread::sleep_for(std::chrono::microseconds(read_time_tier2));
                        //std::cout << "Page migration done.\n" << std::endl;
                        });
                    // write to Tier3
                    std::future<void> my_future2 = pool3.submit_task(
                        [read_time_tier3, asym_tier3]{
                        // page moving time, or replace with R_n_W func
                        // sleep for ms to simulate page movement to tier3
                        std::this_thread::sleep_for(std::chrono::microseconds(static_cast<int>(read_time_tier3*asym_tier3)));
                        // std::cout << "Page dowgrading done.\n" << std::endl;
                        });
                }
                // new task due to the migration (combining upgrade&downgrade as one task right now)
                // read from Tier3
                std::future<void> my_future3 = pool3.submit_task(
                    [read_time_tier3]{
                    // page moving time, or replace with R_n_W func
                     // sleep for ms to simulate page movement from tier2 to tier1
                    std::this_thread::sleep_for(std::chrono::microseconds(read_time_tier3));
                    //std::cout << "Page migration done.\n" << std::endl;
                    });
                // write to Tier2
                std::future<void> my_future2 = pool2.submit_task(
                    [read_time_tier2, asym_tier2]{
                    // page moving time, or replace with R_n_W func
                     // sleep for ms to simulate page movement from tier2 to tier1
                    std::this_thread::sleep_for(std::chrono::microseconds(static_cast<int>(read_time_tier2*asym_tier2)));
                    // std::cout << "Page upgrading done.\n" << std::endl;
                    });
            }
            else{
                // std::cout << "Page " << n_page << " should remain in Tier3.\n"<< std::endl;
                // Update page's recency 
                LC_T3.requestPage(n_page, i);
                // clean LC_T3.min_heap to free space
                // LC_T3.min_heap.pop();
            }
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

    // number of migrations
    std::cout << "\nNumber of page migrated from Tier2 to Tier1: " << num_migr_t2t1 << std::endl;
    std::cout << "Number of page migrated from Tier3 to Tier2: " << num_migr_t3t2 << std::endl;
    

    std::cout.rdbuf(cout_buff); 
    // Close the log file and tier file
    logFile.close();
    file.close();
    // outTier.close();

}
