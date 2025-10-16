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

// Include the header files for the Sibyl driver.
#include "Sibyl_lib.h"


// class for emulating each tier (using thread_pool and Sibyl driver)
class Tier {
public:
    int capacity;
    int num_threads;
    int tier_num;
    int fd;

    // Initialize the thread pool using member initialization
    BS::thread_pool pool;

    Tier(int capacity, int num_threads, int tier_num)
        : capacity(capacity), num_threads(num_threads), 
          tier_num(tier_num), pool(num_threads)  // initialize thread pool here
    {
        // Open the corresponding device based on the tier number
        if (tier_num == 1) {
            this->fd = openFastDevice();
        }
        else if (tier_num == 2) {
            this->fd = openMiddleDevice();
        }
        else if (tier_num == 3) {
            this->fd = openSlowDevice();
        }
        else {
            std::cerr << "Invalid tier number !" << std::endl;
        }
    }

    void Tier_read(int pageID) {
        // use O_DIRECT to read the page from the disk
        std::future<void> my_future = pool.submit_task([pageID, this] {
            // use offset = 0 now, but can be changed with more info
            sibyl_read(this->fd, 4096*(pageID%this->capacity), 4096); // using pageID%capacity to create some random offset, not the real case, to be replaced by real offset
        });
        // my_future.wait();  // Optional: Wait for completion if needed
    }

    void Tier_write(int pageID) {
        // use O_DIRECT to write the page to the disk
        std::future<void> my_future = pool.submit_task([pageID, this] {
            // use offset = 0 now, but can be changed with more info
            sibyl_write(this->fd, 4096*(pageID%this->capacity), 4096); // using pageID%capacity to create some random offset, not the real case, to be replaced by real offset
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