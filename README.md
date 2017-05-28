# OnePunchGo (OPG)
Go AI program which is inspired by the protagonist from the "One Punch Man" anime.

OPG is now in a working state but plays at a very weak level. The key improvements to make next are:
1) Make the obvious optimisations to the playout and moving-making procedures.
2) Embed some Go-specific knowledge in the search routine so that there is a bias towards clearly good moves.
3) Implement well known improvements to the MCTS algorithm such as AMAF/RAVE.

## Testing OPG
I have implemented a few types of unit tests for OPG; these can be executed by running with the *test* argument. The most exhaustive ones are the Tsumego solving tests which perform 5 second searches on positions where there is only one good move.

## Playing against OPG
OPG is GTP compliant and can therefore be used with existing UI programs which support that protocol. I have been using [GoGui](https://sourceforge.net/projects/gogui/) for my testing.

At the moment OPG could give a beginner level player a decent game of 9x9 Go. It's moves on larger board sizes tend to be fairly random!
