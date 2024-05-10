#include <algorithm>   // std::min, std::shuffle, std::sort, std::unique, std::min_element
#include <atomic>      // std::atomic
#include <chrono>      // std::chrono
#include <cmath>       // std::abs, std::cos, std::exp, std::llround, std::log, std::round, std::sqrt
#include <cstddef>     // std::size_t
#include <cstdint>     // std::int_fast64_t
#include <cstdlib>     // std::exit, std::quick_exit
#include <ctime>       // std::localtime, std::strftime, std::time, std::time_t, std::tm
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
            std::cout << "reading page " << pageNum << " from Tier 1 ...\n";
            // Sleep for 0.01 seconds to simulate the read action
            std::this_thread::sleep_for(std::chrono::milliseconds(read_time_tier1));
            std::cout << "Read Done." << std::endl;
        }
        else if (action == "Write"){
            std::cout << "writing page " << pageNum << " to Tier 1 ...\n";
            // Sleep for 0.03 seconds to simulate the write action
            // write_time_tier1 = read_time_tier1*asym_tier1
            std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>(read_time_tier1*asym_tier1)));
            std::cout << "Write Done." << std::endl;
        }
        else {
            std::cout << "Unknown action !" << std::endl;
        }
    }
    // R/W in Tier2
    else if (tier_num == 2){
        if (action == "Read"){
            std::cout << "reading page " << pageNum << " from Tier 2 ...\n";
            // Sleep for 0.05 seconds to simulate the read action
            std::this_thread::sleep_for(std::chrono::milliseconds(read_time_tier2));
            std::cout << "Read Done." << std::endl;
        }
        else if (action == "Write"){
            std::cout << "writing page " << pageNum << " to Tier 2 ...\n";
            // Sleep for 0.1 seconds to simulate the write action
            // write_time_tier2 = read_time_tier2*asym_tier2
            std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>(read_time_tier2*asym_tier2)));
            std::cout << "Write Done." << std::endl;
        }
        else {
            std::cout << "Unknown action !" << std::endl;
        }
    }
    // R/W in Tier3
    else if (tier_num == 3){
        if (action == "Read"){
            std::cout << "reading page " << pageNum << " from Tier 3 ...\n";
            // Sleep for 0.2 seconds to simulate the read action
            std::this_thread::sleep_for(std::chrono::milliseconds(read_time_tier3));
            std::cout << "Read Done." << std::endl;
        }
        else if (action == "Write"){
            std::cout << "writing page " << pageNum << " to Tier 3 ...\n";
            // Sleep for 0.3 seconds to simulate the write action
            // write_time_tier3 = read_time_tier3*asym_tier3
            std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>(read_time_tier3*asym_tier3)));
            std::cout << "Read Done." << std::endl;
        }
        else {
            std::cout << "Unknown action ! \n" << std::endl;
        }    
    }
}

