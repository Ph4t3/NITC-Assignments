#include <limits.h>
#include <stdbool.h>
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

void copyPath(struct NodeList* from, struct NodeList** to)
{
    *to = NULL;
    while (from != NULL) {
        addNodeToPath(to, from->node);
        from = from->next_node;
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
                addNodeToPath(&table[i][j].path, i);
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

int minCost(RoutingTable table[], bool processed[], int n)
{
    int min = INT_MAX, min_idx;
    for (int i = 0; i < n; i++) {
        if (!processed[i] && min > table[i].cost) {
            min = table[i].cost;
            min_idx = i;
        }
    }
    return min_idx;
}

RoutingTable** LSR(Network* network)
{
    int n = network->nodes;
    RoutingTable** table = initTable(n);

    // For All Nodes
    for (int i = 0; i < n; i++) {
        bool processed[n];
        for (int j = 0; j < n; j++)
            processed[j] = false;

        // Apply Dijkstra's
        for (int j = 0; j < n - 1; j++) {
            int u = minCost(table[i], processed, n);
            processed[u] = true;

            Link* head = network->links[u];
            while (head != NULL) {
                if (table[i][u].cost != INT_MAX && table[i][u].cost + head->cost < table[i][head->dest].cost) {
                    table[i][head->dest].cost = head->cost + table[i][u].cost;
                    if (i != u)
                        copyPath(table[i][u].path, &table[i][head->dest].path);
                    addNodeToPath(&table[i][head->dest].path, head->dest);
                }
                head = head->next_link;
            }
        }
    }

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
            // Zero based indexing
            src -= 1;
            dest -= 1;

            addLink(network->links, src, dest, cost);
            addLink(network->links, dest, src, cost);
            i++;
        }
    }

    return network;
}

void printNetwork(Network* network)
{
    for (int i = 0; i < network->nodes; i++) {
        printf("\nLinks of Node %d\n\n", i + 1);
        Link* head = network->links[i];
        while (head != NULL) {
            printf("%d->%d\t%d\n", i + 1, head->dest + 1, head->cost);
            head = head->next_link;
        }
    }
}

void printTableRow(RoutingTable row, int start)
{
    NodeList* path = row.path;
    if (path == NULL)
        printf("NO PATH");
    else
        printf("%d", start);

    while (path != NULL) {
        printf("->%d", path->node + 1);
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
    /* printNetwork(network); */
    RoutingTable** table = LSR(network);
    printRoutingTable(table, network->nodes);
    return 0;
}
