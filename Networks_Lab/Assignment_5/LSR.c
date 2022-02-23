#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

typedef struct Link {
    int dest, cost;
    struct Link* next_link;
} Link;

typedef struct Network {
    int nodes;
    struct Link** links;
} Network;

typedef struct NodeList {
    int node;
    struct NodeList* next_node;
} NodeList;

typedef struct RoutingTable {
    int cost;
    struct NodeList* path;
} RoutingTable;

void addNodeToPath(struct NodeList** path, int node)
{
    struct NodeList* new_node = malloc(sizeof(struct NodeList));
    new_node->node = node;
    new_node->next_node = NULL;

    if (*path == NULL) {
        *path = new_node;
    } else {
        struct NodeList* head = *path;
        while (head->next_node != NULL) {
            head = head->next_node;
        }
        head->next_node = new_node;
    }
}

RoutingTable** initTable(int n)
{
    RoutingTable** table = malloc(sizeof(RoutingTable*) * n);
    for (int i = 0; i < n; i++)
        table[i] = malloc(sizeof(RoutingTable) * n);

    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            table[i][j].path = NULL;
            if (i == j) {
                table[i][j].cost = 0;
                addNodeToPath(&table[i][j].path, i + 1);
            } else {
                table[i][j].cost = INT_MAX;
            }
        }
    }

    return table;
}

void addLink(Link** network, int src, int dest, int cost)
{
    Link* link = malloc(sizeof(Link));
    link->dest = dest;
    link->cost = cost;
    link->next_link = network[src];
    network[src] = link;
}

RoutingTable** LSR(Network* network)
{
    RoutingTable** table = initTable(network->nodes);
    return table;
}

Network* readInput()
{
    int n, m, src, dest, cost;
    scanf("%d %d", &n, &m);

    Network* network = malloc(sizeof(Network));
    network->nodes = n;
    network->links = malloc(sizeof(Link*) * n);
    for (int i = 0; i < n; i++)
        network->links[i] = NULL;

    int i = 0;
    while (i < m) {
        scanf("%d %d %d", &src, &dest, &cost);

        if (src <= 0 || src > n)
            printf("Invalid Source...\n");
        else if (dest <= 0 || dest > n)
            printf("Invalid Destination...\n");
        else if (cost < 0)
            printf("Invalid Cost...\n");
        else {
            addLink(network->links, src, dest, cost);
            addLink(network->links, dest, src, cost);
            i++;
        }
    }

    return network;
}

void printTableRow(RoutingTable row, int start)
{
    NodeList* path = row.path;
    if (path == NULL)
        printf("NO PATH");
    else
        printf("%d", start);

    while (path != NULL) {
        printf("->%d", path->node);
        path = path->next_node;
    }

    if (row.cost == INT_MAX)
        printf("\t\tinf\n");
    else
        printf("\t\t%d\n", row.cost);
}

void printRoutingTable(RoutingTable** table, int n)
{
    for (int i = 0; i < n; i++) {
        printf("\nRouting Table at Node %d\n\n", i + 1);
        for (int j = 0; j < n; j++) {
            printTableRow(table[i][j], i + 1);
        }
    }
}

int main()
{
    Network* network = readInput();
    RoutingTable** table = LSR(network);
    printRoutingTable(table, network->nodes);
    return 0;
}
