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

    const int procs = 15;

	//vector<Selector*> selectors = {new MedianSelector(), new RandomSelector(), new QSelector()};
	MedianAnswerer answerer = MedianAnswerer();
	Solver solver = Solver();	

    vector<vector<F>> strategies;

    strategies.push_back({0.44682301377817496, 0.4637143400093893, 0.47962723802743534, 0.4813562690251669, 0.48155226575351157, 0.49745287158137996, 0.49998999567787655, 0.49999999567787656, 0.5000099956778765, 0.5025473812353886, 0.5184477001077291, 0.5186435931686747, 0.5203731243436185, 0.536285815232788, 0.553177278919289});

    // median strat
    vector<F> median;
    for(int i = 0;i < 15; ++i){
        median.push_back(.5);
    }

    strategies.push_back(median);

	double eps = .1;
	//const F PROBLEM_SIZE = mpfr::mpreal(1e10);
	const int PROBLEM_SIZE = 1000000; 
	const int M = 400;
	int exp_iterations = int(mpfr::log(PROBLEM_SIZE)/(procs*eps*eps));

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
