import java.util.*;

enum Player {
    MAX, MIN
}

class State implements Comparable<State> {
    Integer pile1, pile2, minimaxValue;
    Player player;
    List<State> children;

    State(Integer pile1, Integer pile2, Player player) {
        this.pile1 = pile1;
        this.pile2 = pile2;
        this.player = player;
        this.children = new ArrayList<State>();

        if (pile1 == 0 && pile2 == 0) {
            minimaxValue = (player == Player.MAX ? 0 : 1);
        } else {
            calculateChildren();
        }
    }

    void calculateChildren() {
        Player nextPlayer = (player == Player.MIN ? Player.MAX : Player.MIN);
        for (Integer i = pile1 - 1; i >= 0; i--)
            children.add(new State(i, pile2, nextPlayer));

        for (Integer i = pile2 - 1; i >= 0; i--)
            children.add(new State(pile1, i, nextPlayer));

        minimaxValue = (
                           player == Player.MIN ?
                           Collections.min(children) : Collections.max(children)
                       ).minimaxValue;
    }

    Boolean endState() {
        if (pile1 == 0 && pile2 == 0)
            return true;
        return false;
    }

    State findChildren(Integer x, Integer y)  {
        for (State state : children) {
            if (state.pile1 == x && state.pile2 == y)
                return state;
        }

        return null;
    }

    @Override
    public int compareTo(State s) {
        return (minimaxValue < s.minimaxValue) ? -1 : (minimaxValue == s.minimaxValue ? 0 : 1);
    }
}

public class Q2 {
    public static void main(String[] args) {
        Scanner scan = new Scanner(System.in);
        System.out.print("Initial stones in the piles(x, y): ");
        Integer pile1 = scan.nextInt();
        Integer pile2 = scan.nextInt();
        System.out.print("\nWho is playing first? (1 = Computer, 0 = Human): ");
        Integer choice = scan.nextInt();

        if (choice == 0) {
            System.out.print("\nEnter your move (Pile, Stones): ");
            if (scan.nextInt() == 1)
                pile1 -= scan.nextInt();
            else
                pile2 -= scan.nextInt();
            System.out.println("Current State: " + pile1 + " " + pile2);
        }

        State currentState = new State(pile1, pile2, Player.MAX);
        while (!currentState.endState()) {
            State newState = Collections.max(currentState.children);

            if (newState.pile1 == currentState.pile1)
                System.out.println("\nComputer's Move : 2 " + (currentState.pile2 - newState.pile2));
            else
                System.out.println("\nComputer's Move : 1 " + (currentState.pile1 - newState.pile1));

            currentState = newState;
            System.out.println("Current State: " + currentState.pile1 + " " + currentState.pile2);

            if (currentState.endState()) break;

            System.out.print("\nEnter your move (Pile, Stones): ");
            if (scan.nextInt() == 1)
                currentState = currentState.findChildren(currentState.pile1 - scan.nextInt(), currentState.pile2);
            else
                currentState = currentState.findChildren(currentState.pile1, currentState.pile2 - scan.nextInt());

            System.out.println("Current State: " + currentState.pile1 + " " + currentState.pile2);
        }

        if (currentState.player == Player.MAX)
            System.out.println("\nHuman - Won, Computer - Lost!!\nEvaluation Score: 1");
        else if (currentState.player == Player.MIN)
            System.out.println("\nHuman - Lost, Computer - Won!!\nEvaluation Score: -1");
        scan.close();
    }
}
