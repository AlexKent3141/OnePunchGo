import random
from solution_generator import *

# This class wraps up the parameters for controlling a genetic algorithm.
class GeneticParams:
    def __init__(self, cr, mr, pop_size):
        self.cr = cr # Crossover rate.
        self.mr = mr # Mutation rate.
        self.pop_size = pop_size # Population size.

# This class is used to apply a genetic algorithm to a problem.
class GeneticSolver:
    # Parameters are: GeneticParams, ChromosoneFactory, assessor function for chromosones.
    # The assessor function accepts a list of chromosones and returns a list of fitnesses.
    # The optional "on_best" method is called when a new best chromosone is found.
    def __init__(self, gps, sg, assessor, on_best=None):
        self.gps = gps
        self.sg = sg
        self.assessor = assessor
        self.on_best = on_best

    # Start the solver.
    def solve(self, max_steps):
        pop = [self.sg.random_solution() for i in range(self.gps.pop_size)]
        for i in range(max_steps):
            print "Generation:", i
            pop = self.evolve(pop)

    # Evolve the given population of chromosones and produce a new generation.
    def evolve(self, pop):
        # Assess the population.
        results = self.assessor(pop)

        best_result = max(results)
        i = results.index(best_result)
        best = pop[i]

        # If configured indicate that a new best chromosone has been found.
        if self.on_best:
            self.on_best(best, best_result)

        # Elitism: Keep the best chromosone from the previous generation.
        next_pop = [best]
        while len(next_pop) < self.gps.pop_size:
            # Perform tournament style selection to find two candidates.
            cands = []
            for i in range(0, 2):
                r1 = random.randrange(0, self.gps.pop_size)
                r2 = random.randrange(0, self.gps.pop_size)
                cands.append(pop[r1] if results[r1] > results[r2] else pop[r2])

            # Now combine the candidates to create an element of the next generation.
            r = random.random()
            child = self.cross_over(*cands) if r < self.gps.cr else cands[0]
            child = self.mutate(child)

            next_pop.append(child)
        return next_pop

    # Perform the cross-over operation on the two chromosones and return the result.
    def cross_over(self, c1, c2):
        c = []
        for i in range(len(c1)):
            r = random.random()
            c.append(c1[i] if r < 0.5 else c2[i])
        return c

    # Perform the mutation operation on the specfied chromosone.
    def mutate(self, c):
        for i in range(len(c)):
            r = random.random()
            if r < self.gps.mr:
                c[i] = self.sg.random_component(i)
        return c
