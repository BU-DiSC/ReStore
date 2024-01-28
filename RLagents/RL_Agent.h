// RL_Agent.h - Header file defining the TDAgent class
#ifndef RLAGENT_H
#define RLAGENT_H

#include <vector>
#include <cmath>
#include <numeric>

class TDAgent {
private:
    int n_states;
    std::vector<double> p;
    double a1, a2, a3, b1, b2, b3;
    std::vector<double> alpha;
    double beta, lam;
    std::vector<double> z;

public:
    TDAgent(int n_states, std::vector<double> p_init, double beta, double lam,
            std::vector<double> a_i, std::vector<double> b_i)
        : n_states(n_states), p(p_init), beta(beta), lam(lam) {
        a1 = a_i[0];
        a2 = a_i[1];
        a3 = a_i[2];
        b1 = b_i[0];
        b2 = b_i[1];
        b3 = b_i[2];

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
        double s3 = state[2];

        // Fuzzy Rule-Based function
        double mu_L1 = (1 / (1 + a1 * exp(-b1 * s1)));
        double mu_S1 = 1 - mu_L1;
        double mu_L2 = (1 / (1 + a2 * exp(-b2 * s2)));
        double mu_S2 = 1 - mu_L2;
        double mu_L3 = (1 / (1 + a3 * exp(-b3 * s3)));
        double mu_S3 = 1 - mu_L3;

        std::vector<double> w;
        for (double i : {mu_S1, mu_L1}) {
            for (double j : {mu_S2, mu_L2}) {
                for (double k : {mu_S3, mu_L3}) {
                    w.push_back(i * j * k);
                }
            }
        }

        std::vector<double> phi;
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

    std::vector<double> learn(std::vector<double> state, double reward, std::vector<double> state_next, std::vector<double> phi_list) {
        double s1_not = state[0];
        double s2_not = state[1];
        double s3_not = state[2];

        double s1_up = state_next[0];
        double s2_up = state_next[1];
        double s3_up = state_next[2];

        std::vector<double> phi_n = cost_phi(state).second;
        std::vector<double> phi = cost_phi(state_next).second;

        // Learning rate (N=100)
        for (size_t i = 0; i < alpha.size(); ++i) {
            alpha[i] = (0.1 / (1 + 100 * std::accumulate(phi_list.begin(), phi_list.end() - 1, 0.0)));
        }

        // Eligibility trace
        for (size_t i = 0; i < z.size(); ++i) {
            z[i] = lam * exp(-beta * s3_not) * z[i] + phi_n[i];
        }

        // Update p
        for (size_t i = 0; i < p.size(); ++i) {
            p[i] = p[i] + alpha[i] * (reward + exp(-beta * s3_not) * cost_phi(state_next).first - cost_phi(state).first) * z[i];
        }

        // Return [phi^i] for phi_list
        return p;
    }
};

#endif // RLAGENT_H