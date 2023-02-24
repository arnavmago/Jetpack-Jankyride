# README for Jetpack Jankyride

## Commands

### To run the application -

In the home directory (i.e. the main directory after downloading and unzipping the code files, should have the src, libraries, include and fonts folder within it) run `mkdir build; cd build; cmake ..; make` or run them as the following one after the other `mkdir build`, `cd build`, `cmake ..`, `make`, this will create the build directory and create the executable file needed to run the application

After those commands are done executing a file called `app` will be created in the build directory, this is the executable file for the game. Run `./app` to execute the game

## Game mechanics

### The game has 3 levels that increase in difficulty, it measures score, distance covered and time spent. The levels change based on distance covered and your score is displayed at the end of the game (or after you die)

### Movement -

The character will always be moving to the right, you can hold spacebar to go up (replicating the flight motion) and leaving spacebar will cause the playerto fall but not at a standard rate (simulation gravity in graphics).

### Game objects -

### Coins -

Coins are small yellow randomly apperaing circles that increase your score upon collecting them, a better score is good. Each level spawns an additional coin allowing you a higher score but at a higher risk.

### Zappers

Zappers are the only onstacle in the game, they are randomly spawned and rotate at different speeds (with the speeds increasing with level), contact with the zapper causes the player to die.

## Weird features and information

Both the player and the Zappers will glow throughout the duration of the game

The game is heavy duty and will require a GPU to support its smooth running
