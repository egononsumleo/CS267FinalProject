#ifndef RBS_H
#define RBS_H
#include <algorithm>
#include <utility>
#include <cmath>
#include <cstdio>
#include <vector>
#include <random>
#include "rand.h"
#include "mpreal.h"
using namespace std;

typedef mpfr::mpreal F;
typedef long long I;

// note SegTree could be more optimized but it is not the bottleneck 
struct SegTree {
	const F EPS = 1e-10;
    int l, r;
    SegTree * left = nullptr; 
    SegTree * right = nullptr;
    F value, delayed;

    F max_value;
    int arg_max;
    
    SegTree(int _l, int _r, F _value) : l(_l), r(_r), value(_value), delayed(mpfr::mpreal(1)) {
        max_value = mpfr::mpreal(1)/(r - l + 1);
        arg_max = _l;
        left = right = nullptr;
    } 

    F query(int ql, int qr){
        if(ql <= l && r <= qr){
            return value;
        }
        else if(qr < l || ql > r){
            return 0;
        }
        else{
            push();
            auto result = mpfr::mpreal(0);
            if(left != nullptr){
                result += left->query(ql,qr);
            }
            if(right != nullptr){
                result += right->query(ql,qr);
            }
            return result;
        }
    }

    F get_max_value(){
        return max_value;
    }

    int get_max_ans(){
        return arg_max;
    }

    int mid(){
        return (l + r)/2;
    }

    void apply(F x){
        value *= x;
        delayed *= x;
        max_value *= x;
    }

    void push(){
        split();
        if(delayed != mpfr::mpreal(1)){
            if(left != nullptr){
                left->apply(delayed);
            }
            if(right != nullptr){
                right->apply(delayed);
            }
            delayed = mpfr::mpreal(1);
        }
    }

    void split(){
        if(left == nullptr && right == nullptr && l != r){
            delayed = mpfr::mpreal(1);
            int m = mid();
            auto ratio = mpfr::mpreal(m - l + 1)/mpfr::mpreal(r - l + 1);

            left = new SegTree(l, m, value * ratio);
            right = new SegTree(m + 1, r, value * (1 - ratio));
        }
    }

    // returns the smallest i such that the cumulative sum of values(1) through values(i) >= p
    int find(F p){
        if(l == r) return l; 

        push();

        if(left->value >= p){
            return left->find(p);
        }

        return right->find(p - left->value);
    }

    void build(){
        if(right != nullptr && left != nullptr){
            value = right->value + left->value;
            if(right->value > left->value) {
                max_value = right->value;
                arg_max = right->arg_max;
            }
            else {
                max_value = left->value;
                arg_max = left->arg_max;
            }
        }
    }

    void mult(int ql, int qr, F factor){
        push();
        if(ql <= l && r <= qr){
            apply(factor);
        }
        else if(qr < l || ql > r){
            return;
        }
        else{
            if(left != nullptr){
                left->mult(ql,qr,factor);
            }
            if(right != nullptr){
                right->mult(ql,qr,factor);
            }
            build();
        }
    }


};

// selects the pivot
struct Selector {
	// return the Quantile of the pivot
	virtual vector<F> select() = 0;
};

struct MedianSelector : public Selector {
	 vector<F> select() override {
	 	return {.5};	
	 }
};

struct Problem {
	virtual int f(F x) = 0;
	virtual pair<int, int> range() const = 0;
	virtual int answer() = 0;
};


struct StandardInstance : public Problem {
	uniform_real_distribution<double> distribution = uniform_real_distribution<double>(0,1);
	int N;
	int _answer;
	double eps;
	StandardInstance(int N, int _answer, double eps) : N(N), _answer(_answer), eps(eps){
	}

	int f(F x) override {
        omp_set_lock(&twister_lock);
        double rand = distribution(generator);
        omp_unset_lock(&twister_lock);
		if(x <= _answer){ // TODO CHANGE BACK
			return rand <= .5 - eps ? 1 : 0;
		}
		else{
			return rand <= .5 + eps ? 1 : 0;
		}
	}
	pair<int,int> range() const override {
		return make_pair(0, N);
	}
	int answer(){
		return _answer;
	}
};


bool generate_bernoulli(double p) {
    static random_device rd;
    static mt19937 gen(rd());
    static uniform_real_distribution<> dis(0.0, 1.0);
    return dis(gen) < p;
}

// selects the answer
struct Answerer {
	virtual int answer(const vector<F> &pivots, SegTree * tree, Problem & problem) const = 0;
};

struct MedianAnswerer : public Answerer {
	int answer(const vector<F> &pivots, SegTree * tree, Problem & problem) const override {
		return tree->find(.5);
	}
};

