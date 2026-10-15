// ReStore_driver.h - Header file defining the disk Read/Write driver
#ifndef RSDRIVER_H
#define RSDRIVER_H

#include <cmath>
#include <chrono>      // std::chrono
#include <cstdlib>     // std::exit
#include <iostream>    // std::cerr
#include <numeric>
#include <string>
#include <thread>      // std::this_thread::sleep_for
#include <vector>

// Include the header files for the thread pool library and its utilities.
#include "BS_thread_pool.hpp"
//#include "BS_thread_pool_utils.hpp"

// Raw block-device I/O for the tiers (device_driver/ReStore_device.c).
#include "ReStore_device.h"


// class for emulating each tier (using thread_pool, and the ReStore_device driver if a device is set)
// - Simulated tier (default, no device path set): each read/write takes read_time / read_time * rw_asym
//   microseconds on one of the tier's num_threads I/O threads.
// - Device tier (device path set, see tierDevicePath()): each read/write is a real 4KB O_DIRECT I/O on the
//   device, so its time is the real device latency.
class Tier {
public:
    int capacity;
    int num_threads;
    int tier_num;
    int fd;
    int read_time;
    int write_time;  // write_time = read_time * rw_asym
    float rw_asym;
    bool simulated;

    // Initialize the thread pool using member initialization
    BS::thread_pool pool;

    Tier(int capacity, int num_threads, int tier_num, int read_time, float rw_asym)
        : capacity(capacity), num_threads(num_threads), tier_num(tier_num), fd(-1),
          read_time(read_time), write_time(read_time * rw_asym), rw_asym(rw_asym),
          simulated(true), pool(num_threads)  // initialize thread pool here
    {
        const char* path = tierDevicePath(tier_num);
        if (path == nullptr || path[0] == '\0') {
            std::cout << "Tier" << tier_num << ": simulated (read " << read_time << " us, write " << write_time
                      << " us, " << num_threads << " I/O threads)" << std::endl;
            return;
        }

        // Open the corresponding device based on the tier number
        simulated = false;
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
        if (this->fd < 0) {
            std::cerr << "Error: cannot use '" << path << "' as the device of Tier" << tier_num << "; aborting." << std::endl;
            std::exit(1);
        }
        std::cout << "Tier" << tier_num << ": device " << path << " (" << num_threads << " I/O threads)" << std::endl;
    }

    void Tier_read(int pageID) {
        std::future<void> my_future = pool.submit_task([pageID, this] {
            if (this->simulated) {
                std::this_thread::sleep_for(std::chrono::microseconds(this->read_time));
            } else {
                // use O_DIRECT to read the page from the disk
                device_read(this->fd, 4096*(pageID%this->capacity), 4096); // using pageID%capacity to create some random offset, not the real case, to be replaced by real offset
            }
        });
        // my_future.wait();  // Optional: Wait for completion if needed
    }

    void Tier_write(int pageID) {
        std::future<void> my_future = pool.submit_task([pageID, this] {
            if (this->simulated) {
                std::this_thread::sleep_for(std::chrono::microseconds(this->write_time));
            } else {
                // use O_DIRECT to write the page to the disk
                device_write(this->fd, 4096*(pageID%this->capacity), 4096); // using pageID%capacity to create some random offset, not the real case, to be replaced by real offset
            }
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