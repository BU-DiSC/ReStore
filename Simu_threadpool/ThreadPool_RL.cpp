#include <algorithm>   // std::min, std::shuffle, std::sort, std::unique, std::min_element
#include <atomic>      // std::atomic
#include <chrono>      // std::chrono
#include <cmath>       // std::abs, std::cos, std::exp, std::llround, std::log, std::round, std::sqrt
#include <cstddef>     // std::size_t
#include <cstdint>     // std::int_fast64_t
#include <cstdlib>     // std::exit, std::quick_exit
#include <ctime>       // std::localtime, std::strftime, std::time, std::time_t, std::tm
#include <fstream>     // std::ofstream
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
#include <sstream>     // std::ostringstream
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

// Include the header file where TDAgent is defined
#include "RL_Agent_2states.h"

// Include the header files for the thread pool library and its utilities.
#include "BS_thread_pool.hpp"
#include "BS_thread_pool_utils.hpp"


// Read/Write from/to Tier
void R_n_W(int pageNum, std::string action, int tier_num,
          int read_time_tier1, float asym_tier1,
          int read_time_tier2, float asym_tier2,
          int read_time_tier3, float asym_tier3
          ){
    // R/W in Tier1
    if (tier_num == 1){
        if (action == "Read"){
            std::cout << "reading page " << pageNum << " from Tier 1 ...\n" << std::endl;
            // Sleep for 0.01 seconds to simulate the read action
            std::this_thread::sleep_for(std::chrono::milliseconds(read_time_tier1));
        }
        else if (action == "Write"){
            std::cout << "writing page " << pageNum << " to Tier 1 ...\n" << std::endl;
            // Sleep for 0.03 seconds to simulate the write action
            // write_time_tier1 = read_time_tier1*asym_tier1
            std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>(read_time_tier1*asym_tier1))); 
        }
        else {
            std::cout << "Unknown action !" << std::endl;
        }
    }
    // R/W in Tier2
    else if (tier_num == 2){
        if (action == "Read"){
            std::cout << "reading page " << pageNum << " from Tier 2 ...\n" << std::endl;
            // Sleep for 0.05 seconds to simulate the read action
            std::this_thread::sleep_for(std::chrono::milliseconds(read_time_tier2));
        }
        else if (action == "Write"){
            std::cout << "writing page " << pageNum << " to Tier 2 ...\n" << std::endl;
            // Sleep for 0.1 seconds to simulate the write action
            // write_time_tier2 = read_time_tier2*asym_tier2
            std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>(read_time_tier2*asym_tier2)));
        }
        else {
            std::cout << "Unknown action !" << std::endl;
        }
    }
    // R/W in Tier3
    else if (tier_num == 3){
        if (action == "Read"){
            std::cout << "reading page " << pageNum << " from Tier 3 ...\n" << std::endl;
            // Sleep for 0.2 seconds to simulate the read action
            std::this_thread::sleep_for(std::chrono::milliseconds(read_time_tier3));
        }
        else if (action == "Write"){
            std::cout << "writing page " << pageNum << " to Tier 3 ...\n" << std::endl;
            // Sleep for 0.3 seconds to simulate the write action
            // write_time_tier3 = read_time_tier3*asym_tier3
            std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>(read_time_tier3*asym_tier3)));
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


// Calculate state variables (<s1,s2>)
std::vector<double> state_rvs(const std::map<int, double>& map, int current_threads, int k_thrd) {

    // s1: average temperature

    // Use std::accumulate to sum the values and std::map's size to get the count
    double sum = std::accumulate(map.begin(), map.end(), 0.0,
                                [](double partialSum, const auto& pair) {
                                    return partialSum + pair.second;
                                });

    double s1 = 0.0;  // Initialize s1 here, if empty Tier then s1 = 0

    if (!map.empty()) {
        s1 = sum / map.size();
    }


    // s2: latency potential, ∝ min⁡(𝐶𝑡_𝑟𝑒𝑎𝑑 − 𝑘_𝑟𝑒𝑎𝑑, 𝐶𝑡_𝑤𝑟𝑖𝑡𝑒 − 𝑘_𝑤𝑟𝑖𝑡𝑒)
    double s2 = std::exp(static_cast<double>(current_threads - k_thrd));  //concurrency threashold for coressponding tier
    std::vector<double> State = {s1, s2}; //, s3);

    return State;
}



