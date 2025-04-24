#include "RBS.h"
#include <bits/stdc++.h>

mt19937 generator = mt19937(time(NULL)); // TODO make this random seeded

struct StandardInstance : public Problem {
	uniform_real_distribution<double> distribution = uniform_real_distribution<double>(0,1);
	int N;
	int _answer;
	double eps;
	StandardInstance(int N, int _answer, double eps) : N(N), _answer(_answer), eps(eps){
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
	int digits = 128;
	mpfr::mpreal::set_default_prec(mpfr::digits2bits(digits));

	//vector<Selector*> selectors = {new MedianSelector(), new RandomSelector(), new QSelector()};
	MedianAnswerer answerer = MedianAnswerer();
	Solver solver = Solver();	

    vector<vector<F>> strategies;

    strategies.push_back({.4635623083962456, 0.4813945888128402, 0.49837768466777027, 0.5, 0.5016220983878305, 0.5186052643397824, 0.5364373997474698});
    strategies.push_back({.5,.5,.5,.5,.5,.5,.5});

	double eps = .1;
	//const F PROBLEM_SIZE = mpfr::mpreal(1e10);
	const int PROBLEM_SIZE = 100000; 
	const int M = 100;
	int exp_iterations = int(mpfr::log(PROBLEM_SIZE)/(2*eps*eps));

	cout << exp_iterations << '\n';

    std::uniform_int_distribution<> distrib(1, PROBLEM_SIZE-1);

    for(int strat = 0; strat < strategies.size(); ++strat){
        for(int i = exp_iterations;i <= exp_iterations; ++i){
            vector<F> v_correct;

            //vector<F> pivots = {mpfr::mpreal(.4), mpfr::mpreal(.5), mpfr::mpreal(.6)};
            auto pivots = strategies[strat];

            F correct = 0;
            for(int j = 0;j < M; ++j){
                //F answer = mpfr::mpreal(floor(distribution(generator)));
                int answer = distrib(generator); 
                auto instance = StandardInstance(PROBLEM_SIZE, answer, eps); 
                int res = solver.solve(instance, pivots, answerer, eps, i);
                

                if(res == answer){
                    ++correct;
                }
            }

            cout << "Percent correct: " << correct/M << '\n';
        }
    }
}
