// EXD_Tier.h - Header file defining the EXDCache class
#ifndef EXDTier_H
#define EXDTier_H

#include <iostream>
#include <vector>
#include <cmath>
#include <numeric>
#include <unordered_map>
#include <queue>

// Page structure to store page id and last request round and EXD W
struct Page {
    int id;
    int last_request_round;
    double exd_weight;

    bool operator>(const Page& other) const {
        return exd_weight > other.exd_weight;
    }
};

// EXD class
class EXDCache {
public:
    std::unordered_map<int, Page> page_map;
    // std::unordered_set<int> cache;
    std::priority_queue<Page, std::vector<Page>, std::greater<>> min_heap;

    EXDCache(int capacity, double alpha) : capacity(capacity), alpha(alpha) {}

    void requestPage(int page_id, int current_round) {
        if (page_map.find(page_id) != page_map.end()) {
            // Page is already in cache, update EXD weight and recency
            // double alpha = -2e-4;//-1.16*1e-3;
            double exp_dec = std::exp(alpha*(current_round - page_map[page_id].last_request_round));
            page_map[page_id].exd_weight = 1 + page_map[page_id].exd_weight * exp_dec;
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
};

#endif EXDTier_H