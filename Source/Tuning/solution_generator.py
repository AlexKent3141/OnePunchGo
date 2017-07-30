import random

# This class generates/modifies candidate solutions according to the specified schema.
# The schema (stored in an *.chs file) should be a sequence of upper and lower bounds which
# are the imposed constraints on each component in a solution.
class SolutionGenerator:
    def __init__(self, schema_file):
        self.load_constraints(schema_file)

    # Parse the schema file and load constraints.
    def load_constraints(self, schema_file):
        with open(schema_file) as chs:
            content = chs.readlines()

        content = [l.strip() for l in content]

        # Find the upper and lower bounds that each line represents.
        self.constraints = []
        for l in content:
            tokens = l.split(":")
            self.constraints.append([float(tokens[0]), float(tokens[1])])

    # Generate a random candidate solution.
    def random_solution(self):
        chrom = []
        for i in range(0, len(self.constraints)):
            chrom.append(self.random_component(i))
        return chrom

    # Generate a random component.
    def random_component(self, i):
        constraint = self.constraints[i]
        return random.uniform(constraint[0], constraint[1])

    # Generate a neighbouring solution for the specified one.
    # A neighbour differs from the original in only one component and not by a massive amount.
    def neighbour(self, solution):
        n = solution[:]
        i = int(random.random() * len(self.constraints))
        con = self.constraints[i]
        r = con[1] - con[0]
        limit = 0.1*r # This is how much of a change can be made.
        orig = n[i]
        n[i] = orig + random.uniform(-limit, limit)
        while n[i] < con[0] or n[i] > con[1]:
            n[i] = orig + random.uniform(-limit, limit)
        return n

if __name__ == "__main__":
    cf = SolutionGenerator("test.chs")
    for i in range(0, 10):
        print cf.random_solution()