struct Solver {
    int solve(Problem & problem, vector<F> pivots, const Answerer &answerer, F eps, F delta, int &iterations){

		int upper_bound = problem.range().second;

        // the x'th interval corresponds to interval between coins x and x + 1
		SegTree * tree = new SegTree(0, upper_bound, mpfr::mpreal(1));
        std::cout << " Tree initial max_value " << tree->get_max_value() << '\n';

        F tau = mpfr::mpreal(".5");

        iterations = 0;

        while(delta <= (mpfr::mpreal(1) - tree->get_max_value())/tree->get_max_value()) {
            iterations++;
            vector<int> targets;

            for(F pivot: pivots){
                int x = tree->find(pivot);
                F left = pivot - tree->query(0, x-1);
                F right = tree->query(0, x) - pivot;

                if(left > right) {
                    targets.push_back(x+1);
                }
                else {
                    targets.push_back(x);
                }
            }

            vector<int> results(targets.size());
            
            // will be made parallel
            // #pragma omp parallel for
            for(int i = 0;i < targets.size(); ++i){
                int x = targets[i];
                results[i] = problem.f(x);
            }

            // now update results
            
            for(int i = 0;i < targets.size(); ++i){
                int x = targets[i];
                F left = tree->query(0, x-1);
                F right = tree->query(x, upper_bound);
                if (results[i] == 0) {
                    F lmult = (tau - eps)/((tau - eps) * left  + (tau + eps) * right);
                    F rmult = (tau + eps)/((tau - eps) * left  + (tau + eps) * right);
                    tree->mult(0,x-1,lmult);
                    tree->mult(x,upper_bound,rmult);
                }
                else {
                    F lmult = (tau + eps)/((tau + eps) * left  + (tau - eps) * right);
                    F rmult = (tau - eps)/((tau + eps) * left  + (tau - eps) * right);
                    tree->mult(0,x-1,lmult);
                    tree->mult(x,upper_bound,rmult);
                }
            }

			if(mpfr::abs(tree->query(0, upper_bound) - mpfr::mpreal(1)) > .1){
				printf("BADBADBBADBADBAD\n");
                exit(0);
			}

		}
	
		//int ans = answerer.answer(pivots, tree, problem);
        int ans = tree->get_max_ans();
        std::cout << tree->get_max_value() << ' ' << iterations << '\n';

		delete tree;

		return ans;
	}
};

bool adaptive_flip(const F p0, const F p1, const F alpha, const F beta, Problem & problem, int x, F &emp_bias) {
    F A = (1 - beta) / alpha;
    F B = beta / (1 - alpha);
    F logA = mpfr::log(A);
    F logB = mpfr::log(B);

    F logLikelihood = 0.0;
    int n = 0;
    int success = 0;

    while (true) {
        bool heads = problem.f(x); 
        n++;
        success += heads;

        // Update log likelihood ratio
        if (heads) {
            logLikelihood += mpfr::log(p1 / p0);
        } else {
            logLikelihood += mpfr::log((1 - p1) / (1 - p0));
        }

        if (logLikelihood >= logA) {
            emp_bias = mpfr::mpreal(success)/n;
            return 1;
        } else if (logLikelihood <= logB) {
            emp_bias = mpfr::mpreal(success)/n;
            return 0;
        }
    }
}

struct AdaptiveSolver {
    int solve(Problem & problem, vector<F> pivots, const Answerer &answerer, F eps, int iterations){

		int upper_bound = problem.range().second;

        // the x'th interval corresponds to interval between coins x and x + 1
		SegTree * tree = new SegTree(0, upper_bound, mpfr::mpreal(1));

        F tau = mpfr::mpreal(".5");

        F temp = mpfr::mpreal(".25");
        F gamma = 1 - eps*eps*pivots.size();
        F epsprime = eps;

		for(int i = 0;i < iterations; ++i){
            vector<int> targets;

            for(F pivot: pivots){
                int x = tree->find(pivot);
                F left = pivot - tree->query(0, x-1);
                F right = tree->query(0, x) - pivot;

                if(left > right){
                    targets.push_back(x+1);
                }
                else{
                    targets.push_back(x);
                }
            }

            vector<int> results(targets.size());
            vector<F> estimated_biases(targets.size());
            
            // will be made parallel
            //#pragma omp parallel for
            for(int i = 0;i < targets.size(); ++i){
                int x = targets[i];
                results[i] = adaptive_flip(tau - eps, tau + eps, tau - epsprime, tau - epsprime, problem, x, estimated_biases[i]); 
            }

            // compute average bias
            F average = 0;
            for(auto bias : estimated_biases){
                average += mpfr::abs(bias - 1);
            }

            average /= targets.size();

            // now update results
            
            for(int i = 0;i < targets.size(); ++i){
                int x = targets[i];
                F left = tree->query(0, x-1);
                F right = tree->query(x, upper_bound);
                if (results[i] == 0) {
                    F lmult = (tau - epsprime)/((tau - epsprime) * left  + (tau + epsprime) * right);
                    F rmult = (tau + epsprime)/((tau - epsprime) * left  + (tau + epsprime) * right);
                    tree->mult(0,x-1,lmult);
                    tree->mult(x,upper_bound,rmult);
                }
                else {
                    F lmult = (tau + epsprime)/((tau + epsprime) * left  + (tau - epsprime) * right);
                    F rmult = (tau - epsprime)/((tau + epsprime) * left  + (tau - epsprime) * right);
                    tree->mult(0,x-1,lmult);
                    tree->mult(x,upper_bound,rmult);
                }
            }

			if(mpfr::abs(tree->query(0, upper_bound) - mpfr::mpreal(1)) > .1){
				printf("BAD\n");
                exit(0);
			}

		}
	
		int ans = answerer.answer(pivots, tree, problem);
        //std::cout << tree->arg_max << ' ' << ans << '\n';

		delete tree;

		return ans;
	}
};

#endif
