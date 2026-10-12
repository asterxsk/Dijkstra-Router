# Dijkstra Router

![Language: C](https://img.shields.io/badge/language-C-00599C?logo=c&logoColor=white)

A small C program that finds the shortest route between two of 52 locations on a 13 × 4 grid, using plain Dijkstra and bidirectional Dijkstra side by side.

The whole program is one file, `main.c`. It uses the standard headers `stdio.h` and `string.h`, plus `windows.h` on Windows only (to switch the console to UTF-8 and colours), prints the map, then prints the route and cost found by both algorithms. They always agree on the cost, though on some ties they take different, equally cheap routes.

## The map

This is the actual output of the program. The number on a road is its distance, and every road goes both ways (the graph is undirected).

```text
Map: 13 rows x 4 columns = 52 locations
The number on a road is its distance.

┌───────┬───────┬───────┬───────┐
│ A1  3 │ A2  6 │ A3  9 │ A4    │
├───3───┼───8───┼───6───┼───4───┤
│ B1  5 │ B2  8 │ B3  4 │ B4    │
├───6───┼───4───┼───9───┼───7───┤
│ C1  7 │ C2  3 │ C3  6 │ C4    │
├───9───┼───7───┼───5───┼───3───┤
│ D1  9 │ D2  5 │ D3  8 │ D4    │
├───5───┼───3───┼───8───┼───6───┤
│ E1  4 │ E2  7 │ E3  3 │ E4    │
├───8───┼───6───┼───4───┼───9───┤
│ F1  6 │ F2  9 │ F3  5 │ F4    │
├───4───┼───9───┼───7───┼───5───┤
│ G1  8 │ G2  4 │ G3  7 │ G4    │
├───7───┼───5───┼───3───┼───8───┤
│ H1  3 │ H2  6 │ H3  9 │ H4    │
├───3───┼───8───┼───6───┼───4───┤
│ I1  5 │ I2  8 │ I3  4 │ I4    │
├───6───┼───4───┼───9───┼───7───┤
│ J1  7 │ J2  3 │ J3  6 │ J4    │
├───9───┼───7───┼───5───┼───3───┤
│ K1  9 │ K2  5 │ K3  8 │ K4    │
├───5───┼───3───┼───8───┼───6───┤
│ L1  4 │ L2  7 │ L3  3 │ L4    │
├───8───┼───6───┼───4───┼───9───┤
│ M1  6 │ M2  9 │ M3  5 │ M4    │
└───────┴───────┴───────┴───────┘
```

## Build and run

```bash
gcc -Wall -Wextra main.c -o main
./main            # on Windows run main.exe
```

The file `.zed/tasks.json` holds the same thing as a Zed task named `C: Compile and Run`:

```text
gcc main.c -Wall -Wextra -o main.exe ; ./main.exe
```

The program compiles with no warnings.

## Usage

At the `>` prompt, type two location names separated by a space, for example `A1 M4`.

- A name is a row letter `A`–`M` followed by a column number `1`–`4`.
- Names are not case sensitive, so `c3` and `C3` both work.
- Press Enter on its own to use the default pair `A1 M4`.
- Type `q` (upper or lower case) to quit. The program prints `Goodbye.` and exits.
- If a name is unknown, or the two names are the same place, the program explains the problem and asks again.

## A real example run

These are captured runs of the program. For `A1 -> M4` (opposite corners) the cheapest route costs 77, and for `G3 -> K2` it costs 28. In both cases the two algorithms print the same route, and as always they agree on the cost.

```text
================ FINDING ROUTE A1 -> M4 ================

NORMAL DIJKSTRA
  route : A1 -> B1 -> B2 -> C2 -> C3 -> D3 -> E3 -> F3 -> G3 -> H3 -> I3 -> I4 -> J4 -> K4 -> L4 -> M4
  cost  : 77
  places settled: 52 (out of 52)

BIDIRECTIONAL DIJKSTRA
  route : A1 -> B1 -> B2 -> C2 -> C3 -> D3 -> E3 -> F3 -> G3 -> H3 -> I3 -> I4 -> J4 -> K4 -> L4 -> M4
  cost  : 77
  places settled: 26 forward + 25 backward = 51 (out of 52)

Both algorithms found the same route, as they should.
```

```text
================ FINDING ROUTE G3 -> K2 ================

NORMAL DIJKSTRA
  route : G3 -> H3 -> H2 -> I2 -> J2 -> K2
  cost  : 28
  places settled: 34 (out of 52)

BIDIRECTIONAL DIJKSTRA
  route : G3 -> H3 -> H2 -> I2 -> J2 -> K2
  cost  : 28
  places settled: 14 forward + 12 backward = 26 (out of 52)

Both algorithms found the same route, as they should.
```

## How it works

### The grid and how locations are named

The map is 13 rows named `A` to `M` and 4 columns numbered `1` to `4`, so there are 13 × 4 = **52** locations. Each location is named for where it sits: `A1` is the top-left corner and `M4` the bottom-right.

Inside the code each location also has a plain number, using one rule:

```text
number = row * 4 + column        (both counting from 0)
```

So `A1` = 0, `A2` = 1, `A3` = 2, `A4` = 3, `B1` = 4, and so on up to `M4` = 51. `buildNames()` turns a number back into a label with `sprintf(NAME[i], "%c%d", 'A' + r, c + 1)`.

### Where the road distances come from

Every location is joined to the one on its right and the one below it. The distance is worked out by a small formula rather than typed in by hand:

```c
graph[u][v] = 3 + ((r * 2 + c * 3) % 7);   /* road to the right */
graph[u][v] = 3 + ((r * 3 + c * 5) % 7);   /* road downward     */
```

The `% 7` means "the remainder after dividing by 7", which is always in the range 0–6. Adding 3 makes every distance fall between **3 and 9**, and the two formulas mix the values up so the cheapest route tends to zig-zag instead of running straight across.

A hand-written list would be 87 roads, which is long and easy to mistype; the formula does the same job in four lines. **The distances are synthetic** — invented by a formula, not measured from a real map. The program is demonstrating an algorithm, not solving a real routing problem. If you wanted a hand-written edge list, the formula is the only thing you would replace; the algorithms do not care where the numbers come from.

One more detail: in the code **0 means "no road"**. That is safe here only because every real distance is 3 or more. A real program would use a separate flag, because a distance of 0 can be legal.

### Connectivity

The grid is fully connected, so every location can reach every other. That is worth stating, because "no route" is a real possible answer for a general graph — which is why the code checks for it and prints a message instead of a route.

### How the graph is stored: an adjacency matrix

`graph[52][52]` is a table where `graph[u][v]` is the distance between `u` and `v`. It holds 2704 `int`s, about **10.8 KB**. The graph is undirected, so each road is stored twice, and most of the table is 0.

Counting the roads: each of the 13 rows has 3 horizontal roads (39) and each pair of neighbouring rows has 4 vertical roads (12 × 4 = 48), so there are **87 roads**, each stored in two cells. That fills 174 of the 2704 cells, so the matrix is about 6% full and 94% zeros — it is *sparse*.

| | Adjacency matrix (used here) | Adjacency list |
|---|---|---|
| Memory for 52 locations | 2704 entries | ~87 entries (one per road) |
| Look up "is `u` joined to `v`?" | instant, `graph[u][v]` | walk `u`'s list |
| Iterate `u`'s neighbours | scan all 52, most of them zero | walk the list |

The matrix costs O(N²) memory no matter how few roads there are, and an adjacency list would be the right choice for a real, sparse road network. It is used here because `graph[u][v]` is one simple line of code and is much easier to read and explain.

### Plain Dijkstra, step by step

**The problem it solves:** every road has a cost, and we want the cheapest total cost from one location to another.

Dijkstra works because of one fact: **if you always settle the cheapest unfinished location, its distance is final.** You could not have reached it more cheaply through some other location, because that other location would itself cost at least as much.

The code keeps three arrays:

- `dist[x]` — the cheapest distance to `x` found *so far*.
- `settled[x]` — 1 once we are certain `dist[x]` is final.
- `prev[x]` — the location we came from, so the route can be rebuilt afterwards.

The loop:

1. Pick the unsettled location `u` with the smallest `dist[]`. This is a scan of all 52, so O(N) per round.
2. Mark `u` settled — it is done forever. If `u` is the goal, stop.
3. **Relax** each neighbour: if `dist[u] + weight(u, v)` is less than `dist[v]`, a cheaper way into `v` has been found, so update `dist[v]` and set `prev[v] = u`.
4. Repeat. There are at most N rounds.

Total running time is **O(N²)** ≈ 52 × 52 ≈ 2700 simple steps, from N rounds each scanning N distances. Relaxing neighbours is another N per round, the same order, so it does not change the O(N²).

#### Worked example: A1 → A4 (same row)

| Step | Settle (distance) | Distance updates |
|---|---|---|
| start | — | `dist[A1] = 0`, all others ∞ |
| 1 | A1 (0) | A2 ← 3, B1 ← 3 |
| 2 | A2 (3) | A3 ← 3 + 6 = 9, B2 ← 3 + 8 = 11 |
| 3 | B1 (3) | B2 ← 3 + 5 = 8 (**improves** the 11), C1 ← 3 + 6 = 9 |
| 4 | B2 (8) | B3 ← 8 + 8 = 16, C2 ← 8 + 4 = 12 |
| 5 | A3 (9) | **A4 ← 9 + 9 = 18** (the answer), B3 ← 9 + 6 = 15 (**improves** the 16) |
| 6 | C1 (9) | D1 ← 9 + 9 = 18 |
| 7 | C2 (12) | C3 ← 12 + 3 = 15, D2 ← 12 + 7 = 19 |
| 8 | B3 (15) | B4 ← 15 + 4 = 19 |
| 9 | C3 (15) | C4 ← 15 + 6 = 21, D3 ← 15 + 5 = 20 |
| 10 | A4 (18) | goal settled — stop |

The answer arrives at step 5, when A3 finds a route to A4 costing 18. A4 is **not** settled then, because 18 is not yet the cheapest un-settled distance: C1 (9), C2 (12), B3 (15) and C3 (15) are all cheaper, so the loop deals with those first and A4 waits until step 10. This is the greedy rule doing real work, and it is why the plain search settles 10 locations to answer a question about a location it found at step 5.

Steps 3 and 5 also show a distance being *improved* (B2 from 11 to 8, B3 from 16 to 15). That is the whole point of the relaxation step: a location's distance is only best-so-far until it is settled.

The program confirms this route: `A1 -> A2 -> A3 -> A4`, cost **18**, and the plain search settles **10 of 52** locations. The result runs straight along the top row.

### Bidirectional Dijkstra

**The idea.** A normal search from `A1` spreads out in all directions, and a search from `M4` spreads out too. Run both at the same time and they meet in the middle. Each search then covers about half the radius, which on a big map is a large saving: the number of locations within radius 10 of a point is about four times the number within radius 5, so one search of radius 10 looks at roughly four times as many as two searches of radius 5 put together — a saving of about half.

**What we keep.** Two sets of arrays, one per direction:

- forward: `distF[]`, `prevF[]` — grown from the **start**
- backward: `distB[]`, `prevB[]` — grown from the **goal**

`prevB[x]` points *towards* the goal, not away from it, so it is the parent in the reversed graph.

**Meeting point.** Any location `x` that both searches have reached gives a complete route of length

```text
distF[x] + distB[x]        (start → x, then x → goal)
```

The best `x` over all reached locations lies on the shortest path.

**Each round of the loop:**

1. Find the cheapest unfinished location on each side: `u` (forward) and `v` (backward).
2. **Stopping test** — if `distF[u] + distB[v] >= best`, stop. Any route through an unexplored location must pass through one of these two frontier locations and pay at least that much, so it cannot beat `best`.
3. Expand the side whose frontier is closer — the smaller of `distF[u]` and `distB[v]`. This keeps the two searches balanced so they meet in the middle instead of one racing ahead.
4. After each expansion, check all locations for a better meeting point and update `best` and `meet`.
5. When finished, rebuild the route in two halves: start → meet from `prevF`, then meet → goal from `prevB`.

#### What actually happens on this map

The program prints the settled counts, so these numbers are measured, not estimated. "Places settled" is how many locations the search had to finalise before it could be sure of the answer — fewer means less work.

| Route | Plain settles | Bidirectional settles | Saving |
|---|---|---|---|
| `A1 → A4` (same row, short) | 10 / 52 | 4 + 3 = 7 | 30% |
| `M1 → M4` (same row, short) | 10 / 52 | 3 + 2 = 5 | 50% |
| `G3 → K2` (medium diagonal) | 34 / 52 | 14 + 12 = 26 | 24% |
| `A4 → M1` (opposite corners) | 52 / 52 | 27 + 21 = 48 | 8% |
| `A1 → M4` (opposite corners) | 52 / 52 | 26 + 25 = 51 | 2% |

The saving is real, but it is much smaller than the "twice as fast" figure you often read, and the reason is worth explaining.

- **This map is long and thin** — 13 rows but only 4 columns. The two searches can only meet across the narrow direction, so they both still sweep most of the *length* of the map. On a square map with the same 52 locations the meeting would be far more effective.
- **The worst case is opposite corners.** The searches start at the two most distant points, so each still covers about half the map, and 26 + 25 = 51 is almost no saving at all. Bidirectional search is not a guaranteed speed-up: it is best when the two ends are close across the "width" of the graph, and worst when they are far apart in every direction.

#### Worked example: G3 → K2

The forward search from G3 and the backward search from K2 meet at `I2` — two rows below `G3` (`G` → `H` → `I`) and one column to its left, which is about where the middle of a route should fall. The plain search settles 34 locations; the two searches together settle 26 (14 forward, 12 backward). Both report cost **28**, and here the program prints the same route for both. The guarantee that always holds is on the *cost*: the two algorithms agree on how much the trip costs. When several different routes tie on cost they can pick different ones. `A4 → M1` is an example — both return **77**, but plain Dijkstra goes `A4 → B4 → C4 → D4 → E4 → E3 → F3 → G3 → H3 → H2 → H1 → I1 → J1 → K1 → L1 → M1`, while the bidirectional search goes `A4 → B4 → C4 → D4 → E4 → E3 → F3 → G3 → G2 → H2 → I2 → J2 → K2 → L2 → M2 → M1`. (The program's closing line, `Both algorithms found the same route, as they should.`, is printed unconditionally, so on a tie like this it is only literally true of the cost.)

#### Does stopping early really matter?

Yes, and this map shows it happening. For **A1 → M4** the route found costs 77 and the two searches finally meet at `G3` — but the *first* location that both searches reach is `H1`. `H1` is not on the shortest route; it sits off to the left as the two searches sweep past each other. A program that stopped the moment the searches touched would take `H1` as the junction and report a route longer than 77.

The stop test in the code is the one that is safe: keep going until `distF[u] + distB[v] >= best`, where `best` is the cheapest complete route found so far. On A1 → M4 the test is reached when the two frontier distances are `39` and `38` — they add to `77`, so no unexplored route can beat the best of 77, and only then is the answer final. For G3 → K2 the first touch *is* the meeting point, so both situations are normal — which is exactly why the test cannot be "stop as soon as they touch".

### Is Dijkstra recursive? (a likely viva question)

**No. It is iterative — a loop, not recursion.**

Recursion needs a problem that breaks into a **smaller copy of itself**. A good recursive algorithm:

- **shrinks** — each call handles less than its parent, like `factorial(n)` calling `factorial(n - 1)`;
- **shrinks towards a base case**, so the calls stop;
- works on **independent sub-problems**, each solvable without the others (this is what the call stack gives you).

Dijkstra has none of these:

- **It cannot shrink.** Every step asks "which unfound location is cheapest *anywhere on the map*?" That question is always about the whole graph, and there is no smaller sub-graph to hand to a recursive call.
- **State is shared and edited by everyone.** `dist[]` is not a value passed down and combined on the way back up. It is one shared table that many steps read and rewrite, and correctness depends on those edits being seen in the right global order.
- **It is a while-loop whose exit is a loop condition, not a base case.** It stops when the goal is settled, or when everything is settled.

The load-bearing idea is the **greedy** choice at the top of the loop: *always settle the cheapest unfinished location, because its distance is now final.* Recursion can express "divide and conquer" or "try every branch"; it cannot express "keep making the globally cheapest choice until done". A recursive rewrite would look like this and would be strictly worse:

```c
/* DO NOT DO THIS - shown only to make the point */
void dijkstra_recursive(int settled[], int dist[], int goal) {
    int u = cheapestUnsettled(settled, dist);   /* still a scan of ALL n */
    if (u == -1 || u == goal) return;           /* base case */
    settled[u] = 1;
    relax(u, dist);
    dijkstra_recursive(settled, dist, goal);    /* tail call, same n each time */
}
```

That is not recursion in any useful sense: the array never shrinks, so every call does the same work as the one before it, the stack grows to n frames for nothing, and all the state has to be passed through as arguments. The loop is the honest form of the same thing (and `gcc` would turn that tail call back into a loop anyway).

If the project asks where recursion appears: the route *reconstruction* walks `prev[]` backwards from the goal, which is an iterative traversal of a tree by following parents. Dijkstra itself is greedy and iterative — do not claim otherwise.

### Two more likely questions

**Why not breadth-first search?** BFS finds the shortest path in *number of hops* (edges), not in total distance. On this map a 2-hop route can easily cost more than a 4-hop one, because a single road can cost anywhere from 3 to 9. BFS would pick the route with the fewest hops and get it wrong. Dijkstra is the correct tool because it accounts for the weights.

**Why can't Dijkstra handle negative roads?** The rule "a settled location's distance is final" breaks down: a longer route containing a negative road could later turn out to be cheaper. Bellman–Ford is the algorithm used when edges can be negative.

## Complexity

| What | Time | Space |
|---|---|---|
| Plain Dijkstra, as written (scan for the minimum) | O(N²) — about 2700 steps for N = 52 | O(N²) for the matrix, plus O(N) working arrays |
| Plain Dijkstra with a binary heap | O((N + E) log N) | O(N + E) |
| Bidirectional Dijkstra, as written | same cost per search, and fewer locations settled per search on most routes | O(N²) for the matrix, plus O(N) working arrays |
| Adjacency matrix | — | O(N²) |
| Adjacency list | — | O(N + E) |

Here N = 52 and E = 87 roads. For a graph this small the simple array scan is faster in practice and much easier to read than a heap, which is why it is used. On this map the bidirectional saving ranges from about 50% down to about 2% depending on the route (the table in the previous section).

## Project structure

```text
dsal/
├─ main.c              the whole program
├─ README.md           this file
├─ .zed/tasks.json     Zed task "C: Compile and Run"
└─ docs/
   └─ index.html       annotated walkthrough of main.c (see below)
```

## Annotated walkthrough

`docs/index.html` is a self-contained page that walks through every line of `main.c` in source order, with the algorithm and data structures explained between the code blocks. Open it in a browser straight from the repository:

- [docs/index.html](docs/index.html)

It is published with GitHub Pages, served straight from the `docs` folder on `main` (that setting lives under **Settings → Pages** if you ever need to change it):

- https://asterxsk.github.io/Dijkstra-Router/

## Limitations

- **The coloured map needs a terminal that understands ANSI colours and UTF-8.** Windows Terminal, VS Code's integrated terminal, and modern consoles all work. In an old console that does not support virtual-terminal sequences, the colour escape codes appear literally as stray characters around the map, and the box-drawing lines may look wrong. The `setupConsole()` function switches the Windows console to UTF-8 and enables virtual-terminal processing; on other systems it does nothing, because those terminals already support both.
- **The distances are synthetic**, generated by a formula rather than measured, so the routes are not routes on a real map.
- **`0` means "no road"** in the matrix, which is only safe because every real distance is at least 3.
- **The graph is undirected.** Each distance is stored twice. A one-way road would store only `graph[u][v]`, and the backward search would need the reversed graph.
