from runner import *
from chromosone_factory import *
from multiprocessing import Pool

# Determine optimal parameter values using a genetic algorithm.

class Match:
    def __init__(self, p1, p2, c1, c2):
        self.p1 = p2
        self.p2 = p2
        self.c1 = c1
        self.c2 = c2

def play_match(match):
    path = "../OPG.out"
    game = GameParameters(9, 7.5, 1)
    res = PlayCustom(path, path, game, match.c1, match.c2)
    winner = match.p1 if res == 0 else match.p2 if res == 1 else -1
    print winner
    return winner

# Double round-robin tournament between the given chromosones.
def tournament(chromosones):
    # Execute the matches in parallel.
    num_chroms = len(chromosones)
    matchups = []
    matchups.extend([Match(i, j, chromosones[i], chromosones[j]) \
                     for i in range(0, num_chroms) \
                     for j in range(i+1, num_chroms)])
    matchups.extend([Match(j, i, chromosones[j], chromosones[i]) \
                     for i in range(0, num_chroms) \
                     for j in range(i+1, num_chroms)])

    p = Pool(4)
    results = p.map(play_match, matchups)

    totals = []
    for i in range(0, num_chroms):
        totals.append(results.count(i))

    print totals

cf = ChromosoneFactory("test.chs")
chroms = [cf.random_chromosone() for i in range(0, 7)]

tournament(chroms)
