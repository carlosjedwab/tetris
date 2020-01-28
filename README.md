# Tetris
A game made inspired by the classic Tetris

Built with the SFML C++ library, which is necessary for running the game.

# Running on Linux
With SFML installed, run the run.py script to open the game.

![Tetris game](Screenshot.png?raw=true "Title")

# Game mechanics
### Key commands
The right and left key move the fallling piece accordingly, the up key rotates the falling piece, the down key makes the falling piece fall slightly faster
### Level system
For every 10 lines cleared during a game, the level is increased by 1. Higher levels have higher falling speeds for the falling piece (maximum speed is reached at level 7)

# Some simple customization
It is easy to change:
- the game's dimentions by changing the constant 'L';
- the number of lines necessery to clear for a level up by changing the constant 'LinesPerLevel';
- the score points per line set in the 'scoringSystem' function;
- the relationship between level and falling speed int the 'levelSystem' function.