int main(
        // what should be put here?
        ){
    // set environmental arguments here
    // read/write time of each tier
    int read_time_tier1 = 10;  float asym_tier1 = 3.0;
    int read_time_tier2 = 50;  float asym_tier2 = 2.0;
    int read_time_tier3 = 200; float asym_tier3 = 1.5;
    // page migration time (shall be replaced by a function act read/write in real),
    int page_migr_time_t2t1 = 100;
    int page_migr_time_t3t2 = 300;
    // capacity of each tier
    int max_capacity_tier1 = 10;
    int max_capacity_tier2 = 30;
    int max_capacity_tier3 = 10;
    // Concurrency (available number of threads)
    int num_threads_tier1 = 3;
    int num_threads_tier2 = 2;
    int num_threads_tier3 = 1;
    // concurrency threashold (k_read/write)
    int k_thrd_tier1 = 4;
    int k_thrd_tier2 = 3;
    int k_thrd_tier3 = 2;
    // total number of pages
    int total_num_pages = 100;
    // total number of requests
    int total_num_reqs = 500;
    // temperature increase/drop hyperparameters
    double temp_incr_alpha = 0.05;
    int temp_drop_thrd = 20;
    double temp_drop_scale = 0.1;
    // RL hyperparameters
    double beta = 0.01;
    double lam = 0.8;
        // b_i = 10 / (avg(s_i)), a_i = exp( (max_s_i-min_s_i) x b_i )
    std::vector<double> b_i_1 = {10/0.7, 10/1.37};
    std::vector<double> a_i_1 = {exp(0.3*10/0.7), exp(2.7*10/1.37)};
    std::vector<double> b_i_2 = {10/0.5, 10/0.51};
    std::vector<double> a_i_2 = {exp(0.4*10/0.5), exp(0.98*10/0.51)};
    std::vector<double> b_i_3 = {10/0.4, 10/0.2};
    std::vector<double> a_i_3 = {exp(0.4*10/0.4), exp(0.35*10/0.2)};


    // Seed the random number generator
    //std::srand(std::time(0));
    
    std::random_device rd;
    std::mt19937 gen(rd());

    // Define thread pools, with different concurrency
    BS::thread_pool pool1(num_threads_tier1);
    BS::thread_pool pool2(num_threads_tier2);
    BS::thread_pool pool3(num_threads_tier3);
    
    // Create a vector for all pools
    std::vector<BS::thread_pool*> pools = {&pool1, &pool2, &pool3};

    /*
    // Create a vector of shared pointers to BS::thread_pool
    std::vector<std::shared_ptr<BS::thread_pool>> pools = {
        std::make_shared<BS::thread_pool>(pool1),
        std::make_shared<BS::thread_pool>(pool2),
        std::make_shared<BS::thread_pool>(pool3)
    };
    */

    // Check the type of pool
    const std::type_info& typeInfo = typeid(pool1);
    std::cout << "Type name of pool: " << typeInfo.name() << std::endl;

    // Declare maps with int keys (page no.) and double values (temperature)
    // For pages in each tier
    std::map<int, double> Tier1;  // max capacity 10
    std::map<int, double> Tier2;  // max capacity 30
    std::map<int, double> Tier3;  // max capacity 100

    // Initiation rule can be changed here.
    // first put all pages in Tier3, with temperature = 0.5
    for (int key = 1; key <= total_num_pages; ++key) {
        Tier3[key] = 0.5;
    }

    // List of access frequency, key = page no., value = request time
    std::map<int, int> list_num_req;
    for (int key = 1; key <= total_num_pages; ++key) {
        list_num_req[key] = 0;
    }

    // List of idle time, how long since last request
    std::map<int, int> list_idle;
    for (int key = 1; key <= total_num_pages; ++key) {
        list_idle[key] = 0;
    }

    // Define RL agents for each tier
    int n_states = 2;

    std::vector<double> p_init(4);  // initial value of p_i 2^n_states
    std::uniform_real_distribution<double> dis(0.0, 1.0); // Define the range [0, 1)
    // initial values for p
    for (int i = 0; i < 4; ++i) {
        p_init[i] = 0.0; // = dis(gen); // Assign a random value to the vector at index i
    }
    std::cout << "Initial p values: ";
    for (double p_value : p_init) {
        std::cout << p_value << " ";
    }
    std::cout << std::endl;

    TDAgent agent1(n_states, p_init, beta, lam, a_i_1, b_i_1);
    TDAgent agent2(n_states, p_init, beta, lam, a_i_2, b_i_2);
    TDAgent agent3(n_states, p_init, beta, lam, a_i_3, b_i_3);

    // if use decreasing learning rate, collect previous phi values:
    std::vector<std::vector<double>> phi_list_t1 = {{0.0, 0.0, 0.0, 0.0}};
    std::vector<std::vector<double>> phi_list_t2 = {{0.0, 0.0, 0.0, 0.0}};
    std::vector<std::vector<double>> phi_list_t3 = {{0.0, 0.0, 0.0, 0.0}};
    
    // if use dynamic discounting factor, define tau_n
    /*
    int tau_t1 = 0;
    int tau_t2 = 0;
    int tau_t3 = 0;
    */


    // Generate random read/write request for random page
    // Set up random number generator for page numbers
    std::uniform_int_distribution<int> distNumbers(1, total_num_pages);
    // random number for left right (no need if RLagents are defined)
    std::uniform_real_distribution<double> distrand(-1.0, 1.0);


    // Set up random number generator for strings
    std::vector<std::string> actions = {"Read", "Write"};
    std::uniform_int_distribution<int> distActions(0, 1);

    // Vector to store the table
    std::vector<std::pair<int, std::string>> ReqTable;

    // Generate the request table
    for (int i = 0; i < total_num_reqs; ++i) {  // Generating 500 requests for example
        int randomNumber = distNumbers(gen);
        std::string randomAction = actions[distActions(gen)];
        ReqTable.push_back(std::make_pair(randomNumber, randomAction));
    }

    // Begin loop of requests
    for (int i = 0; i < total_num_reqs; ++i){

        // Page number being requested
        int n_page = ReqTable[i].first;
        // Read/write
        std::string action = ReqTable[i].second;

        std::cout << "Begin request " << i+1 << ", " << action << " page " << n_page << "\n" << std::endl;

        // num of request +1
        list_num_req[n_page]++;

        // idle time since last request +1
        for (auto& pair : list_idle) {
            pair.second += 1;
        }
        // except for the current requested page
        list_idle[n_page] = 0;

        // Locate current Tier the page is in
        auto it1 = Tier1.find(n_page);
        auto it2 = Tier2.find(n_page);
        auto it3 = Tier3.find(n_page);

        if (it1 != Tier1.end()) {
            std::cout << "page " << n_page << " currently in Tier1\n" << std::endl;
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
                });
            // my_future.wait_for(std::chrono::milliseconds(1));
            std::cout << action << " done.\n"<< std::endl;

            // Update its temperature
            Tier1[n_page] = temp_increase(n_page, list_num_req, temp_incr_alpha);

            // no need for migration if page already in Tier1

        }
        else if (it2 != Tier2.end()) {
            std::cout << "page " << n_page << " currently in Tier2\n" << std::endl;
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
                });
            // my_future.wait_for(std::chrono::milliseconds(1));
            std::cout << action << " done.\n"<< std::endl;

            // Update its temperature
            Tier2[n_page] = temp_increase(n_page, list_num_req, temp_incr_alpha);

            // Decide migration
            
            // Calculate state variables
            // before movement
            int current_threads = selectedPool.get_thread_count(); // !!modify to the correct one!!  // set a counter
            std::vector<double> state_t2_be = state_rvs(Tier2,current_threads,k_thrd_tier2);
            std::vector<double> state_t1_be = state_rvs(Tier1,current_threads,k_thrd_tier1);
            // after movement
            std::map<int, double> Tier2_af = PageMigrate(Tier2,Tier1,n_page).first;
            std::map<int, double> Tier1_af = PageMigrate(Tier2,Tier1,n_page).second;
            int post_threads = current_threads + 1;
            std::vector<double> state_t2_af = state_rvs(Tier2_af,post_threads,k_thrd_tier2);
            std::vector<double> state_t1_af = state_rvs(Tier1_af,post_threads,k_thrd_tier1);

            ///* uncomment when RLagents are defined
            // c_up, c_not
            auto [cost_tier2_be, phi_t2] = agent2.cost_phi(state_t2_be);
            auto [cost_tier1_be, phi_t1] = agent1.cost_phi(state_t1_be);
            phi_list_t2.push_back(phi_t2);
            phi_list_t1.push_back(phi_t1);
            double cost_tier2_af = agent2.cost_phi(state_t2_af).first;
            double cost_tier1_af = agent1.cost_phi(state_t1_af).first;
            // left, right of condition
            double left  = cost_tier1_be * state_t1_be[0] + cost_tier2_be * state_t2_be[0];
            double right = cost_tier1_af * state_t1_af[0] + cost_tier2_af * state_t2_af[0];
            //*/

            //double left = 0.0;
            //double right = left + distrand(gen);
            // if condition, then move
            std::cout << "left = " << left << ", right = " << right << std::endl;
            if (left <= right){
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
                }
                const std::future<void> my_future = selectedPool.submit_task(
                    [page_migr_time_t2t1]{
                    // page moving time, or replace with R_n_W func
                     // sleep for 0.5s to simulate page movement from tier2 to tier1
                    std::this_thread::sleep_for(std::chrono::milliseconds(page_migr_time_t2t1));  // 500ms is a hyperparameter
                    std::cout << "Page migration done.\n" << std::endl;
                    });
                // Update RL agents
                double reward = 1/0.500;  // current using the respond time as the reward, need to think more about how to define the reward function.
                std::vector<double> p_t1 = agent1.learn(state_t1_be, state_t1_af, reward, phi_list_t1, 1);
                std::vector<double> p_t2 = agent2.learn(state_t2_be, state_t2_af, reward, phi_list_t2, 1);
            }
            else{
                std::cout << "Page " << n_page << " should remain in Tier2.\n"<< std::endl;
            }
        }
        else if (it3 != Tier3.end()) {
            std::cout << "page " << n_page << " currently in Tier3\n" << std::endl;
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
                });
            my_future.wait_for(std::chrono::milliseconds(1));
            std::cout << action << " done.\n"<< std::endl;

            // Update its temperature
            Tier3[n_page] = temp_increase(n_page, list_num_req, temp_incr_alpha);

            // Decide migration
            
            // Calculate state variables
            // before movement
            int current_threads = selectedPool.get_thread_count(); // !!modify to the correct one!!  // set a counter
            std::vector<double> state_t3_be = state_rvs(Tier3,current_threads,k_thrd_tier3);
            std::vector<double> state_t2_be = state_rvs(Tier2,current_threads,k_thrd_tier2);
            // after movement
            std::map<int, double> Tier3_af = PageMigrate(Tier3,Tier2,n_page).first;
            std::map<int, double> Tier2_af = PageMigrate(Tier3,Tier2,n_page).second;
            int post_threads = current_threads + 1;
            std::vector<double> state_t3_af = state_rvs(Tier3_af,post_threads,k_thrd_tier3);
            std::vector<double> state_t2_af = state_rvs(Tier2_af,post_threads,k_thrd_tier2);

            ///* uncomment when RLagents are defined
            // c_up, c_not
            auto [cost_tier3_be, phi_t3] = agent3.cost_phi(state_t3_be);
            auto [cost_tier2_be, phi_t2] = agent2.cost_phi(state_t2_be);
            phi_list_t3.push_back(phi_t3);
            phi_list_t2.push_back(phi_t2);
            double cost_tier3_af = agent3.cost_phi(state_t3_af).first;
            double cost_tier2_af = agent2.cost_phi(state_t2_af).first;
            // left, right of condition
            double left  = cost_tier2_be * state_t2_be[0] + cost_tier3_be * state_t3_be[0];
            double right = cost_tier2_af * state_t2_af[0] + cost_tier3_af * state_t3_af[0];
            //*/

            //double left = 0.0;
            //double right = left + distrand(gen);
            // if condition, then move
            std::cout << "left = " << left << ", right = " << right << std::endl;
            if (left <= right){
                std::cout << "Page " << n_page << " should be moved from Tier3 to Tier2\n"<< std::endl;
                auto tier_pair = PageMigrate(Tier3,Tier2,n_page);
                Tier3 = tier_pair.first;
                Tier2 = tier_pair.second;
                // if Tier 1 is full
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
                }
                const std::future<void> my_future = selectedPool.submit_task(
                    [page_migr_time_t3t2]{
                    // page moving time, or replace with R_n_W func
                     // sleep for 1.5s to simulate page movement from tier2 to tier1
                    std::this_thread::sleep_for(std::chrono::milliseconds(page_migr_time_t3t2)); // 1500ms is a hyperparameter
                    std::cout << "Page migration done.\n" << std::endl;
                    });
                // Update RL agents
                double reward = 1/1.500;  // current using the respond time as the reward, need to think more about how to define the reward function.
                std::vector<double> p_t2 = agent2.learn(state_t2_be, state_t2_af, reward, phi_list_t2, 1);
                std::vector<double> p_t3 = agent3.learn(state_t3_be, state_t3_af, reward, phi_list_t3, 1);
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

        //std::cout << "Running on pool " << rv << ", results:" << my_future.get() << '\n';
        std::cout << "Finish request " << i+1 << "\n" << std::endl;
        
    }

std::cout << std::endl;
std::cout << "Pages in Tier1: " <<std::endl;
for(const auto& elem : Tier1){
   std::cout << elem.first << ", ";
}
std::cout << "\nAnd their temperatures: " <<std::endl;
for(const auto& elem : Tier1){
   std::cout << elem.second << ", ";
}

std::cout << "\nPages in Tier2: " <<std::endl;
for(const auto& elem : Tier2){
   std::cout << elem.first << ", ";
}


}
