#include <fstream>     // std::ifstream, std::ofstream
#include <random>
#include <iostream>
#include <unordered_set>


int main(){
    int total_num_pages = 1e4;
    int num_reqs = 1e6;
    float r_w_ratio = 0.5;

    // Seed the random number generator
    //std::srand(std::time(0));    
    std::random_device rd;
    std::mt19937 gen(rd());

    // Set up random number generator for page numbers
    // Uniform distribution for pageId
    std::uniform_int_distribution<int> distuni(0, total_num_pages-1);
    
    // x0% pages (randomly picked) with higher frequence (x0% access on x0% pages)
    std::vector<int> probabilities(total_num_pages, 1); // Initialize with 1
    //for (int i : selectedIds){
    for (int i = 0; i < 0.05*total_num_pages; ++i) {
        //probabilities[i] = 81; // Set probabilities to 81 so that 10x81/90x1=9
        //probabilities[i] = 72; // Set probabilities to 72 so that 10x72/90x1=8
        // probabilities[i] = 36; // Set probabilities to 36 so that 20x36/80x1=9
        //probabilities[i] = 32; // Set probabilities to 32 so that 20x32/80x1=8
        probabilities[i] = 171; // Set probabilities to 171 so that 5x171/95x1=9
        //probabilities[i] = 11; // Set probabilities to 11 so that 45x11/55x1=9
    }
    // Shuffle the probabilities to ensure randomness
    std::shuffle(probabilities.begin(), probabilities.end(), gen);
    // Print all pageId where its prob is equal to 
    std::vector<int> indices;
    indices.reserve(0.2*total_num_pages);
    for (int i = 0; i < probabilities.size(); ++i) {
        if (probabilities[i] == 171) {
            indices.push_back(i);
        }
    }
    std::ofstream pageid_file("workload_5hf90_1e4_rw1_1e6.pageids");
    std::cout << "hf pageIds: ";
    for (int index : indices) {
        //std::cout
        pageid_file << index << ", ";
    }
    pageid_file << std::endl;
    // pageid_file.close();
    
    std::cout << std::endl;
    // pageId generator with skewness
    std::discrete_distribution<int> distskew(probabilities.begin(), probabilities.end());

    // // addtional pageId generator for zoomin_zoomout workload
    // // generate subsets from 20hf pageids
    // std::vector<int> pageids_10hf;
    // pageids_10hf.reserve(0.1*total_num_pages);
    // std::vector<int> pageids_5hf;
    // pageids_5hf.reserve(0.05*total_num_pages);

    // std::sample(indices.begin(), indices.end(), std::back_inserter(pageids_10hf),
    //             static_cast<size_t>(0.1*total_num_pages), std::mt19937{std::random_device{}()});

    // std::vector<int> probabilities_10hf(total_num_pages, 1);
    // for (int id : pageids_10hf){
    //     probabilities_10hf[id] = 81; // Set probabilities to 81 so that 10x81/90x1=9
    // }
    // // pageId generator with skewness
    // std::discrete_distribution<int> distskew_10hf(probabilities_10hf.begin(), probabilities_10hf.end());

    // std::sample(pageids_10hf.begin(), pageids_10hf.end(), std::back_inserter(pageids_5hf),
    //             static_cast<size_t>(0.05*total_num_pages), std::mt19937{std::random_device{}()});

    // std::vector<int> probabilities_5hf(total_num_pages, 1);
    // for (int id : pageids_5hf){
    //     probabilities_5hf[id] = 171; // Set probabilities to 171 so that 5x171/95x1=9
    // }
    // // pageId generator with skewness
    // std::discrete_distribution<int> distskew_5hf(probabilities_5hf.begin(), probabilities_5hf.end());


    // // addtional pageId generator for 5hf90+uni+10hf80
    // std::vector<int> probabilities_10hf(total_num_pages, 1);
    // // Create a vector containing all page IDs
    // std::vector<int> all_pages(total_num_pages);
    // std::iota(all_pages.begin(), all_pages.end(), 0);  // Fill with 0, 1, 2, ..., total_num_pages - 1
    // // Remove the indices in 'indices' from 'all_pages'
    // std::unordered_set<int> indices_set(indices.begin(), indices.end());
    // all_pages.erase(
    //     std::remove_if(all_pages.begin(), all_pages.end(),
    //                    [&indices_set](int page_id) {
    //                        return indices_set.find(page_id) != indices_set.end();
    //                    }),
    //     all_pages.end()
    // );
    // // Shuffle the remaining page IDs
    // std::shuffle(all_pages.begin(), all_pages.end(), gen);
    // // Select the first 10% of the remaining page IDs
    // int required_size = static_cast<int>(0.10 * total_num_pages);
    // std::vector<int> pageids_10hf(all_pages.begin(), all_pages.begin() + required_size);
    // // record the 10hf pageIds in .pageids
    // pageid_file << std::endl;
    // for (int index : pageids_10hf) {
    //     pageid_file << index << ", ";
    // }
    // pageid_file.close();
    // // Assign higher probability for 10hf pages
    // for (int id : pageids_10hf){
    //     probabilities_10hf[id] = 72; // Set probabilities to 81 so that 10x72/90x1=8
    // }
    // // pageId generator with skewness
    // std::discrete_distribution<int> distskew_10hf80(probabilities_10hf.begin(), probabilities_10hf.end());


    // Set up random number generator for strings
    std::vector<std::string> actions = {"R ", "W "};
    // Uniform distribution for action
    std::uniform_int_distribution<int> distActions(0, 1);
    // Binary distribution B(p) with udis01 < r_w_ratio
    std::uniform_real_distribution<double> udis01(0.0, 1.0);


    std::ofstream workload_file("workload_5hf90_1e4_rw1_1e6.txt");

    for (int i = 0; i < num_reqs; i++) {
        
        // uniform pageId
        //int pageId = distuni(gen);
        // skew pageId
        int pageId = distskew(gen);

        // uniform action
        std::string randomAction = actions[distActions(gen)];
        // imbalanced R/W
        //std::string randomAction = actions[(udis01(gen) < r_w_ratio) ? 1 : 0];

        workload_file << randomAction << pageId << std::endl;

        // // zoomin_zoomout generator
        // if (i < 2e6){
        //     // skew pageId 20hf
        //     int pageId = distskew(gen);
        //     // uniform action
        //     std::string randomAction = actions[distActions(gen)];

        //     workload_file << randomAction << pageId << std::endl;
        // }
        // else if (i < 4e6){
        //     // skew pageId 10hf
        //     int pageId = distskew_10hf(gen);
        //     // uniform action
        //     std::string randomAction = actions[distActions(gen)];

        //     workload_file << randomAction << pageId << std::endl;
        // }
        // else if (i < 6e6){
        //     // skew pageId 5hf
        //     int pageId = distskew_5hf(gen);
        //     // uniform action
        //     std::string randomAction = actions[distActions(gen)];

        //     workload_file << randomAction << pageId << std::endl;
        // }
        // else if (i < 8e6){
        //     // skew pageId 10hf
        //     int pageId = distskew_10hf(gen);
        //     // uniform action
        //     std::string randomAction = actions[distActions(gen)];

        //     workload_file << randomAction << pageId << std::endl;
        // }
        // else {
        //     // skew pageId 20hf
        //     int pageId = distskew(gen);
        //     // uniform action
        //     std::string randomAction = actions[distActions(gen)];

        //     workload_file << randomAction << pageId << std::endl;
        // }

        // // 5hf90+uni+10hf80 generator (Aug05)
        // if (i < num_reqs/2){
        //     // skew pageId 5hf90
        //     int pageId = distskew(gen);
        //     // uniform action
        //     std::string randomAction = actions[distActions(gen)];
        //     workload_file << randomAction << pageId << std::endl;
        // }
        // // else if (i < 2*num_reqs/3){
        // //     // uniform pageId
        // //     int pageId = distuni(gen);
        // //     // uniform action
        // //     std::string randomAction = actions[distActions(gen)];
        // //     workload_file << randomAction << pageId << std::endl;
        // // }
        // else {
        //     // skew pageId 10hf80
        //     int pageId = distskew_10hf80(gen);
        //     // uniform action
        //     std::string randomAction = actions[distActions(gen)];
        //     workload_file << randomAction << pageId << std::endl;
        // }
    

    }
    // close opened file
    workload_file.close();
}