// Page movement between tiers
std::pair<std::map<int, double>, std::map<int, double>> PageMigrate(
    const std::map<int, double>& map1,
    const std::map<int, double>& map2,
    int keyToMove) {
    
    std::map<int, double> resultMap1 = map1;
    std::map<int, double> resultMap2 = map2;

    auto it1 = resultMap1.find(keyToMove);
    auto it2 = resultMap2.find(keyToMove);

    if (it1 != resultMap1.end()) {
        resultMap2.insert(*it1);
        resultMap1.erase(it1);
        //std::cout << "Page " << keyToMove << " moved from Tier 1 to Tier 2.\n";
    } else if (it2 != resultMap2.end()) {
        resultMap1.insert(*it2);
        resultMap2.erase(it2);
        //std::cout << "Page " << keyToMove << " moved from Tier 2 to Tier 1.\n";
    } else {
        std::cout << "Page " << keyToMove << " not found in either Tier.\n";
    }

    // return new maps for Tiers after page being moved
    return std::make_pair(resultMap1, resultMap2);
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

// Temperature increase
double temp_increase(int PageKey, const std::map<int, int>& list_num_req, double temp_incr_alpha){

    int req_time = list_num_req.find(PageKey)->second;   // get historical number of request of this page
    double temp = 1 - 0.5 / exp(temp_incr_alpha * req_time);        // calculate the new temperature  // alpha=0.05

    return temp;
}


int main(
        // what should be put here?
    ){
    // set environmental arguments here
    // read/write time of each tier
    int read_time_tier1 = 2;  float asym_tier1 = 2.0;
    int read_time_tier2 = 10; float asym_tier2 = 3.0;
    int read_time_tier3 = 50; float asym_tier3 = 4.0;
    // page migration time, replaced by a function acting real read/write 
    // upgrade time
    //int page_migr_time_t2t1 = 50;
    //int page_migr_time_t3t2 = 500;
    // downgrade time, can be ignored if upgrade = add replica
    //int page_migr_time_t1t2 = 150;
    //int page_migr_time_t2t3 = 300;
    // capacity of each tier
    int max_capacity_tier1 = 10;
    int max_capacity_tier2 = 30;
    int max_capacity_tier3 = 100;
    // Concurrency (available number of threads)
    int num_threads_tier1 = 8;
    int num_threads_tier2 = 4;
    int num_threads_tier3 = 2;
    // concurrency threashold (k_read/write)
    int k_thrd_tier1 = 9;
    int k_thrd_tier2 = 5;
    int k_thrd_tier3 = 3;
    // total number of pages
    int total_num_pages = 100;
    // total number of requests
    int total_num_reqs = 10000;
    // use real workload
    std::string workload_path = "workload_20hf90_rw1_10000.txt";
    // temperature increase/drop hyperparameters
    int temp_incr_buffersize = 1000;
    double temp_incr_alpha = 0.10;
    int temp_drop_thrd = 20;
    double temp_drop_scale = 0.05;

    // Open the log file for writing
    std::ofstream logFile("output_20hf90_rw1_10000_LRU.log");

    /*// Create a stringstream to capture the output
    std::ostringstream outputStream;

    // Redirect std::cout to the log file and the stringstream
    std::streambuf* originalCoutBuffer = std::cout.rdbuf();
    std::streambuf* logFileBuffer = logFile.rdbuf();
    std::streambuf* outputStreamBuffer = outputStream.rdbuf();
    std::cout.rdbuf(outputStreamBuffer);*/

    auto cout_buff = std::cout.rdbuf(); 
    std::cout.rdbuf(logFile.rdbuf());

    // Open txt file for tiers' content in each step
    std::ofstream outTier("Tier123_20hf90_rw1_10000_LRU.txt");

    // Define thread pools, with different concurrency
    BS::thread_pool pool1(num_threads_tier1);
    BS::thread_pool pool2(num_threads_tier2);
    BS::thread_pool pool3(num_threads_tier3);
    
    // Create a vector for all pools
    std::vector<BS::thread_pool*> pools = {&pool1, &pool2, &pool3};

    // Declare maps with int keys (page no.) and double values (temperature)
    // For pages in each tier
    std::map<int, double> Tier1;  // max capacity 10
    std::map<int, double> Tier2;  // max capacity 30
    std::map<int, double> Tier3;  // max capacity 100

    // Initiation rule can be changed here.
    // initial policy: fill each tier and keep them static
    for (int key = 0; key < max_capacity_tier1; ++key) {
        Tier1[key] = 0.5;
    }
    for (int key = max_capacity_tier1; key < max_capacity_tier1 + max_capacity_tier2; ++key) {
        Tier2[key] = 0.5;
    }
    for (int key = max_capacity_tier1 + max_capacity_tier2; key < total_num_pages; ++key) {
        Tier3[key] = 0.5;
    }

    // List of access frequency, key = page no., value = request time
    std::map<int, int> list_num_req;
    for (int key = 0; key < total_num_pages; ++key) {
        list_num_req[key] = 0;
    }

    // List of idle time, how long since last request
    std::map<int, int> list_idle;
    std::map<int, int> list_idle_LRU;    // This is a list of idle time without zero out, for recency count 
    for (int key = 0; key < total_num_pages; ++key) {
        list_idle[key] = 0;
        list_idle_LRU[key] = 0;
    }

    // Vector to store the table
    std::vector<std::pair<int, std::string>> ReqTable;

    // Generate the request table
    /*for (int i = 0; i < total_num_reqs; ++i) {  // Generating 500 requests for example
        int randomNumber = distNumbers(gen);
        std::string randomAction = actions[distActions(gen)];
        ReqTable.push_back(std::make_pair(randomNumber, randomAction));
    }*/

    // Or load from workload file
     // Open the file
    std::ifstream file(workload_path);
     // Read file line by line
    std::string line;
    while (std::getline(file, line)) {
            std::istringstream iss(line);
            char action;
            int pageNumber;
            if (iss >> action >> pageNumber) {
                // Convert 'R' to "Read" and 'W' to "Write"
                std::string pageAction = (action == 'R') ? "Read" : "Write";
                ReqTable.emplace_back(pageNumber, pageAction);  // Store the pair
            } 
            else {
                std::cerr << "Invalid line: " << line << std::endl;
            }
        }
     // Close the file
    file.close();
 

    // Counter of Read/Write requests
    int num_reads = 0;
    int num_write = 0;

    // Counter of page hit
    int page_hit_Tier1 = 0;
    int page_hit_Tier2 = 0;
    int page_hit_Tier3 = 0;
 

    // Start the timer
    auto start_time = std::chrono::high_resolution_clock::now();

    // Begin loop of requests
    for (int i = 0; i < total_num_reqs; ++i){

        // Page number being requested
        int n_page = ReqTable[i].first;
        // Read/write
        std::string action = ReqTable[i].second;

        std::cout << "Begin request " << i+1 << ", " << action << " page " << n_page << "\n" << std::endl;

        if (action == "Read"){
            num_reads++;
        }
        else if (action == "Write"){
            num_write++;
        }
        else {
            std::cerr << "Unknown action!" << std::endl;
        }

        // Count occurrences of n_page in the last 1000 requests (temp_incr_buffersize=1000)
        int freq_count = 0;
        for (int j = i - temp_incr_buffersize + 1; j <= i; ++j) {
            if (j >= 0 && ReqTable[j].first == n_page) {
                freq_count++;
            }
        }
        list_num_req[n_page] = freq_count;

        // idle time since last request +1
        for (auto& pair : list_idle) {
            pair.second += 1;
        }
        // except for the current requested page
        list_idle[n_page] = 0;

        // idle time since last request +1
        for (auto& pair : list_idle_LRU) {
            pair.second += 1;
        }
        // except for the current requested page
        list_idle_LRU[n_page] = 0;

        // Locate current Tier the page is in
        auto it1 = Tier1.find(n_page);
        auto it2 = Tier2.find(n_page);
        auto it3 = Tier3.find(n_page);

        if (it1 != Tier1.end()) {
            std::cout << "page " << n_page << " currently in Tier1\n" << std::endl;
            ++page_hit_Tier1;
            // Choose the corresponding pool
            BS::thread_pool& selectedPool = *pools[0];

            // Execute action
            int tier_num = 1;
            std::future<void> my_future = selectedPool.submit_task(
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
            // my_future.wait_for(std::chrono::milliseconds(1));
            //std::cout << action << " done.\n"<< std::endl;

            // Update its temperature
            Tier1[n_page] = temp_increase(n_page, list_num_req, temp_incr_alpha);
            
            // no need for migration if page already in Tier1
        }

        else if (it2 != Tier2.end()) {
            std::cout << "page " << n_page << " currently in Tier2\n" << std::endl;
            ++page_hit_Tier2;
            // Choose the corresponding pool
            BS::thread_pool& selectedPool = *pools[1];

            // Execute action
            int tier_num = 2;
            std::future<void> my_future = selectedPool.submit_task(
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

            // Update its temperature
            Tier2[n_page] = temp_increase(n_page, list_num_req, temp_incr_alpha);

            // Decide migration by LRU-K

            // Compare the recency of all pages in Tier1 with K
            int n_smaller = 0;   // number of pages in Tier1 which has older recency than K = 40
            for (const auto& pair : Tier1) {
                int key = pair.first;
                if (list_idle_LRU[key] > 60){   // K=40 is a hyperparameter here
                    ++n_smaller;
                }
            }
            // If any page in Tier1 has recency older than K, then upgrade current page to Tier1
            if (n_smaller > 2){
                std::cout << "Page " << n_page << " should be moved from Tier2 to Tier1\n"<< std::endl;
                auto tier_pair = PageMigrate(Tier2,Tier1,n_page);
                Tier2 = tier_pair.first;
                Tier1 = tier_pair.second;
                // if Tier 1 is full
                while (Tier1.size() > max_capacity_tier1){
                    // Find the page number with lowest temperature
                    auto minElement = std::min_element(Tier1.begin(), Tier1.end(),
                        [](const auto& lhs, const auto& rhs) {
                            return lhs.second < rhs.second;
                        }
                    );
                    int minKey = minElement->first;
                    // Downgrade that page to Tier2
                    auto tier_pair = PageMigrate(Tier2,Tier1,minKey);
                    Tier2 = tier_pair.first;
                    Tier1 = tier_pair.second;
                    // new task should be submitted here to simulate page downgrade
                    // read from Tier1
                    std::future<void> my_future = pool1.submit_task(
                        [read_time_tier1]{
                        // page moving time, or replace with R_n_W func
                        // sleep for ms to simulate page movement from tier1
                        std::this_thread::sleep_for(std::chrono::milliseconds(read_time_tier1));
                        //std::cout << "Page migration done.\n" << std::endl;
                        });
                    // write to Tier2
                    std::future<void> my_future2 = pool2.submit_task(
                        [read_time_tier2, asym_tier2]{
                        // page moving time, or replace with R_n_W func
                        // sleep for ms to simulate page movement to tier2
                        std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>(read_time_tier2*asym_tier2)));
                        std::cout << "Page downgrading done.\n" << std::endl;
                        });
                }
                // new task due to the migration
                // read from Tier2
                std::future<void> my_future = pool2.submit_task(
                    [read_time_tier2]{
                    // page moving time, or replace with R_n_W func
                     // sleep for ms to simulate page movement from tier2 to tier1
                    std::this_thread::sleep_for(std::chrono::milliseconds(read_time_tier2));
                    //std::cout << "Page migration done.\n" << std::endl;
                    });
                // write to Tier1
                std::future<void> my_future1 = pool1.submit_task(
                    [read_time_tier1, asym_tier1]{
                    // page moving time, or replace with R_n_W func
                     // sleep for ms to simulate page movement from tier2 to tier1
                    std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>(read_time_tier1*asym_tier1)));
                    std::cout << "Page upgrading done.\n" << std::endl;
                    });
            }
            else{
                std::cout << "Page " << n_page << " should remain in Tier2.\n"<< std::endl;
            }
        }

        else if (it3 != Tier3.end()) {
            std::cout << "page " << n_page << " currently in Tier3\n" << std::endl;
            ++page_hit_Tier3;
            // Choose the corresponding pool
            BS::thread_pool& selectedPool = *pools[2];

            // Execute action
            int tier_num = 3;
            std::future<void> my_future = selectedPool.submit_task(
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

            // Update its temperature
            Tier3[n_page] = temp_increase(n_page, list_num_req, temp_incr_alpha);

            // Decide migration by LRU-K

            // Compare the recency of all pages in Tier2 with K
            int n_smaller = 0;   // number of pages in Tier2 which has older recency than K = 100
            for (const auto& pair : Tier2) {
                int key = pair.first;
                if (list_idle_LRU[key] > 120){   // K=100 is a hyperparameter here
                    ++n_smaller;
                }
            }
            // If any page in Tier2 has recency older than K, then upgrade current page to Tier2
            if (n_smaller > 2){
                std::cout << "Page " << n_page << " should be moved from Tier3 to Tier2\n"<< std::endl;
                auto tier_pair = PageMigrate(Tier3,Tier2,n_page);
                Tier3 = tier_pair.first;
                Tier2 = tier_pair.second;
                // if Tier 2 is full
                while (Tier2.size() > max_capacity_tier2){
                    // Find the page number with lowest temperature
                    auto minElement = std::min_element(Tier2.begin(), Tier2.end(),
                        [](const auto& lhs, const auto& rhs) {
                            return lhs.second < rhs.second;
                        }
                    );
                    int minKey = minElement->first;
                    // Downgrade that page to Tier2
                    auto tier_pair = PageMigrate(Tier3,Tier2,minKey);
                    Tier3 = tier_pair.first;
                    Tier2 = tier_pair.second;
                    // new task should be submitted here to simulate page downgrade
                    // read from Tier2
                    std::future<void> my_future = pool2.submit_task(
                        [read_time_tier2]{
                        // page moving time, or replace with R_n_W func
                        // sleep for ms to simulate page movement from tier2
                        std::this_thread::sleep_for(std::chrono::milliseconds(read_time_tier2));
                        //std::cout << "Page migration done.\n" << std::endl;
                        });
                    // write to Tier2
                    std::future<void> my_future2 = pool3.submit_task(
                        [read_time_tier3, asym_tier3]{
                        // page moving time, or replace with R_n_W func
                        // sleep for ms to simulate page movement to tier3
                        std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>(read_time_tier3*asym_tier3)));
                        std::cout << "Page downgrading done.\n" << std::endl;
                        });
                }
                // new task due to the migration (combining upgrade&downgrade as one task right now)
                // read from Tier3
                std::future<void> my_future = pool3.submit_task(
                    [read_time_tier3]{
                    // page moving time, or replace with R_n_W func
                     // sleep for ms to simulate page movement from tier2 to tier1
                    std::this_thread::sleep_for(std::chrono::milliseconds(read_time_tier3));
                    //std::cout << "Page migration done.\n" << std::endl;
                    });
                // write to Tier2
                std::future<void> my_future2 = pool2.submit_task(
                    [read_time_tier2, asym_tier2]{
                    // page moving time, or replace with R_n_W func
                     // sleep for ms to simulate page movement from tier2 to tier1
                    std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>(read_time_tier2*asym_tier2)));
                    std::cout << "Page upgrading done.\n" << std::endl;
                    });
            }
            else{
                std::cout << "Page " << n_page << " should remain in Tier3.\n"<< std::endl;
            }
        }

        else {
            std::cout << "page not exist in Tiers!\n" << std::endl;
        }

        // temperature decrease for long idle pages
        std::vector<int> idle_pages;
        for (const auto& keyValue : list_idle) {
            // if a page hasn't been requested in the past 20 (parameter variable)
            if (keyValue.second == temp_drop_thrd) {
                idle_pages.push_back(keyValue.first);
                // reset idle counter
                list_idle[keyValue.first] = 0;
            }
        }
        // drop temperature of these pages by 0.1 (parameter variable)
        for (const auto& key : idle_pages) {
            if (Tier1.find(key) != Tier1.end()) {
                Tier1[key] -= temp_drop_scale;
            }
            if (Tier2.find(key) != Tier2.end()) {
                Tier2[key] -= temp_drop_scale;
            }
            if (Tier3.find(key) != Tier3.end()) {
                Tier3[key] -= temp_drop_scale;
            }
        }
        // If the temperature value is dropped below 0.0, set it to 0.0
        // Lambda function to set negative values to zero
        auto setNegativeToZero = [](std::pair<const int, double>& pair) {
            if (pair.second < 0) {
                pair.second = 0.0;  // Set negative values to zero
            }
        };
        // Apply the transformation to each element in Tier1,2,3
        for (auto& entry : Tier1) {setNegativeToZero(entry);}
        for (auto& entry : Tier2) {setNegativeToZero(entry);}
        for (auto& entry : Tier3) {setNegativeToZero(entry);} 
          

        //std::cout << "Running on pool " << rv << ", results:" << my_future.get() << '\n';
        std::cout << "Finish request " << i+1 << "\n" << std::endl;

        outTier << "Page distribution after request " << i+1 << std::endl;
        // Write Tier1 to the file
        outTier << "Tier1:" << std::endl;
        for (const auto& entry : Tier1) {
            outTier << entry.first << " " << entry.second << ", ";
        }
        outTier << std::endl;
        // Write Tier2 to the file
        outTier << "Tier2:" << std::endl;
        for (const auto& entry : Tier2) {
            outTier << entry.first << " " << entry.second << ", ";
        }
        outTier << std::endl;
        // Write Tier3 to the file
        outTier << "Tier3:" << std::endl;
        for (const auto& entry : Tier3) {
            outTier << entry.first << " " << entry.second << ", ";
        }
        outTier << std::endl;
        
    }

    while (pool1.get_tasks_running() > 0 || pool2.get_tasks_running() > 0 || pool3.get_tasks_running() > 0) {
    // Check if tasks are still running in pools
    // If tasks are still running, continue to wait
    // If not, proceed to end the timer
    }

    // End the timer
    auto end_time = std::chrono::high_resolution_clock::now();

    // Calculate the duration
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

    // Output the duration
    std::cout << "Total requests time: " << duration.count() << " milliseconds\n" << std::endl;

    // counter of read/write
    std::cout << "There are " << num_reads << " reads, and " << num_write << " writes.\n" << std::endl;

    // page hits
    std::cout << "Number of page hits in Tier1: " << page_hit_Tier1 << std::endl;
    std::cout << "Number of page hits in Tier2: " << page_hit_Tier2 << std::endl;
    std::cout << "Number of page hits in Tier3: " << page_hit_Tier3 << std::endl;
    
    
    std::cout << "\nPages in Tier1: " <<std::endl;
    for(const auto& elem : Tier1){
    std::cout << elem.first << ", ";
    }
    std::cout << "\nAnd their temperatures: " <<std::endl;
    for(const auto& elem : Tier1){
    std::cout << elem.second << ", ";
    }

    std::cout << std::endl;
    std::cout << "\nPages in Tier2: " <<std::endl;
    for(const auto& elem : Tier2){
    std::cout << elem.first << ", ";
    }
    std::cout << "\nAnd their temperatures: " <<std::endl;
    for(const auto& elem : Tier2){
    std::cout << elem.second << ", ";
    }

    /*// Write the captured output to the log file
    std::cout << outputStream.str();

    // Restore the original std::cout buffer
    std::cout.rdbuf(originalCoutBuffer);*/
    std::cout.rdbuf(cout_buff); 

    // Close the log file and tier file
    logFile.close();
    outTier.close();

}
