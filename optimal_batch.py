import math
import numpy as np
from scipy.optimize import minimize

# python code to calculate what the optimal points to flip are


EPS = 0.1

# binary entropy (with logits instead of bits) of p biased coin
def H(p):
    return p * math.log(1/p) + (1 - p) * math.log(1/(1-p))

# if coin at quantile p results in res how does the quantile q change?
def update_quantile(q, p, res):
    if p >= q:
        if res == True:
            return q * (0.5 + EPS)/(0.5 + EPS * (2*p-1))
        else:
            return q * (0.5 - EPS)/(0.5 - EPS * (2*p-1))
    else:
        return 1 - update_quantile( 1- q, 1 - p, not res)


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
    bias = (1-q) * (0.5 - EPS) + q * (0.5 + EPS)
    entropy = H(bias)

    # update the remaining quantiles

    L = quantiles[:med]
    L1 = []
    L2 = []

    for i in range(len(L)):
        L1.append(update_quantile(L[i],q,True))
        L2.append(update_quantile(L[i],q,False))
        
    R = quantiles[med+1:]
    R1 = []
    R2 = []

    for i in range(len(R)):
        R1.append(update_quantile(R[i],q,True))
        R2.append(update_quantile(R[i],q,False))

    ent1 = bias * quantiles_to_entropy(L1)  + (1 - bias) * quantiles_to_entropy(L2)
    ent2 = bias * quantiles_to_entropy(R1)  + (1 - bias) * quantiles_to_entropy(R2)
    return ent1 + ent2 + entropy


n = 7

# Initial guess: already increasing
initial_guess = np.array([.1,.2,.4,.5,.6,.8,.9])

# Constraints: x[i+1] - x[i] > 0 (strictly increasing)
constraints = []
for i in range(n - 1):
    constraints.append({
        'type': 'ineq',
        'fun': lambda x, i=i: x[i+1] - x[i] - 1e-5  # small epsilon to enforce strictly increasing
    })

result = minimize(lambda x: -wrapper(x), initial_guess, constraints=constraints)

# Results
x_max = result.x 
f_max = wrapper(x_max) - n * H(.5 + EPS)

print(f"Maximum occurs at x = {x_max}")
print(f"Maximum value = {f_max}")
