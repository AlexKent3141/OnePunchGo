# OnePunchGo (OPG)
Go AI program which is inspired by the protagonist from the "One Punch Man" anime.

OPG is in a working state, but while it plays competitively on a 9x9 board it is still very weak on a full size board.

## Compiling OPG
OPG uses CMake to generate its build system and can be compiled on both Linux and Windows.

## Direction
The next stages in OPG's development will be to:
1) Embed more Go-specific knowledge in the search routine so that it is more biased towards good moves.
2) Improve scalability when using a multi-threaded search.
3) Experiment with further search improvements - neural networks are in vogue these days :)

## Testing OPG
I have implemented a few types of unit tests for OPG; these can be executed by running the *test* executable. The most exhaustive ones are the Tsumego solving tests which perform 5 second searches on positions where there is only one good move.

## Playing against OPG
OPG is GTP compliant and can therefore be used with existing UI programs which support that protocol. I have been using [GoGui](https://sourceforge.net/projects/gogui/) for my testing.

At the moment OPG can play a decent game of 9x9 Go. Its moves on larger board sizes tend to be fairly random!
