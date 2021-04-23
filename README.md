
# GrandmAgenda

A simple daily agenda for a forgetful grandmother. Tested on Ubuntu 20.04 LTS.

---------------------------------------------------------------------------------------------------------

## Requirements

*   make (`sudo apt-get install make`)

*   CMake (`sudo apt-get install cmake`)

---------------------------------------------------------------------------------------------------------

## Build

Clone or download the repository and type the following commands in the root folder:

```mkdir build```

```cd build```

```cmake ..```

```make```

---------------------------------------------------------------------------------------------------------

## Usage

To run the application, navigate to the build folder and type:

```./GrandmAgenda [filepath] [speed factor]```

### Argument `filepath`
The full path to a file containing activities. Requirements for the file: 
* No overlapping, no free slot. 
* Format: See the example in the scenarios/activities.txt

### Argument `speed factor`
How much faster than real world time, the time in the program should progress, according to:

```1 min in program time = (60 / speed factor) seconds in real time```

No upper bound is applied, so you can go crazy, if you want to quickly pass through the entire day.
But, due to the 3 secs printing interval, be prepared for some weird output sequence after a limit.

The program asks for the initial time in the beginning. Just type "now" for the real-world experience.
For testing purposes, you can input any time of the day you want.

---------------------------------------------------------------------------------------------------------

## Contact

adam.pasvatis@outlook.com

---------------------------------------------------------------------------------------------------------