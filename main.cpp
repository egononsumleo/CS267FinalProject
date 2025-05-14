#include "RBS.h"
#include "stdc++.h"

int main(){
    int digits = 128;
	mpfr::mpreal::set_default_prec(mpfr::digits2bits(digits));

    /* Setting up */
    const int procs = 63;
    double eps = 0.1;
	const int PROBLEM_SIZE = 1000000; 
	const int M = 400;

	// vector<Selector*> selectors = {new MedianSelector(), new RandomSelector(), new QSelector()};
	MedianAnswerer answerer = MedianAnswerer();
	Solver solver = Solver();	
	// AdaptiveSolver solver = AdaptiveSolver();	

    /* Initialize the strategies that we will test */
    /* A strategy is a vector of length procs that decides on which quantiles we will query.
       For example, the Equal Splitting strategy will query at 1/p, 2/p, ... 1.
       The Median strategy always queries at 0.5.
    */

    vector<vector<F>> strategies;

    // strategies.push_back({0.44682301377817496, 0.4637143400093893, 0.47962723802743534, 0.4813562690251669, 0.48155226575351157, 0.49745287158137996, 0.49998999567787655, 0.49999999567787656, 0.5000099956778765, 0.5025473812353886, 0.5184477001077291, 0.5186435931686747, 0.5203731243436185, 0.536285815232788, 0.553177278919289});
    // strategies.push_back({0.44682301377817496, 0.4637143400093893, 0.47962723802743534, 0.4813562690251669, 0.48155226575351157, 0.49745287158137996, 0.49998999567787655, 0.49999999567787656, 0.5000099956778765, 0.5025473812353886, 0.5184477001077291, 0.5186435931686747, 0.5203731243436185, 0.536285815232788, 0.553177278919289});
    // strategies.push_back({0.43015326941020887, 0.44660087494765055, 0.4622698593474061, 0.46271200063645024, 0.463663865381379, 0.47853717859890554, 0.4804150215616535, 0.4804250215616535, 0.4804350215616535, 0.48187061542966286, 0.4964709372018966, 0.49648093720189657, 0.49812627715117735, 0.49998039928574345, 0.49999039928574346, 0.5000003992857435, 0.5000103992857434, 0.5000203992857435, 0.5018739941030633, 0.503519285465945, 0.503529285465945, 0.5181297779358827, 0.5195655750534574, 0.5195755750534574, 0.5195855750534574, 0.5214635901675466, 0.5363371429945477, 0.5372885234655407, 0.5377304045167289, 0.5533998758533965, 0.5698475056902559});
    // strategies.push_back({0.4152918523389327, 0.4304811495712107, 0.4451616712503585, 0.44564991585020686, 0.4471890988194534, 0.4609077606757841, 0.46207543774085075, 0.4620854377408507, 0.4620954377408507, 0.4645101967612611, 0.47781275940932677, 0.4778227594093268, 0.47922167484796724, 0.4793463371785009, 0.4793563371785009, 0.4793663371785009, 0.4793763371785009, 0.47938633717850093, 0.4814611055841553, 0.48271650526296284, 0.48272650526296285, 0.495910125637268, 0.49637651192061616, 0.49638651192061617, 0.4963965119206162, 0.4982920869796602, 0.4999501593631548, 0.49996015936315474, 0.49997015936315475, 0.49998015936315476, 0.4999901593631548, 0.5000001593631548, 0.5000101593631547, 0.5000201593631548, 0.5000301593631548, 0.5000401593631548, 0.5000501593631548, 0.5017078944544977, 0.5036034862509652, 0.5036134862509651, 0.5036234862509652, 0.5040898892586586, 0.5172740806553954, 0.5172840806553953, 0.5185387002994158, 0.5206135131959347, 0.5206235131959347, 0.5206335131959346, 0.5206435131959347, 0.5206535131959347, 0.520777210500872, 0.5221767779274706, 0.5221867779274706, 0.5354889104071354, 0.5379042154627235, 0.5379142154627234, 0.5379242154627235, 0.5390932735836644, 0.5528097103338222, 0.554349833516779, 0.5548371469522237, 0.5695193937852223, 0.584707434132887});

    // 1. Median strategy
    vector<F> median;
    for(int i = 0;i < procs; ++i){
        median.push_back(.5);
    }
    strategies.push_back(median);

    // 2. Equal Splitting strategy
    vector<F> equal;
    for(int i = 0;i < procs; ++i){
        equal.push_back(float(i+1)/procs);
    }
    // strategies.push_back(equal);

	int exp_iterations = int(mpfr::log(PROBLEM_SIZE)/(.9 * procs*eps*eps));
	cout << "Expected number of iterations is" << exp_iterations << '\n';

    std::uniform_int_distribution<> distrib(1, PROBLEM_SIZE-1); // constructs uniform distribution over the integers in [1, PROBLEM_SIZE)

    auto delta = mpfr::mpreal(.1); // we keep iterating until we have 1 - delta confidence => 1 - delta success rate

    for(int strat = 0; strat < strategies.size(); ++strat) { // for every strategy
        for(int i = exp_iterations; i <= exp_iterations; ++i) { // TODO: why write it like this?
            
            vector<F> v_correct;
            auto pivots = strategies[strat];

            F correct = 0;
            int total_flips = 0;
            for(int j = 0;j < M; ++j) {

                // Prepare the problem instance
                int answer = distrib(generator); // choose the answer that we want at random
                auto instance = StandardInstance(PROBLEM_SIZE, answer, eps); // initialize the problem instance where we have PROBLEM_SIZE number of coins with biases 1/2-eps, ..., 1/2-eps (<-answer), 1/2+eps, ..., 1/2+eps

                int iterations;
                int res = solver.solve(instance, pivots, answerer, eps, delta, iterations);

                if(res == answer) { // if we have successfully computed the answer
                    ++ correct;
                }
                total_flips += iterations;
            }
            cout << "Percent correct: " << correct/M << " average # of flips " << total_flips/M << '\n';
        }
    }
}
