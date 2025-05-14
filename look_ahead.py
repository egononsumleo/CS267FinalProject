import math
import argparse
import numpy as np
from scipy.optimize import minimize

EPS = 0.1 # The epsilon from NBS
depth = 1 # The number of steps to look ahead
procs = 2**depth - 1 # Number of processors. This is never used.
precision = 400 # The number of binary digits after the decimal point base 2 for gradient descent to be accurate on.

# Heads (True) suggests that the answer is to the left
# Tails (False) suggests that the answer is to the right
# Quantiles are measured starting from the left to the right

# The binary entropy value of a p biased coin
def H(p):
    if p == 1 or p == 0:
        return 0
    else:
        return p * math.log(1/p) + (1 - p) * math.log(1/(1-p))

# 0.5 + EPS * (2*p-1) = p * (0.5 + EPS) + (1 - p) * (0.5 - EPS)
# def new_update_quantile(q, p, res):
#     if (p >= q) and (res == True):
#         return q * (0.5 + EPS)/(p * (0.5 + EPS) + (1 - p) * (0.5 - EPS))

#     elif (p >= q) and (res == False):
#         return q * (0.5 - EPS)/(p * (0.5 - EPS) + (1 - p) * (0.5 + EPS))

#     elif (p < q) and (res == True):
#         return (1 - ((1-q) * (0.5 - EPS)/(0.5 - EPS * (2*(1-p)-1))))

#     elif (p < q) and (res == False):
#         return (1 - ((1-q) * (0.5 - EPS)/(0.5 + EPS * (2*(1-p)-1))))

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

# This function takes q as input and does the reverse of update_quantile via gradient descent.
# (i.e. we should have that reverse_update_quantile(update_quantile(q, p, res), p, res, 1000) = q.)
def reverse_update_quantile(q, p, res, accuracy):

    cur_guess = q # Initial guess
    cur_acc = 2 # The initial granularity of the increments (cur_acc = -log_2(inc) always)
    inc = 1/4 # The initial increment for gradient descent

    # Do a while loop to find the right answer
    got_smaller = True
    while got_smaller:

        got_smaller = False

        # Check the number below
        next_guess = max(cur_guess-inc, 0)
        if abs(q - update_quantile(next_guess, p, res)) < abs(q - update_quantile(cur_guess, p, res)):
            cur_guess = next_guess
            got_smaller = True

        # Check the number above (if below did not work)
        if not got_smaller:
            next_guess = min(cur_guess+inc, 1)
            if abs(q - update_quantile(next_guess, p, res)) < abs(q - update_quantile(cur_guess, p, res)):
                cur_guess = next_guess
                got_smaller = True

        # If neither answer worked, shrink your increment by half
        if (not got_smaller) and cur_acc < accuracy:
            cur_acc += 1
            inc = inc/2
            got_smaller = True

    return cur_guess


# This function takes the number of steps to look ahead (level) and outputs the quantiles to query at.
# (Accuracy is the parameter used for the gradient descent. Just make it 100 or more and the answer should be fine.)
def get_quantiles(level, accuracy):

    # Base case
    if level == 1:
        return [0.5]
    
    # With one step down, the next level-1 steps should be the same
    lower_quantiles = get_quantiles(level-1, accuracy)

    # Reverse the quantiles depending on what the answer of the first coin was
    got_tails_quantiles = [reverse_update_quantile(lower_q, 0.5, True, accuracy) for lower_q in lower_quantiles]
    got_heads_quantiles = [reverse_update_quantile(lower_q, 0.5, False, accuracy) for lower_q in lower_quantiles]

    return sorted(got_tails_quantiles + [0.5] + got_heads_quantiles)

# Outputs the look ahead quantiles
print(get_quantiles(depth, precision))
