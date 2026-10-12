/*
 * DSAL Mini Project : Shortest Path using Dijkstra's Algorithm
 * 52 locations (13x4 map), normal and bidirectional Dijkstra.
 *
 * Compile : gcc main.c -o main
 */

#include <stdio.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>    /* only used to switch the console to UTF-8 + colours */
#endif

#define ROWS 13
#define COLS 4
#define N    (ROWS * COLS)   /* 52 */
#define INF  1000000

/* ANSI colour codes */
#define RESET   "\033[0m"
#define GREY    "\033[90m"
#define CYAN    "\033[36m"
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"
#define MAGENTA "\033[35m"
#define BOLD    "\033[1m"

char NAME[N][4];      /* "A1" .. "M4" */
int  graph[N][N];     /* 0 = no road */

/* name each location: row * 4 + column */
void buildNames(void) {
    int r, c, i = 0;
    for (r = 0; r < ROWS; r++) {
        for (c = 0; c < COLS; c++) {
            sprintf(NAME[i], "%c%d", 'A' + r, c + 1);
            i++;
        }
    }
}

/* join each location to its right and below neighbour */
void buildGraph(void) {
    int r, c, u, v;

    for (r = 0; r < ROWS; r++) {
        for (c = 0; c < COLS; c++) {
            u = r * COLS + c;
            if (c + 1 < COLS) {
                v = r * COLS + (c + 1);
                graph[u][v] = graph[v][u] = 3 + ((r * 2 + c * 3) % 7);
            }
            if (r + 1 < ROWS) {
                v = (r + 1) * COLS + c;
                graph[u][v] = graph[v][u] = 3 + ((r * 3 + c * 5) % 7);
            }
        }
    }
}

