import random

# This class wraps up the parameters for controlling the simulated annealing algorithm.
class AnnealingParams:
    def __init__(self, temp_target, temp_iterations):
        self.temp_target = temp_target # The target temperature.
        self.temp_iterations = temp_iterations # The number of cycles at each temperature.

# This class is used to apply simulated annealing to a problem.
class AnnealingSolver:
    # Parameters are: AnnealingParams, SolutionGenerator, assessor function for chromosones,
    # (optional) callback when a new best solution candidate is found.
    def __init__(self, aps, sg, assessor, on_best=None):
        self.aps = aps
        self.sg = sg
        self.assessor = assessor
        self.on_best = on_best

    # Start the solver.
    def solve(self):
        solution = self.sg.random_solution()
        old_fitness = self.assessor(solution)
        self.solution_assessed(solution, old_fitness)
        temp = 1.0
        alpha = 0.9
        while temp > self.aps.temp_target:
            i = 0
            while i < self.aps.temp_iterations:
                new_solution = self.sg.neighbour(solution)
                new_fitness = self.assessor(new_solution)
                ap = self.acceptance_probability(old_fitness, new_fitness, temp)
                if ap > random.random():
                    solution = new_solution
                    old_fitness = new_fitness
                    self.solution_assessed(solution, old_fitness)
                i += 1
            temp *= alpha
        return solution, old_fitness


    # Note: this isn't strictly how the acceptabls probability is meant to work - but my
    # fitness method is very discrete ATM so it's probably OK!
    def acceptance_probability(self, old_fitness, new_fitness, temp):
        p = 0.0
        if new_fitness > old_fitness:
            p = 1.0
        elif new_fitness == old_fitness:
            p = 0.5
        return p

    def solution_assessed(self, solution, fitness):
        print "Solution:", solution, fitness
        if self.on_best:
            self.on_best(solution, fitness)
