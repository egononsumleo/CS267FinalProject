#include "RBS.h"
#include <bits/stdc++.h>

struct StandardInstance : public Problem {
	mt19937 generator; // TODO make this random seeded
	uniform_real_distribution<double> distribution = uniform_real_distribution<double>(0,1);
	int N;
	int _answer;
	double eps;
	StandardInstance(int N, int _answer, double eps) : N(N), _answer(_answer), eps(eps){
		generator = mt19937(time(NULL));
	}

	int f(F x) override {
		if(x <= _answer){ // TODO CHANGE BACK
			return distribution(generator) <= .5 - eps ? 1 : 0;
		}
		else{
			return distribution(generator) <= .5 + eps ? 1 : 0;
		}
	}
	pair<int,int> range() const override {
		return make_pair(0, N);
	}
	int answer(){
		return _answer;
	}
};

int main(){
	int digits = 60;
	mpfr::mpreal::set_default_prec(mpfr::digits2bits(digits));

	//vector<Selector*> selectors = {new MedianSelector(), new RandomSelector(), new QSelector()};
	MedianAnswerer answerer = MedianAnswerer();
	Solver solver = Solver();	

	double eps = .1;
	//const F PROBLEM_SIZE = mpfr::mpreal(1e10);
	const int PROBLEM_SIZE = 100000; 
	const int M = 1;
	int exp_iterations = int(mpfr::log(PROBLEM_SIZE)/(2*eps*eps));

	cout << exp_iterations << '\n';

	auto generator = mt19937(1776);

	for(int i = exp_iterations;i <= exp_iterations; ++i){
		vector<F> v_correct;

        vector<F> pivots = {mpfr::mpreal(.5)};

        F correct = 0;
        for(int j = 0;j < M; ++j){
            //F answer = mpfr::mpreal(floor(distribution(generator)));
            int answer = PROBLEM_SIZE/2; 
            auto instance = StandardInstance(PROBLEM_SIZE, answer, eps); 
            int res = solver.solve(instance,pivots,answerer,eps,i);
            
            /*
            if(mpfr::floor(res) == answer){
                ++correct;
            }
            */

            correct += res; 
        
        }
        v_correct.push_back(correct);
		
		cout << i;
		for(F x:v_correct){
			cout << ' ' << x/M; 
		}
		//cout << '\n';
		cout << endl;
	}
}
