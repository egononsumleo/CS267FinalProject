#ifndef RBS_H
#define RBS_H
#include <algorithm>
#include <utility>
#include <cmath>
#include <cstdio>
#include <vector>
#include <random>
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
    
    SegTree(int _l, int _r, F _value) : l(_l), r(_r), value(_value), delayed(mpfr::mpreal(1)) {
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

    int mid(){
        return (l + r)/2;
    }

    void apply(F x){
        value *= x;
        delayed *= x;
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

// selects the answer
struct Answerer {
	virtual int answer(const vector<F> &pivots, SegTree * tree, Problem & problem) const = 0;
};

struct MedianAnswerer : public Answerer {
	int answer(const vector<F> &pivots, SegTree * tree, Problem & problem) const override {
		return tree->find(.5);
	}
};

struct Solver{
    int solve(Problem & problem, vector<F> pivots, const Answerer &answerer, F eps, int iterations){

		int upper_bound = problem.range().second;

        // the x'th interval corresponds to interval between coins x and x + 1
		SegTree * tree = new SegTree(0, upper_bound, mpfr::mpreal(1));

        F tau = mpfr::mpreal(".5");

		for(int i = 0;i < iterations; ++i){
            vector<int> targets;

            for(F pivot: pivots){
                int x = tree->find(pivot);
                F left = pivot - tree->query(0, x);
                F right = tree->query(0, x) - pivot;

                if(left > right){
                    targets.push_back(x+1);
                }
                else{
                    targets.push_back(x);
                }
            }

            vector<int> results;
            
            // will be made parallel
            for(int x: targets){
                results.push_back(problem.f(x));
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
	
		int ans = answerer.answer(pivots, tree, problem);

		delete tree;

		return ans;
	}
};

#endif
