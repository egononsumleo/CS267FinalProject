import math
import argparse
import numpy as np
import random
from scipy.optimize import minimize

EPS = 0.125 # The epsilon from NBS
procs = 7 # Number of processors.
precision = 20 # The number of binary digits after the decimal point base 2 for gradient descent to be accurate on.

# Heads (True) suggests that the answer is to the left
# Tails (False) suggests that the answer is to the right
# Quantiles are measured starting from the left to the right

# The binary entropy value of a p biased coin
def H(p):
    if p == 1 or p == 0:
        return 0
    else:
        return p * math.log(1/p) + (1 - p) * math.log(1/(1-p))

# If the coin output at the p'th quantile was res, what will the q'th quantile of 
# the prior distribution be located wrt the posterior distribution
def update_quantile(q, p, res):
    if p >= q:
        if res == True:
            return q * (0.5 + EPS)/(0.5 + EPS * (2*p-1))
        else:
            return q * (0.5 - EPS)/(0.5 - EPS * (2*p-1))
    else:
        return 1 - update_quantile(1-q, 1-p, not res)

# inputs is a list of quantiles, and the output is the entropy from choosing said quantiles
def quantiles_to_entropy(quantiles):

    if len(quantiles) == 0:
        return 0

    bias = quantiles[0] * (0.5 + EPS) + (1 - quantiles[0]) * (0.5 - EPS)
    first_entropy = H(quantiles[0])

    # update the remaining quantiles
    other_quants_if_heads = [update_quantile(other_q, quantiles[0], True) for other_q in quantiles[1:]]
    other_quants_if_tails = [update_quantile(other_q, quantiles[0], False) for other_q in quantiles[1:]]

    return (first_entropy + bias * quantiles_to_entropy(other_quants_if_heads) + (1 - bias) * quantiles_to_entropy(other_quants_if_tails))

def grad_desc(init_guess, accuracy, procs):

    cur_guess = [init_guess[i] for i in range(procs)] # Initial guess
    cur_acc = 2 # The initial granularity of the increments (cur_acc = -log_2(inc) always)
    inc = 1/4 # The initial increment for gradient descent

    # Do a while loop to find the right answer
    got_smaller = True
    while got_smaller:

        # print(cur_guess)

        got_smaller = False
        for i in range(procs):
            next_guess = [cur_guess[i] for i in range(procs)]
            if i == 0:
                next_guess[i] = max(next_guess[i] - inc, 0)
            else:
                next_guess[i] = max(next_guess[i] - inc, next_guess[i-1])
            if quantiles_to_entropy(next_guess) > quantiles_to_entropy(cur_guess):
                cur_guess = next_guess
                got_smaller = True
                break

        if not got_smaller:
            for i in range(procs):
                next_guess = [cur_guess[i] for i in range(procs)]
                if i == procs-1:
                    next_guess[i] = min(next_guess[i] + inc, 1)
                else:
                    next_guess[i] = min(next_guess[i] + inc, next_guess[i+1])
                if quantiles_to_entropy(next_guess) > quantiles_to_entropy(cur_guess):
                    cur_guess = next_guess
                    got_smaller = True
                    break

        # If neither answer worked, shrink your increment by half
        if (not got_smaller) and cur_acc < accuracy:
            cur_acc += 1
            inc = inc/2
            got_smaller = True

    return cur_guess

print(grad_desc([0.5 for i in range(procs)], precision, procs))


# The following commented code is used to test the volatility of the entropy function

# outputs = []
# for whatever in range(10000):
#     cur_quants = [0.01, 0.04, 0.09, 0.16, 0.25, 0.36, 0.49, 0.64, 0.81, 1]
#     # cur_quants = [(i+1)/(procs+1) for i in range(procs)]
#     random.shuffle(cur_quants)
#     # cur_quants = [0.5 for i in range(procs)]
#     outputs.append(quantiles_to_entropy(cur_quants))

# outputs.sort()
# print(outputs[0])
# print(outputs[len(outputs)-1])

# print(quantiles_to_entropy([0.0001, 0.0001]))
# print(quantiles_to_entropy([0.9999, 0.9999]))