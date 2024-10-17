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
#include <set>
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
void R_n_W(int pageNum, std::string action,
          int read_time_tier, float asym_tier
          ){
    // R/W time
    if (action == "Read"){
        // Sleep for 0.01 seconds to simulate the read action
        std::this_thread::sleep_for(std::chrono::microseconds(read_time_tier));
    }
    else if (action == "Write"){
        // Sleep for 0.03 seconds to simulate the write action
        // write_time_tier1 = read_time_tier1*asym_tier1
        std::this_thread::sleep_for(std::chrono::microseconds(static_cast<int>(read_time_tier*asym_tier)));
    }
    else {
        std::cerr << "Unknown action !" << std::endl;
    }
}
    
//     // R/W in Tier2
//     else if (tier_num == 2){
//         if (action == "Read"){
//             // Sleep for 0.05 seconds to simulate the read action
//             std::this_thread::sleep_for(std::chrono::microseconds(read_time_tier2));
//         }
//         else if (action == "Write"){
//             // Sleep for 0.1 seconds to simulate the write action
//             // write_time_tier2 = read_time_tier2*asym_tier2
//             std::this_thread::sleep_for(std::chrono::microseconds(static_cast<int>(read_time_tier2*asym_tier2)));
//         }
//         else {
//             std::cerr << "Unknown action !" << std::endl;
//         }
//     }
//     // R/W in Tier3
//     else if (tier_num == 3){
//         if (action == "Read"){
//             // Sleep for 0.2 seconds to simulate the read action
//             std::this_thread::sleep_for(std::chrono::microseconds(read_time_tier3));
//         }
//         else if (action == "Write"){
//             // Sleep for 0.3 seconds to simulate the write action
//             // write_time_tier3 = read_time_tier3*asym_tier3
//             std::this_thread::sleep_for(std::chrono::microseconds(static_cast<int>(read_time_tier3*asym_tier3)));
//         }
//         else {
//             std::cerr << "Unknown action ! \n" << std::endl;
//         }    
//     }
// }

// Page movement between tiers
// void movePage(int page_id, std::unordered_set<int>& from_tier, std::unordered_set<int>& to_tier) {
//     from_tier.erase(page_id);
//     to_tier.insert(page_id);
// }

// Helper function to trim whitespace from both ends of a string
std::string trim(const std::string &str) {
    size_t start = str.find_first_not_of(" \t\n\r\f\v");
    size_t end = str.find_last_not_of(" \t\n\r\f\v");
    return (start == std::string::npos) ? "" : str.substr(start, end - start + 1);
}


