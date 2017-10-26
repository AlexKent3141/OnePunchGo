import numpy as np

# The class wraps up the parameters for controlling the quadratic solver.
class QuadraticParams:
    def __init__(self, num_split):
        assert num_split >= 3
        self.num_split = num_split

# This class attempts to optimise a system of parameters using quadratic fits.
# The basic plan is to:
# 1) Select a parameter to tune.
# 2) Sample the search space varying only the chosen parameter and applying the fitness function.
# 3) Fit a quadratic polynomial to the results and determine the parameter value which maximises
#    the fitness curve.
# 4) Repeat steps 1-3 for all parameters, possibly shrinking search space each time a parameter is
#    encountered.
class QuadraticSolver:
    def __init__(self, qps, sg, assessor, on_best=None):
        self.qps = qps
        self.sg = sg
        self.assessor = assessor
        self.on_best = on_best

    def solve(self, max_iterations=10):
        num_parameters = self.sg.num_parameters()
        solutions = [self.sg.parameter_middle(i) for i in range(num_parameters)]

        for i in range(max_iterations):
            for p in range(num_parameters):
                # Find the parameter range (centred around the current best value).
                r = self.sg.parameter_range(p, solutions[p], i)

                # Find a better value for this parameter by sampling the range.
                solutions[p] = self.fit_parameter(p, r, solutions)
                print "Current best: ", solutions

    def fit_parameter(self, p, p_range, all_parameters):
        # Sample the specified range.
        sample_diff = (p_range[1] - p_range[0]) / (self.qps.num_split - 1)
        param_vals = []
        results = []
        current_params = all_parameters
        for i in range(self.qps.num_split):
            param_val = p_range[0] + i*sample_diff
            current_params[p] = param_val
            param_vals.append(param_val)
            results.append(self.assessor(current_params))
        print results

        # Given the fitness landscape, fit a quadratic polynomial to estimate the best parameter
        # value.
        x = np.array(param_vals)
        y = np.array(results)
        fit = np.polyfit(x, y, 2)
        print fit

        # Find the parameter value which maximises this function.
        best = self.find_max((param_vals[0], param_vals[-1]), fit)

        print best, max(results)
        if self.on_best:
            self.on_best(best, max(results))

        return best

    def find_max(self, x_range, coeffs):
        steps = 100
        r = x_range[1] - x_range[0]
        inc = r / steps
        f_max = -1000
        x = x_range[0]
        best = x
        for i in range(steps):
            # Find the fitness for the current x.
            f = coeffs[0]*x*x + coeffs[1]*x + coeffs[2]
            if f > f_max:
                f_max = f
                best = x

            x += inc

        return best
