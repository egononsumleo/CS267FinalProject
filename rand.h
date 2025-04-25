#pragma once
#include <random>
#include <omp.h>
using namespace std;

mt19937 generator = mt19937(time(NULL)); 
omp_lock_t twister_lock;

bool flip_coin(double p) {
    static random_device rd;
    static mt19937 gen(rd());
    static uniform_real_distribution<> dis(0.0, 1.0);
    return dis(gen) < p;
}

// return true if we accept H1, false if we accept H2
bool sprt(double p0, double p1, double alpha, double beta, double true_p) {
    double A = (1 - beta) / alpha;
    double B = beta / (1 - alpha);
    double logA = log(A);
    double logB = log(B);

    double logLikelihood = 0.0;
    int n = 0;
    int success = 0;

    while (true) {
        bool x = flip_coin(true_p);
        n++;
        success += x;

        // Update log likelihood ratio
        if (x) {
            logLikelihood += log(p1 / p0);
        } else {
            logLikelihood += log((1 - p1) / (1 - p0));
        }

        if (logLikelihood >= logA) {
            return 1;
        } else if (logLikelihood <= logB) {
            return 0;
        }
    }
}
