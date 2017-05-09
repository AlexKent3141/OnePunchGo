# OnePunchGo (OPG)
Go AI program which is inspired by the protagonist from the "One Punch Man" anime.

## Plan
This program is at an early stage of development. The rough roadmap is:

### Phase 1
<b>Board representation + tests.</b> Creating a correct and efficient board representation is a non-trivial problem for Go. This phase is primarily concerned with correctness.

### Phase 2
<b>Implement Monte-Carlo Tree Search.</b> MCTS is the most effective known tree-searching algorithm for Go: it copes well with the high branching factor and lack of a good heuristic function.

### Phase 3
<b>GTP (Go Text Protocol) compliance.</b> GTP is the standard protocol for communicating with Go engines. Once this phase is complete it will be possible to test OPG against itself and other engines to check performance.
