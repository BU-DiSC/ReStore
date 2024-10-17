// LFU_Tier.h - Header file defining the LFUCache class
#ifndef LFUTier_H
#define LFUTier_H

#include <iostream>
#include <vector>
#include <cmath>
#include <numeric>
#include <unordered_map>
#include <queue>

// Page structure
struct Page {
    int id;
    int frequency;

    bool operator>(const Page& other) const {
        return frequency > other.frequency;
    }
};

// LFU class
class LFUCache {
public:
    std::unordered_map<int, Page> page_map;
    // std::unordered_set<int> cache;
    std::priority_queue<Page, std::vector<Page>, std::greater<>> min_heap;

    LFUCache(int capacity) : capacity(capacity) {}

    // void preloadPages(const std::unordered_set<int>& pages) {
    //     for (int page_id : pages) {
    //         if (cache.size() >= capacity) {
    //             // If cache is full during preload, we stop adding more pages.
    //             break;
    //         }
    //         if (page_map.find(page_id) == page_map.end()) {
    //             Page new_page = {page_id, 0};
    //             page_map[page_id] = new_page;
    //             cache.insert(page_id);
    //             min_heap.push(new_page);
    //         }
    //     }
    // }

    void requestPage(int page_id) {
        if (page_map.find(page_id) != page_map.end()) {
            // Page is already in cache, increment frequency
            page_map[page_id].frequency++;
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

#endif LFUTier_H