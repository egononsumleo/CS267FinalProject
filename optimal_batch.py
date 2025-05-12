import math
import argparse
import numpy as np
import random
from datetime import datetime
#from scipy.optimize import minimize
from skopt import gp_minimize
from skopt.space import Real

# python code to calculate what the optimal points to flip are

parser = argparse.ArgumentParser(
                    prog='Optimal Strategy Calculator',
                    description='Uses numerical optimization to find the optimal strategy',
                    epilog=':)')

parser.add_argument('-e', "--epsilon")
parser.add_argument('-t', "--tau")
parser.add_argument('-p', "--procs")

args = parser.parse_args()

EPS = float(args.epsilon)
TAU = float(args.tau)
procs = int(args.procs)

# binary entropy (with logits instead of bits) of p biased coin
def H(p):
    return p * math.log(1/p) + (1 - p) * math.log(1/(1-p))

def estimate_entropy(quantiles):
    quantiles = quantiles.copy()
    result = 0
    ans = random.uniform(0, 1)
    samples = []
    for q in quantiles:
        bias = TAU - EPS if q <= ans else TAU + EPS
        flipresult = True if (random.uniform(0, 1) <= bias) else False
        samples.append(flipresult)

    # so the SAMPLES ARE WHAT WE EXPECT

    # calculate effect on the middle
    for i in range(len(quantiles)):
        result -= math.log(calc_mult(ans, quantiles[i], samples[i]))
        #result += (calc_mult(ans, quantiles[i], samples[i]))
        # update all future locations
        for j in range(i+1, len(quantiles)):
            assert False
            quantiles[j] = update_quantile(quantiles[j], quantiles[i], TAU, EPS)

    return result

def f(deltas):
    # convert deltas to quantiles
    quantiles = [deltas[0]]
    for i in range(1,len(deltas)):
        quantiles.append(min(quantiles[-1] + deltas[i], 1))


    #M = 10000
    M = 1000
    res = 0
    for _ in range(M):
        res += estimate_entropy(quantiles) 
    return res/M

def wrapper2(inputs):
    return f(list(inputs))

# if coin at quantile p results in res how does the weight at q change?
def calc_mult(q, p, res):
    if p >= q:
        if res == True:
            return (TAU + EPS)/((TAU + EPS) * p + (TAU - EPS) * (1-p)) 
        else:
            return (1 - TAU - EPS)/((1 - TAU + EPS) * (1 - p) + (1 - TAU - EPS) * (p))
    else:
        if res == True:
            return (TAU - EPS)/((TAU + EPS) * p + (TAU - EPS) * (1-p)) 
        else:
            return (1 - TAU + EPS)/((1 - TAU + EPS) * (1 - p) + (1 - TAU - EPS) * (p))


# if coin at quantile p results in res how does the quantile q change?
def update_quantile(q, p, tau, res):
    if p >= q:
        if res == True:
            return q * (tau + EPS)/((tau + EPS) * p + (tau - EPS) * (1-p)) 
        else:
            return q * (1 - tau - EPS)/((1 - tau + EPS) * p + (1 - tau - EPS) * (1-p))
    else:
        return 1 - update_quantile(1 - q, 1 - p, 1 - tau, not res)

def wrapper(inputs):
    return quantiles_to_entropy(list(inputs))

# inputs is a list of quantiles, and the output is the entropy from choosing said quantiles
def quantiles_to_entropy(quantiles):
    if len(quantiles) == 0:
        return 0

    quantiles = sorted(quantiles)
    # resolve the median one
    med = len(quantiles)//2
    q = quantiles[med]
    bias = (1 - q) * (TAU - EPS) + q * (TAU + EPS)
    entropy = H(bias)

    # update the remaining quantiles

    L = quantiles[:med]
    L1 = []
    L2 = []

    for i in range(len(L)):
        L1.append(update_quantile(L[i],q,TAU,True))
        L2.append(update_quantile(L[i],q,TAU,False))
        
    R = quantiles[med+1:]
    R1 = []
    R2 = []

    for i in range(len(R)):
        R1.append(update_quantile(R[i],q,TAU,True))
        R2.append(update_quantile(R[i],q,TAU,False))

    ent1 = bias * quantiles_to_entropy(L1)  + (1 - bias) * quantiles_to_entropy(L2)
    ent2 = bias * quantiles_to_entropy(R1)  + (1 - bias) * quantiles_to_entropy(R2)
    return ent1 + ent2 + entropy


n = procs
random.seed(datetime.now().timestamp())

# Initial guess: already increasing
# initial_guess = np.array([ (i+1)/(procs+1) for i in range(procs)])
# initial_guess = np.array([ .5 for i in range(procs)])

initial_guess = np.array([ .1 for i in range(procs)])

# Constraints: x[i+1] - x[i] > 0 (strictly increasing)
constraints = []

for i in range(n - 1):
    constraints.append({
        'type': 'ineq',
        'fun': lambda x: x[i+1] - x[i] - 1e-5  # small epsilon to enforce strictly increasing
    })


space = [Real(0, 1.0, name=f'delta{i}') for i in range(n)]
#bounds = [(0.0, 1.0) for _ in range(n)]

result = gp_minimize(func = lambda x: wrapper2(x), noise='gaussian', dimensions = space, n_calls=100, n_random_starts = 10, random_state = 42 ) 

print(wrapper2([.5]))
print(wrapper2([.65]))

# Results
print(result.x)

