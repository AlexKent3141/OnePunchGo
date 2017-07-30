from runner import *
from solution_generator import *
from genetic_solver import *
from annealing_solver import *
from multiprocessing import cpu_count, Pool

# Determine optimal parameter values using a genetic algorithm.

class Match:
    def __init__(self, p1, p2, c1, c2):
        self.p1 = p1
        self.p2 = p2
        self.c1 = c1
        self.c2 = c2

class MasterMatch:
    def __init__(self, p, c, time_per_move):
        self.p = p
        self.c = c
        self.time_per_move = time_per_move
        self.res = 0

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

# Play both sides of the chromosone vs current master.
def play_master_match(match):
    opg_path = "../OPG.out"
    master_path="../master/OPG.out"
    game = GameParameters(9, 7.5, match.time_per_move)

    # Score from c's perspective.
    res = PlayCustom(opg_path, master_path, game, match.c, [])
    winner1 = 1 if res == 0 else (-1 if res == 1 else 0)
    print match.p, match.c, winner1
    res = PlayCustom(master_path, opg_path, game, [], match.c)
    winner2 = 1 if res == 1 else (-1 if res == 0 else 0)
    print match.p, match.c, winner2

    match.res = winner1 + winner2
    print "Match:", match.res
    return match

# Double round-robin tournament between the given chromosones.
def tournament(chromosones):
    # Construct a list of all matchups to execute.
    num_chroms = len(chromosones)
    num_matches_per_chrom = 5
    matchups = []

    for i in range(num_chroms):
        matchups.extend([MasterMatch(i, chromosones[i], 2) for j in range(num_matches_per_chrom)])

    # Scale the number of worker threads in a logical way.
    # Each worker will start two instances of OPG which perform CPU bound work.
    num_workers = cpu_count()/2
    print "Num workers:", num_workers

    p = Pool(num_workers)
    results = p.map(play_master_match, matchups)

    totals = [0 for i in range(num_chroms)]
    for m in results:
        totals[m.p] += m.res

    print totals
    return totals

def assess(solution):
    # Play a tournament against current master for reference.
    num_games = 100;
    source = [MasterMatch(0, solution, 1) for i in range(num_games/2)]
    num_workers = cpu_count()/2
    p = Pool(num_workers)
    results = p.map(play_master_match, source)

    score = 0
    for m in results:
        score += m.res

    return score

def on_best(best_chrom, result):
    with open("best.txt", 'a+') as f:
        f.write(str(best_chrom) + " " + str(result) + "\n")

gps = GeneticParams(0.8, 0.15, 7)
cf = SolutionGenerator("test.chs")
gs = GeneticSolver(gps, cf, tournament, on_best)

gs.solve(1000)