/* let the console print UTF-8 and colours; does nothing on other systems */
void setupConsole(void) {
#ifdef _WIN32
    HANDLE out = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD mode;

    SetConsoleOutputCP(65001);
    if (GetConsoleMode(out, &mode))
        SetConsoleMode(out, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
#endif
}

/* colour for one place: 2 = start, 3 = goal, 1 = on the route, 0 = ordinary */
const char *nameColor(int m) {
    if (m == 2) return BOLD YELLOW;
    if (m == 3) return BOLD MAGENTA;
    if (m == 1) return BOLD GREEN;
    return "";
}

/* draw the map; mark[] says which place is the start, the goal or on the route */
void printGraph(int mark[]) {
    int r, c, u, v, m;

    printf("Map: %d rows x %d columns = %d locations\n", ROWS, COLS, N);
    if (mark != NULL)
        printf(BOLD YELLOW "start" RESET "   " BOLD MAGENTA "goal" RESET "   "
               BOLD GREEN "on the route" RESET "\n");
    printf("The number on a road is its distance.\n\n");

    for (r = 0; r < ROWS; r++) {
        /* edge above this row: the roads coming down from the row before */
        printf(GREY "%s" RESET, r == 0 ? "┌" : "├");
        for (c = 0; c < COLS; c++) {
            if (r == 0) {
                printf(GREY "───────" RESET);
            } else {
                u = (r - 1) * COLS + c;
                v = r * COLS + c;
                printf(GREY "───" RESET CYAN "%d" RESET GREY "───" RESET, graph[u][v]);
            }
            if (c + 1 < COLS) printf(GREY "%s" RESET, r == 0 ? "┬" : "┼");
            else              printf(GREY "%s" RESET, r == 0 ? "┐" : "┤");
        }
        printf("\n");

        /* the locations, each with the road to its right */
        printf(GREY "│" RESET);
        for (c = 0; c < COLS; c++) {
            u = r * COLS + c;
            m = (mark == NULL) ? 0 : mark[u];
            printf(" %s%s " RESET, nameColor(m), NAME[u]);
            if (c + 1 < COLS) {
                printf(CYAN " %d " RESET, graph[u][u + 1]);
            } else {
                printf("   ");
            }
            printf(GREY "│" RESET);
        }
        printf("\n");
    }

    printf(GREY "└" RESET);
    for (c = 0; c < COLS; c++) {
        printf(GREY "───────" RESET);
        printf(GREY "%s" RESET, (c + 1 < COLS) ? "┴" : "┘");
    }
    printf("\n");
}

/* "b2" or "B2" -> location number, or -1 */
int findNode(const char *name) {
    char wanted[4];
    int i;

    for (i = 0; name[i] != '\0' && i < 3; i++) {
        wanted[i] = name[i];
        if (wanted[i] >= 'a' && wanted[i] <= 'z') wanted[i] -= 32;
    }
    wanted[i] = '\0';

    for (i = 0; i < N; i++) {
        if (strcmp(NAME[i], wanted) == 0) return i;
    }
    return -1;
}

void printPath(int path[], int length) {
    int i;
    printf(BOLD GREEN);
    for (i = 0; i < length; i++) {
        printf("%s", NAME[path[i]]);
        if (i < length - 1) printf(" -> ");
    }
    printf(RESET);
}

/* O(N^2): settle the cheapest unfinished location each round, relax its neighbours */
int dijkstra(int start, int goal, int prev[], int *settledCount) {
    int dist[N], settled[N];
    int i, step;

    for (i = 0; i < N; i++) {
        dist[i] = INF;
        settled[i] = 0;
        prev[i] = -1;
    }
    dist[start] = 0;
    *settledCount = 0;

    for (step = 0; step < N; step++) {
        int u = -1;
        for (i = 0; i < N; i++) {
            if (settled[i] == 0 && dist[i] < INF) {
                if (u == -1 || dist[i] < dist[u]) u = i;
            }
        }
        if (u == -1) break;

        settled[u] = 1;
        (*settledCount)++;

        if (u == goal) break;

        for (i = 0; i < N; i++) {
            if (graph[u][i] != 0 && dist[u] + graph[u][i] < dist[i]) {
                dist[i] = dist[u] + graph[u][i];
                prev[i] = u;
            }
        }
    }
    return dist[goal];
}

int rebuildPath(int start, int goal, int prev[], int path[]) {
    int backward[N];
    int length = 0, i, cur = goal;

    while (cur != -1) {
        backward[length++] = cur;
        cur = prev[cur];
    }
    if (backward[length - 1] != start) return 0;

    for (i = 0; i < length; i++) {
        path[i] = backward[length - 1 - i];
    }
    return length;
}

/* search from both ends; prevB[x] points towards the goal */
int bidirectionalDijkstra(int start, int goal, int path[], int *length,
                          int *settledF, int *settledB) {
    int distF[N], distB[N];
    int doneF[N], doneB[N];
    int prevF[N], prevB[N];
    int best = INF;
    int meet = -1;
    int i;

    for (i = 0; i < N; i++) {
        distF[i] = distB[i] = INF;
        doneF[i] = doneB[i] = 0;
        prevF[i] = prevB[i] = -1;
    }
    distF[start] = 0;
    distB[goal] = 0;

    while (1) {
        int u = -1, v = -1;
        for (i = 0; i < N; i++) {
            if (doneF[i] == 0 && distF[i] < INF && (u == -1 || distF[i] < distF[u])) u = i;
            if (doneB[i] == 0 && distB[i] < INF && (v == -1 || distB[i] < distB[v])) v = i;
        }
        if (u == -1 && v == -1) break;

        /* stop only when the frontiers can no longer beat `best` */
        if (u != -1 && v != -1 && distF[u] + distB[v] >= best) break;

        /* expand the closer side */
        if (u != -1 && (v == -1 || distF[u] <= distB[v])) {
            doneF[u] = 1;
            for (i = 0; i < N; i++) {
                if (graph[u][i] != 0 && distF[u] + graph[u][i] < distF[i]) {
                    distF[i] = distF[u] + graph[u][i];
                    prevF[i] = u;
                }
            }
        } else {
            doneB[v] = 1;
            for (i = 0; i < N; i++) {
                if (graph[v][i] != 0 && distB[v] + graph[v][i] < distB[i]) {
                    distB[i] = distB[v] + graph[v][i];
                    prevB[i] = v;
                }
            }
        }

        /* best meeting point = cheapest route through a shared location */
        for (i = 0; i < N; i++) {
            if (distF[i] < INF && distB[i] < INF && distF[i] + distB[i] < best) {
                best = distF[i] + distB[i];
                meet = i;
            }
        }
    }

    *settledF = 0;
    *settledB = 0;
    for (i = 0; i < N; i++) {
        if (doneF[i]) (*settledF)++;
        if (doneB[i]) (*settledB)++;
    }

    if (best == INF) return -1;

    /* start -> meet from prevF, then meet -> goal from prevB */
    {
        int backward[N], nBack = 0, cur = meet, k = 0;
        while (cur != -1) {
            backward[nBack++] = cur;
            cur = prevF[cur];
        }
        for (i = nBack - 1; i >= 0; i--) path[k++] = backward[i];
        cur = prevB[meet];
        while (cur != -1) {
            path[k++] = cur;
            cur = prevB[cur];
        }
        *length = k;
        return best;
    }
}

int main(void) {
    char line[64], firstName[16], secondName[16];
    int start, goal, words, length, cost, settled;
    int prev[N], path[N];

    setupConsole();
    buildNames();
    buildGraph();
    printGraph(NULL);

    printf("\nType two locations to connect, e.g.  A1 M4\n");
    printf("Press Enter on its own for the default A1 -> M4, or type q to quit.\n");

    while (1) {
        printf("\n> ");
        if (fgets(line, sizeof(line), stdin) == NULL) break;   /* end of input */
        if (line[0] == 'q' || line[0] == 'Q') break;

        words = sscanf(line, "%15s %15s", firstName, secondName);
        if (words != 2) {          /* Enter on its own: use the default pair */
            strcpy(firstName, "A1");
            strcpy(secondName, "M4");
        }

        start = findNode(firstName);
        goal  = findNode(secondName);
        if (start == -1 || goal == -1) {
            printf("Unknown location - use names like A1, c3 or M4.\n");
            continue;
        }
        if (start == goal) {
            printf("Start and goal are the same place!\n");
            continue;
        }

        printf("\n================ FINDING ROUTE %s -> %s ================\n",
               NAME[start], NAME[goal]);

        cost = dijkstra(start, goal, prev, &settled);
        length = rebuildPath(start, goal, prev, path);
        if (length == 0) {
            printf("There is no route between these locations.\n");
            continue;
        }
        printf("\n" BOLD CYAN "NORMAL DIJKSTRA" RESET "\n  route : ");
        printPath(path, length);
        printf("\n  cost  : %d\n", cost);
        printf("  places settled: %d (out of %d)\n", settled, N);

        {
            int path[2 * N], cost, settledF, settledB, length;
            cost = bidirectionalDijkstra(start, goal, path, &length, &settledF, &settledB);
            if (cost == -1) {
                printf("\nThere is no route between these locations.\n");
                continue;
            }

            printf("\n" BOLD CYAN "BIDIRECTIONAL DIJKSTRA" RESET "\n  route : ");
            printPath(path, length);
            printf("\n  cost  : %d\n", cost);
            printf("  places settled: %d forward + %d backward = %d (out of %d)\n",
                   settledF, settledB, settledF + settledB, N);
        }

        printf("\nBoth algorithms found the same route, as they should.\n");

        /* draw the map again, with this route marked on it */
        {
            int i, mark[N];
            for (i = 0; i < N; i++) mark[i] = 0;
            for (i = 0; i < length; i++) mark[path[i]] = 1;
            mark[start] = 2;
            mark[goal]  = 3;
            printf("\n");
            printGraph(mark);
        }
    }

    printf("\nGoodbye.\n");
    return 0;
}
