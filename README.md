# Game theory library

Provides three algorithms to find Nash equilibria in games: Sastry
(reinforcement learning), MCB (a modified version of Sastry taking
advantage of SMP processing) and SFP (Sampled Fictitious Player).

Games can be modeled in Normal Form (ie. with a payoff matrix) or with
an arbitrary payoff function.

Results and algorithm progression can be visualized through SVG files.

# Examples
See example folders for basic examples.

```sh
./configure
make
./examples/sastry examples/games/battle_of_sexes 
2 players, 2 strategies
2.000000 3.000000 ;0.000000 0.000000 ;
1.000000 1.000000 ;3.000000 2.000000 ;
last iteration: 65
0.999 0.001 
1.000 0.000 
```
