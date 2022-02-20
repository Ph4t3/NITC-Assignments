import java.util.Random;

class Environment {
    char agentPosition;
    // True means dirty
    Boolean blockAStatus, blockBStatus;
    Environment(char agentPosition, Boolean blockAStatus, Boolean blockBStatus) {
        this.agentPosition = agentPosition;
        this.blockAStatus = blockAStatus;
        this.blockBStatus = blockBStatus;
    }
}

class SimpleReflexAgent {
    Integer performanceMeasure = 0;
    Random random;
    Integer steps = 1000;
    Environment env;

    SimpleReflexAgent(Environment env) {
        this.env = env;
        this.random = new Random();
    }

    void clean() {
        String output;
        output = "Agent at " + env.agentPosition + ". ";
        output += "Dirt configuration: " +
                  env.blockAStatus.toString() + " " +
                  env.blockBStatus.toString() + ". ";

        for (int i = 0; i < steps; i++) {
            if (env.agentPosition == 'A') {
                if (env.blockAStatus) suck();
            } else {
                if (env.blockBStatus) suck();
            }

            if (random.nextBoolean()) left();
            else right();
        }

        output += "Performance Measure: " + performanceMeasure;
        System.out.println(output);
    }

    void left() {
        if (env.agentPosition == 'B')
            env.agentPosition = 'A';
    }

    void right() {
        if (env.agentPosition == 'A')
            env.agentPosition = 'B';
    }

    void suck() {
        performanceMeasure += 1;
        switch (env.agentPosition) {
        case 'A':
            env.blockAStatus = false;
            break;
        case 'B':
            env.blockBStatus = false;
            break;
        }
    }
}

public class Q1 {
    public static void main(String[] args) {
        Environment[] testCases = {
            new Environment('A', true, true),
            new Environment('A', true, false),
            new Environment('A', false, true),
            new Environment('A', false, false),
            new Environment('B', true, true),
            new Environment('B', true, false),
            new Environment('B', false, true),
            new Environment('B', false, false)
        };

        // Run all possible testcases
        for (Environment var : testCases) {
            SimpleReflexAgent agent = new SimpleReflexAgent(var);
            agent.clean();
        }
    }
}
