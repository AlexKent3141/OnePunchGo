from runner import *
from chromosone_factory import *
from multiprocessing import cpu_count, Pool

# Determine optimal parameter values using a genetic algorithm.

class Match:
    def __init__(self, p1, p2, c1, c2):
        self.p1 = p1
        self.p2 = p2
        self.c1 = c1
        self.c2 = c2

def play_match(match):
    opg_path = "../OPG.out"
    game = GameParameters(9, 7.5, 1)

    winner = -1
    try:
        res = PlayCustom(opg_path, opg_path, game, match.c1, match.c2)
        winner = match.p1 if res == 0 else (match.p2 if res == 1 else -1)
        print "Res:", res
        print "Players:", match.p1, match.p2
        print "Chroms:", match.c1, match.c2
        print "Winner:", winner
    except Exception as e:
        print e

    return winner

# Double round-robin tournament between the given chromosones.
def tournament(chromosones):
    # Construct a list of all matchups to execute.
    num_chroms = len(chromosones)
    matchups = []
    matchups.extend([Match(i, j, chromosones[i], chromosones[j]) \
                     for i in range(0, num_chroms) \
                     for j in range(i+1, num_chroms)])
    matchups.extend([Match(j, i, chromosones[j], chromosones[i]) \
                     for i in range(0, num_chroms) \
                     for j in range(i+1, num_chroms)])

    # Scale the number of worker threads in a logical way.
    # Each worker will start two instances of OPG which perform CPU bound work.
    num_workers = cpu_count()/2
    print "Num workers:", num_workers

    p = Pool(num_workers)
    results = p.map(play_match, matchups)

    totals = []
    for i in range(0, num_chroms):
        totals.append(results.count(i))

    print totals
    return totals

cf = ChromosoneFactory("test.chs")
chroms = [cf.random_chromosone() for i in range(0, 10)]
print chroms

tournament(chroms)
