#include <iostream>
#include <vector>
#include <cmath>
#include <numeric>
#include <random>
#include "RL_Agent.h" // Include the header file where TDAgent is defined

int main() {
    // Example input values
    int n_states = 3;

    std::vector<double> p_init(8);  // initial value of p_i
    std::random_device rd; // Obtain a random seed from hardware
    std::mt19937 gen(rd()); // Mersenne Twister random number engine
    std::uniform_real_distribution<double> dis(0.0, 1.0); // Define the range [0, 1)
    // Generate 8 random values for p_init
    for (int i = 0; i < 8; ++i) {
        p_init[i] = 0.0; // = dis(gen); // Assign a random value to the vector at index i
    }
    std::cout << "Initial p values: ";
    for (double p_value : p_init) {
        std::cout << p_value << " ";
    }
    std::cout << std::endl;

    double beta = 0.01;

    double lam = 0.1;

    std::vector<double> b_i_1 = {10/0.6, 10/3, 10/1};
    std::vector<double> a_i_1 = {exp(0.3*10/0.5), exp(2*10/3), exp(2*10/1)};
    std::vector<double> b_i_2 = {10/0.4, 10/4, 10/2};
    std::vector<double> a_i_2 = {exp(0.6*10/0.4), exp(3*10/4), exp(2*10/2)};

    TDAgent agent1(n_states, p_init, beta, lam, a_i_1, b_i_1);
    TDAgent agent2(n_states, p_init, beta, lam, a_i_2, b_i_2);

    // Test cost_phi function
    std::vector<double> state = {0.5, 0.5, 1.0};

    auto cost_phi_result1 = agent1.cost_phi(state);
    auto cost_phi_result2 = agent2.cost_phi(state);
    //double cost = cost_phi_result.first;
    //std::vector<double> phi_values = cost_phi_result.second;

    std::cout << "Cost of tier1: " << cost_phi_result1.first << std::endl;
    std::cout << "Phi of tier1: ";
    for (double phi_value : cost_phi_result1.second) {
        std::cout << phi_value << " ";
    }
    std::cout << std::endl;

    std::cout << "Cost of tier2: " << cost_phi_result2.first << std::endl;
    std::cout << "Phi of tier2: ";
    for (double phi_value : cost_phi_result2.second) {
        std::cout << phi_value << " ";
    }
    std::cout << std::endl;

    // Test learn function (simplified example)
    std::vector<double> state_next = {0.6, 1.3, 2.0};
    std::vector<double> phi_values = cost_phi_result1.second;

    std::vector<double> result_p = agent1.learn(state, 0.5, state_next, phi_values);

    std::cout << "Resulting p after learning: ";
    for (double p_value : result_p) {
        std::cout << p_value << " ";
    }
    std::cout << std::endl;

    // Output new costs and phi
    auto cost_phi_result1new = agent1.cost_phi(state_next);
    auto cost_phi_result2new = agent2.cost_phi(state_next);
    //double cost = cost_phi_result.first;
    //std::vector<double> phi_values = cost_phi_result.second;

    std::cout << "New cost of tier1: " << cost_phi_result1new.first << std::endl;
    std::cout << "New Phi of tier1: ";
    for (double phi_value : cost_phi_result1new.second) {
        std::cout << phi_value << " ";
    }
    std::cout << std::endl;

    std::cout << "New cost of tier2: " << cost_phi_result2new.first << std::endl;
    std::cout << "New Phi of tier2: ";
    for (double phi_value : cost_phi_result2new.second) {
        std::cout << phi_value << " ";
    }
    std::cout << std::endl;

    return 0;
}

