# AI Assignment 1

## Nikhil R - B180283CS

The codes are written in Java and can be normally compiled

## Question 1

#### Design
- Environment class is used to define the current environment and agent position
- Each possible environment is passed to the SimpleReflexAgent class in which we have added the logic for our Vacum cleaner bot
- The bot iterates for 1000 time steps and randomly moves left or right, each time cleaning if dirt is present
- For each time Suck() is called, the performanceMeasure is increased.

#### Sample Output 1
![Screenshot 1](https://raw.githubusercontent.com/Ph4t3/NITC-Assignments/master/Artificial%20Intelligence/Assignment_1/Q1/ss1.png)

#### State  Space Search Graph
![State Space Graph](https://raw.githubusercontent.com/Ph4t3/NITC-Assignments/master/Artificial%20Intelligence/Assignment_1/Q1/StateSpaceSearchGraph.png)

## Question 2

#### Design
- User input for current stones in the piles is taken
- This is passed to a State Object which recursively build the minimax tree and finds optimal solution
- Depending on user move, the current state is changed.
- The State Object calculates the optimal move given the current state
- The optimal move by computer is printed to the screen
- This is continued until end state is reached, and the winner is declared.

#### Sample Output 1
![Screenshot 1](https://raw.githubusercontent.com/Ph4t3/NITC-Assignments/master/Artificial%20Intelligence/Assignment_1/Q2/ss1.png)

#### Sample Output 2
![Screenshot 2](https://raw.githubusercontent.com/Ph4t3/NITC-Assignments/master/Artificial%20Intelligence/Assignment_1/Q2/ss2.png)

