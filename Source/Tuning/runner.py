# This python library exposes methods for playing games between different versions of OPG.

import pexpect
import time

pass_move = "pass"
resign_move = "resign"
colours = ["B", "W"]

# This class defines the parameters for a game.
class GameParameters:
    def __init__(self, board_size, komi, secs_per_move):
        self.board_size = board_size
        self.komi = komi
        self.secs_per_move = secs_per_move

# This class wraps up a single instance of OPG (or in fact and GTP compatible Go AI).
class AIProcess:
    COMMANDS = ["boardsize", "clear_board", "komi", "time_settings", "quit", "play", "genmove", "opg_parameters"]

    def __init__(self, path):
        self.path = path
        self.proc = None

    def new_game(self, game_params):
        if not self.proc:
            self.proc = pexpect.spawn(self.path)

        self.proc.sendline("boardsize " + str(game_params.board_size))
        self.proc.sendline("clear_board")
        self.proc.sendline("komi " + str(game_params.komi))

        # Specify the time controls to use.
        secs = str(game_params.secs_per_move)
        self.proc.sendline("time_settings " + secs + " " + secs + " 1")

    def quit(self):
        self.proc.sendline("quit")

    def add_move(self, move):
        self.proc.sendline("play " + move)

    def get_move(self, colour):
        self.proc.sendline("genmove " + colour)
        best_move = ""
        while not best_move:
            # Keep going until a move is returned.
            res = self.proc.readline()
            sent = [c for c in self.COMMANDS if c in res]
            if len(res.strip()) > 1 and not sent:
                best_move = res
                break

        return best_move.strip("=").strip()

# Play a game between two AI programs with the specified game parameters.
# Return the index (0 or 1) of the winning program.
# If the winner was not clear then return -1.
def Play(path1, path2, game_params):
    ais = [AIProcess(path1), AIProcess(path2)]
    for ai in ais:
        ai.new_game(game_params)

    # Keep asking for moves while neither player has resigned and there
    # have not been two consecutive passes.
    num_passes = 0
    turn_number = 0
    while True:
        player = ais[turn_number % 2]
        opponent = ais[1 - turn_number % 2]
        col = colours[turn_number % 2]
        move = player.get_move(col)
        print move

        opponent.add_move(col + " " + move)

        if pass_move in move:
            num_passes += 1
            if num_passes == 2:
                break
        elif resign_move in move:
            break
        else:
            num_passes = 0
            
        turn_number += 1

    # The game has terminated and it *should* be obvious who won! (i.e. a player resigned).
    winner = -1
    if num_passes != 2:
        winner = 1 - turn_number % 2

    return winner

if __name__ == "__main__":
    ai = "Source/OPG.out"
    params = GameParameters(9, 7.5, 1)
    print Play(ai, ai, params)
