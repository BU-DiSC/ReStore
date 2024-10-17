// TEMP_Tier.h - Header file defining the TEMPCache class
#ifndef TEMPTier_H
#define TEMPTier_H

#include <iostream>
#include <vector>
#include <cmath>
#include <numeric>
#include <unordered_map>
#include <queue>


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

#endif TEMPTier_H