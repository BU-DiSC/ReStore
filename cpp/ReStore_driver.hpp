// ReStore_driver.h - Header file defining the disk Read/Write driver
#ifndef RSDRIVER_H
#define RSDRIVER_H

#include <cmath>
#include <chrono>      // std::chrono
#include <iostream>    // std::cerr
#include <numeric>
#include <vector>

// Include the header files for the thread pool library and its utilities.
#include "BS_thread_pool.hpp"
//#include "BS_thread_pool_utils.hpp"


// class for emulating each tier (currently using thread_pool)
class Tier {
public:
    int capacity;
    int num_threads;
    int read_time;
    int write_time;  // write_time = read_time * rw_asym
    float rw_asym;

    // Initialize the thread pool using member initialization
    BS::thread_pool pool;

    Tier(int capacity, int num_threads, int read_time, float rw_asym)
        : capacity(capacity), num_threads(num_threads), 
          read_time(read_time), rw_asym(rw_asym), 
          pool(num_threads)  // initialize pool here
    {
        write_time = read_time * rw_asym;
    }

    void Tier_read(int pageID) {
        // (to do) replace by real disk read
        std::future<void> my_future = pool.submit_task([pageID, this] {
            std::this_thread::sleep_for(std::chrono::microseconds(this->read_time));
        });
        // my_future.wait();  // Optional: Wait for completion if needed
    }

    void Tier_write(int pageID) {
        // (to do) replace by real disk write
        std::future<void> my_future = pool.submit_task([pageID, this] {
            std::this_thread::sleep_for(std::chrono::microseconds(this->write_time));
        });
        // my_future.wait();  // Optional: Wait for completion if needed
    }

    void exec(int pageID, std::string action) {
        if (action == "Read"){
            // std::cout << "reading page " << pageID << "\n";
            Tier_read(pageID);
        }
        else if (action == "Write"){
            // std::cout << "writing page " << pageNum << "\n";
            Tier_write(pageID);
        }
        else {
            std::cerr << "Unknown action !" << std::endl;
        }
    }
};

// Function to migrate page between Tiers
// void Migrate_Page(int page_ID,
//                   Tier& from_Tier, Tier& to_Tier,
//                   Tier_Meta& from_Tier_Meta, Tier_Meta& to_Tier_Meta) {}


#endif // RSDRIVER_H