int main(
    int argc, char *argv[]
    ){
    // set environmental arguments here
    // read/write time of each tier
    int read_time_tier1 = 30;   double asym_tier1 = 1.5;
    int read_time_tier2 = 200;  double asym_tier2 = 2.0;
    int read_time_tier3 = 500;  double asym_tier3 = 4.0;
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
        } else if (arg.find("-num_threads_tier3=") == 0) {
            num_threads_tier3 = static_cast<int>(std::stod(arg.substr(19))); // Extracts and converts the substring after "-num_threads_tier3=" to an integer
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
    std::string log_path = folder_name + "/output_" + workload + "_static.log";
    std::ofstream logFile(log_path);

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
    // std::ofstream outTier("Tier123_5hf90_rw1_10000_static.txt");

    // Define thread pools, with different concurrency
    BS::thread_pool pool1(num_threads_tier1);
    BS::thread_pool pool2(num_threads_tier2);
    BS::thread_pool pool3(num_threads_tier3);

    // Declare maps with int keys (page no.) and double values (temperature)
    // For pages in each tier
    std::unordered_set<int> Tier1;
    std::unordered_set<int> Tier2;
    std::unordered_set<int> Tier3;

    // Initiation rule can be changed here.
    // static initial: fill each tier with the hf pageIds and keep them static
    // Define all keys
    std::vector<int> allKeys;
    if (workload == "YCSB" || workload == "TPCC" || workload == "TPCE") {
        // initial policy for workload_YCSB: fill with specific page ids
        std::string page_ids_file = std::string("workload_") + workload + ".allpageids";  // Order file to read from
        std::ifstream IDs(page_ids_file);
        // std::vector<int> allKeys;
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
    }
    // usual cases
    else {
        // Resize the vector to the appropriate size
        allKeys.resize(total_num_pages);
        // Fill allKeys with values from 0 to total_num_pages - 1
        std::iota(allKeys.begin(), allKeys.end(), 0);
    }
    //double check if num of pageids are the same
    if (allKeys.size() != total_num_pages){
        std::cerr << "allKeys length doesn't match total_num_pages !" << std::endl;
        return 1;
    }

    // Static placement
    // Insert first max_capacity_tier1 ids into Tier1
    for (size_t i = 0; i < max_capacity_tier1 && i < allKeys.size(); ++i) {
        int key = allKeys[i];
        Tier1.insert(key);
    }
    // Insert next max_capacity_tier2 ids into Tier2
    for (size_t i = max_capacity_tier1; i < max_capacity_tier1 + max_capacity_tier2 && i < allKeys.size(); ++i) {
        int key = allKeys[i];
        Tier2.insert(key);
    }
    // Insert remaining ids into Tier3
    for (size_t i = max_capacity_tier1 + max_capacity_tier2; i < allKeys.size(); ++i) {
        int key = allKeys[i];
        Tier3.insert(key);
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

        // auto end_time_i_find = std::chrono::high_resolution_clock::now();

        if (it1 != Tier1.end()) {
            // std::cout << "page " << n_page << " currently in Tier1\n" << std::endl;
            ++page_hit_Tier1;

            // Execute action
            int tier_num = 1;
            std::future<void> my_future = pool1.submit_task(
                [n_page, action,
                read_time_tier1, asym_tier1]{
                R_n_W(n_page, action,
                      read_time_tier1, asym_tier1);
                //return curr_thrds_tier1-1;  //after finishing the action, current threads -= 1
                });
            //my_future.wait();
            // my_future.wait_for(std::chrono::microseconds(1));
            //std::cout << action << " done.\n"<< std::endl;

            // Update its temperature
            // ptm.requestPage(n_page);

        }
        else if (it2 != Tier2.end()) {
            // std::cout << "page " << n_page << " currently in Tier2\n" << std::endl;
            ++page_hit_Tier2;

            // Execute action
            int tier_num = 2;
            std::future<void> my_future = pool2.submit_task(
                [n_page, action,
                read_time_tier2, asym_tier2]{
                R_n_W(n_page, action,
                      read_time_tier2, asym_tier2);
                //return curr_thrds_tier2-1;  //after finishing the action, current threads -= 1
                });
            //my_future.wait();
            //std::cout << action << " done.\n"<< std::endl;

            // Update its temperature
            // ptm.requestPage(n_page);

        }
        else if (it3 != Tier3.end()) {
            // std::cout << "page " << n_page << " currently in Tier3\n" << std::endl;
            ++page_hit_Tier3;

            // Execute action
            int tier_num = 3;
            std::future<void> my_future = pool3.submit_task(
                [n_page, action,
                read_time_tier3, asym_tier3]{
                R_n_W(n_page, action,
                      read_time_tier3, asym_tier3);
                //return curr_thrds_tier3-1;  //after finishing the action, current threads -= 1
                });
            //my_future.wait();
            //std::cout << action << " done.\n"<< std::endl;

            // Update its temperature
            // ptm.requestPage(n_page);

        }
        else {
            std::cerr << "page not exist in Tiers!\n" << std::endl;
        }

        //std::cout << "Running on pool " << rv << ", results:" << my_future.get() << '\n';
        // std::cout << "Finish request " << i+1 << "\n" << std::endl;
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


    // std::cout << "Pages in Tier1 and their temperature: ";
    // for (int page_id : Tier1) {
    //     std::cout << page_id << " " << ptm.getTemperature(page_id) << ", ";
    // }
    // std::cout << std::endl;

    // std::cout << "Pages in Tier2 and their temperature: ";
    // for (int page_id : Tier2) {
    //     std::cout << page_id << " " << ptm.getTemperature(page_id) << ", ";
    // }
    // std::cout << std::endl;

    // std::cout << "Pages in Tier3 and their temperature: ";
    // for (int page_id : Tier3) {
    //     std::cout << page_id << " " << ptm.getTemperature(page_id) << ", ";
    // }
    // std::cout << std::endl;
    

    /*// Write the captured output to the log file
    std::cout << outputStream.str();

    // Restore the original std::cout buffer
    std::cout.rdbuf(originalCoutBuffer);*/
    std::cout.rdbuf(cout_buff);

    // Close the log file and tier file
    logFile.close();
    file.close();

}
