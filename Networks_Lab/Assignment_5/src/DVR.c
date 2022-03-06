#include <stdio.h>
#include <stdlib.h>

typedef struct Link {
    int src, dest, cost;
} Link;

typedef struct RoutingTable {
    int next_hop, cost;
} RoutingTable;


// Returns a table with default values
// -1 = inf
RoutingTable** initTable(int n)
{
    RoutingTable** table = malloc(sizeof(RoutingTable*) * n);
    for (int i = 0; i < n; i++)
        table[i] = malloc(sizeof(RoutingTable) * n);

    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            if (i == j) {
                table[i][j].next_hop = i;
                table[i][j].cost = 0;
            } else {
                table[i][j].cost = -1;
                table[i][j].next_hop = -1;
            }
        }
    }

    return table;
}

// Checks if a > b, where -1 = inf
int greaterThan(int a, int b)
{
    if ((a == -1 && b != -1) || a > b)
        return 1;
    return 0;
}

RoutingTable** DVR(Link* links, int n, int m)
{
    RoutingTable** table = initTable(n);

    // For Each Node
    for (int i = 0; i < n; i++) {
        // Apply Bellman Ford Algorithm
        for (int j = 0; j < n - 1; j++) {
            for (int k = 0; k < m; k++) {
                // Input indexing is at 1
                int src = links[k].src - 1;
                int dest = links[k].dest - 1;

                if (table[i][src].cost != -1) {
                    int connected_cost = table[i][src].cost + links[k].cost;
                    int next_hop = table[i][src].next_hop;

                    if (greaterThan(table[i][dest].cost, connected_cost)) {
                        table[i][dest].cost = connected_cost;
                        table[i][dest].next_hop = src == i ? dest : next_hop;
                    }
                }
            }
        }
    }

    return table;
}

// Prints the Routing Table
// Indexing starts at 0, so +1 is added everywhere
void printRoutingTable(RoutingTable** table, int n)
{
    for (int i = 0; i < n; i++) {
        printf("\nRouting Table at Node %d\n\n", i + 1);
        for (int j = 0; j < n; j++) {
            printf("%d\t%d\t%d\n", j + 1, table[i][j].next_hop + 1, table[i][j].cost);
        }
    }
}

int main()
{
    int n, m;
    scanf("%d %d", &n, &m);

    // Since its undirected edges
    m *= 2;

    Link* links = malloc(sizeof(Link) * m);

    for (int i = 0; i < m;) {
        scanf("%d %d %d", &(links[i].src), &(links[i].dest), &(links[i].cost));

        // Since its undirected edges
        links[i + 1].src = links[i].dest;
        links[i + 1].dest = links[i].src;
        links[i + 1].cost = links[i].cost;

        if (links[i].src <= 0 || links[i].src > n)
            printf("Invalid Source...\n");
        else if (links[i].dest <= 0 || links[i].dest > n)
            printf("Invalid Destination...\n");
        else if (links[i].cost < 0)
            printf("Invalid Cost...\n");
        else
            i += 2;
    }

    RoutingTable** table = DVR(links, n, m);
    printRoutingTable(table, n);
    return 0;
}
