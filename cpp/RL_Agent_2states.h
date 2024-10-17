// RL_Agent_2states.h - Header file defining the TDAgent class
#ifndef RLAGENT_H
#define RLAGENT_H

#include <vector>
#include <cmath>
#include <numeric>

class TDAgent {
private:
    int n_states;
    // std::vector<double> p;
    // double a1, a2, b1, b2;
    std::vector<double> alpha;
    double beta, lam;
    std::vector<double> z;

public:
    std::vector<double> p;
    double a1, a2, b1, b2;

    TDAgent(int n_states, std::vector<double> p_init, double beta, double lam,
            std::vector<double> a_i, std::vector<double> b_i)
        : n_states(n_states), p(p_init), beta(beta), lam(lam) {
        a1 = a_i[0];
        a2 = a_i[1];
        b1 = b_i[0];
        b2 = b_i[1];

        alpha.resize(p_init.size(), 0);
        z.resize(p_init.size(), 0);
    }

    int act(int state) {
        // Random - Replace with C++ random number generation logic
        return rand() % n_states;
    }

    std::pair<double, std::vector<double>> cost_phi(const std::vector<double>& state) {
        double s1 = state[0];
        double s2 = state[1];

        // Fuzzy Rule-Based function
        double mu_L1 = (1 / (1 + exp( -b1 * s1 + 1e10*log(a1) )));  // use exp(log()) to prevent overflow
        double mu_S1 = 1 - mu_L1;
        double mu_L2 = (1 / (1 + a2 * exp(-b2 * s2)));
        double mu_S2 = 1 - mu_L2;

        std::vector<double> w = {mu_L1, mu_S1, mu_L2, mu_S2};
        // for (double i : {mu_S1, mu_L1}) {
        //     for (double j : {mu_S2, mu_L2}) {
        //         w.push_back(i * j);
        //     }
        // }
        // std::cout << "w = ";
        // for (double value : w) {
        //     std::cout << value << ", ";
        // }
        // std::cout << std::endl;

        std::vector<double> phi;
        phi.reserve(w.size()); // Preallocate memory to avoid multiple allocations
        double sum_w = std::accumulate(w.begin(), w.end(), 0.0);
        for (double i : w) {
            double phi_i = i / sum_w;
            phi.push_back(phi_i);
        }

        double Cost = 0;
        for (size_t i = 0; i < p.size(); ++i) {
            Cost += p[i] * phi[i];
        }

        return std::make_pair(Cost, phi);
    }

    std::vector<double> learn(std::vector<double> state, 
                              std::vector<double> state_next,
                              std::vector<double> last_phi,
                              double cost_be,
                              double cost_af,
                              double reward, 
                              std::vector<double> sum_phi,
                              int tau_n){
        double s1_not = state[0];
        double s2_not = state[1];

        double s1_up = state_next[0];
        double s2_up = state_next[1];

        // Learning rate (N=100)
        for (size_t i = 0; i < alpha.size(); ++i) {
            // double sum_phi_i = std::accumulate(phi_list.begin(), phi_list.end()-1, 0.0,
            //                     [i](double partial_sum, const std::vector<double>& row){
            //                     return partial_sum + row[i];
            //                     });
            // alpha[i] = (0.1 / (1 + 2 * sum_phi[i])); //for 1e5 use 10*, can be set as a parameter
            alpha[i] = 1e-4;
        }

        // Eligibility trace
        for (size_t i = 0; i < z.size(); ++i) {
            z[i] = lam * exp(-beta * tau_n) * z[i] + last_phi[i];
        }

        // Update p
        std::vector<double> delta_p(p.size());
        for (size_t i = 0; i < p.size(); ++i) {
            delta_p[i] = alpha[i] * (reward + exp(-beta * tau_n) * cost_af - cost_be) * z[i];
            // std::cout << "updating p[" << i << "]" << std::endl;
            // std::cout << "alpha[i]: " << alpha[i] << std::endl;
            // std::cout << "reward: " << reward << std::endl;
            // std::cout << "discount factor: " << exp(-beta * tau_n) << std::endl;
            // std::cout << "cost(state_next): " << cost_phi(state_next).first << "   ";
            // std::cout << "cost(state_curr): " << cost_phi(state).first << std::endl;
            // std::cout << "elg trace: " << z[i] << std::endl;
        }
        for (size_t i = 0; i < p.size(); ++i) {
            p[i] = p[i] + delta_p[i];
        }

        // Return [phi^i] for phi_list
        return p;
    }

    // Update value of a_i, b_i
    void update_a_b(std::vector<double> a_i, std::vector<double> b_i){
        a1 = a_i[0];
        a2 = a_i[1];
        b1 = b_i[0];
        b2 = b_i[1];
    }
};

#endif // RLAGENT_H