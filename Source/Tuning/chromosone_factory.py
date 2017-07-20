import random

# This class generates/modifies chromosones according to the specified schema.
# The schema (stored in an *.chs file) should be a sequence of upper and lower bounds which
# are the imposed constraints on each base in a chromosone.
class ChromosoneFactory:
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

    # Generate a random chromosone.
    def random_chromosone(self):
        chrom = []
        for i in range(0, len(self.constraints)):
            chrom.append(self.random_base(i))
        return chrom

    # Generate a random base.
    def random_base(self, i):
        constraint = self.constraints[i]
        return random.uniform(constraint[0], constraint[1])

if __name__ == "__main__":
    cf = ChromosoneFactory("test.chs")
    for i in range(0, 10):
        print cf.random_chromosone()
