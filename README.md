# DSAL Project — Shortest Path with Bidirectional Dijkstra

**Files**

| File | Purpose |
|---|---|
| `main.c` | The whole program (one file, no libraries beyond `stdio.h` and `string.h`) |
| `README.md` | This explanation, for your viva / report |

**Build and run**

```bash
gcc main.c -o main
./main            # Windows: main.exe
```

Type two location names when asked, e.g. `A1 M4`. Pressing Enter with nothing typed uses the default A1 → M4.

---

## 1. What the program does

It builds a weighted graph of **52 locations** arranged as a 13 × 4 map, then finds the shortest route between two of them in **two different ways**:

1. Normal Dijkstra's algorithm — search outward from the start.
2. **Bidirectional** Dijkstra — search outward from the start *and* backward from the goal at the same time, and meet in the middle.

Both are printed, so you can see they give the same route and the same total distance.

## 2. Is Dijkstra recursive?

**No. It is iterative — a loop, not recursion.** Worth getting right in the viva.

Recursion needs a problem that breaks into a **smaller copy of itself**. Checklist for a good recursive algorithm:

- **Shrinks** — each call handles less than its parent. `factorial(n)` calls `factorial(n-1)`. Depth-first search on a tree calls itself on each *subtree*, which is smaller than the whole tree.
- **Shrinks towards a base case** — so the calls stop.
- **Independent sub-problems** — each call can be solved without waiting on the others (this is what the stack gives you).

Dijkstra has **none** of these:

- **It cannot shrink.** Every step has to ask "which unfound location is cheapest *anywhere on the map*?" That question is always about the whole graph. There is no smaller sub-graph you can hand to a recursive call and then forget about.
- **State is shared and edited by everyone.** `dist[]` is not a value passed down and combined on the way back up, like a factorial's return value. It is one shared table that many steps read and rewrite, and the correctness of the algorithm depends on those edits being seen in the right global order.
- **It is a while-loop with no base case in the recursive sense.** It stops when the goal is settled or everything is settled — a loop condition, not a base case.

The load-bearing idea of Dijkstra is the **greedy** choice at the top of the loop: *always settle the cheapest unfinished location, because that distance is now final.* Recursion can express "divide and conquer" or "try every branch"; it cannot express "keep making the globally cheapest choice until done". A recursive rewrite would look like this and would be strictly worse:

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

That is not recursion in any useful sense. The array size never shrinks, so every call does the same amount of work as the previous one, the stack grows to n frames for nothing, and you have to pass all the state through as arguments. The loop is the honest form of the same thing. (`gcc` would turn that tail call into a loop anyway, which tells you it was a loop all along.)

**So if the project asks for recursion, where is it?** In the *reconstruction* of the route, subtly: walking `prev[]` backward from the goal until `prev` runs out is an iterative traversal of a tree by following parents. Depth-first search is recursive. Dijkstra is not. If you want something recursive in the program for the report, the natural place is a recursive path-printer in place. But do not claim Dijkstra itself is recursive.

## 3. The map

13 rows named `A`…`M`, 4 columns numbered `1`…`4` → 13 × 4 = **52** locations. Every location is named for where it sits: `A1` is the top-left corner, `M4` the bottom-right.

```
A1--3--A2--6--A3--9--A4
|3     |8     |6     |4
B1--5--B2--8--B3--4--B4
|6     |4     |9     |7
C1--7--C2--3--C3--6--C4
   ...  (rows D .. L)  ...
M1--6--M2--9--M3--5--M4
```

The number on a road is its **distance**. A road goes both ways, so the graph is **undirected**.

Each location has a number in the code, following one rule:

```
number = row * 4 + column        (both counting from 0)
```

so `A1` = 0, `A2` = 1, `A3` = 2, `A4` = 3, `B1` = 4, … `M4` = 51.

### Where the distances come from

Every location is joined to the one on its **right** and the one **below** it, with a distance worked out by a small formula:

```c
graph[u][v] = 3 + ((r * 2 + c * 3) % 7);   /* road to the right */
graph[u][v] = 3 + ((r * 3 + c * 5) % 7);   /* road downward     */
```

This gives thoroughly mixed distances between 3 and 9, arranged so that the cheapest route zig-zags rather than running straight across. The `% 7` means "the remainder after dividing by 7", which keeps the result in the range 0–6; adding 3 makes it 3–9.

A hand-written list would be about 100 entries for 52 locations, which is long and easy to mistype. The formula does the same job in four lines. **Say plainly in your report that the distances are synthetic**, generated by a formula rather than measured from a real map. You are demonstrating an algorithm, not solving a routing problem with real data. If your course wants a hand-written edge list, the formula is easy to replace with a list — the algorithm does not care where the numbers come from.

### Connectivity

The map is a fully connected grid, so **every location can reach every other**. This is worth stating (and worth verifying, which the program's output lets you do): a shortest-path program is not very interesting if half the map is unreachable, and "no route" is a real possible answer for a general graph — which is why the code checks for it.

### How the graph is stored: an adjacency matrix

`graph[52][52]` is a table where `graph[u][v]` is the distance between `u` and `v`. It holds 2704 `int`s — it is symmetric, so half of that is duplicate, and most of it is 0.

| | Adjacency matrix | Adjacency list |
|---|---|---|
| Memory for 52 locations | 2704 entries | ~100 entries (one per road) |
| Look up "is `u` joined to `v`?" | instant, `graph[u][v]` | walk `u`'s list |
| Iterate `u`'s neighbours | scan all 52, mostly zeros | walk the list |

Counting all the roads: each of the 13 rows has 3 horizontal roads (39) and each adjacent row pair has 4 vertical roads (12 × 4 = 48), so there are **87 roads**, stored twice each because the graph is undirected.

87 roads fill 174 of the 2704 matrix cells (each road twice), so the matrix is about **6% full and 94% zeros** — it is *sparse*. An adjacency list would use far less memory and would skip the empty cells when relaxing neighbours. The matrix is used here because `graph[u][v]` is one line of code and much easier to read and explain. **Say in your report that the matrix costs O(N²) memory regardless of how many roads there are, and that an adjacency list would be the right choice for a real road network.** That trade-off is exactly the kind of thing that earns marks.

Note also: **0 means "no road".** That is safe here only because every real distance is 3 or more. It is a simplification — a real program would keep a separate flag, because a distance of 0 can be legal.

## 4. Normal Dijkstra — step by step

**The problem it solves:** every road has a cost, and we want the cheapest total cost from one location to another.

Dijkstra works because of one fact: **if you always settle the cheapest unfinished location, its distance is final.** You could not have reached it more cheaply through some other location, because that other location would itself cost at least as much.

The code keeps three arrays:

- `dist[x]` — the cheapest distance to `x` found *so far*.
- `settled[x]` — 1 once we are certain `dist[x]` is final.
- `prev[x]` — the location we came from, so the route can be rebuilt.

The loop:

1. Pick the unsettled location `u` with the smallest `dist[]`. We scan all 52; that is O(N) per round.
2. Mark `u` settled — done forever. If `u` is the goal, stop.
3. **Relax** each neighbour: if `dist[u] + weight(u,v)` is less than `dist[v]`, we have found a cheaper way into `v`, so update `dist[v]` and set `prev[v] = u`.
4. Repeat. N rounds at most.

Total running time is **O(N²)** = about 52 × 52 ≈ 2700 simple steps, because of N rounds each scanning N distances. Each round also relaxes neighbours, which is another N — same order, so it does not change the O(N²). A version using a priority queue (binary heap) gets **O((N + E) log N)**, and that is what you would use on a real road network. For 52 locations the simple scan is faster in practice *and* much easier to read, so it is the right choice for the project. Worth a sentence in your report.

### Worked example: A1 → A4 (same row)

| Step | Settle | dist updates |
|---|---|---|
| start | — | dist[A1] = 0, all others ∞ |
| 1 | A1 (0) | A2 ← 3, B1 ← 3 |
| 2 | A2 (3) | A3 ← 3+6 = 9, B2 ← 3+8 = 11 |
| 3 | B1 (3) | B2 ← 3+5 = 8 (**improves** the 11), C1 ← 3+6 = 9 |
| 4 | B2 (8) | B3 ← 8+8 = 16, C2 ← 8+4 = 12 |
| 5 | A3 (9) | **A4 ← 9+9 = 18** ← the answer, B3 ← 9+6 = 15 (**improves** the 16) |
| 6 | C1 (9) | D1 ← 9+9 = 18 |
| 7 | C2 (12) | C3 ← 12+3 = 15, D2 ← 12+7 = 19 |
| 8 | B3 (15) | B4 ← 15+4 = 19 |
| 9 | C3 (15) | C4 ← 15+6 = 21, D3 ← 15+5 = 20 |
| 10 | A4 (18) | goal settled — stop |

Find the answer's arrival: at step 5, A3 finds a route to A4 costing 18. A4 is **not** settled then, because 18 is not yet the cheapest un-settled distance — C1 (9), C2 (12) and B3 (15) are all cheaper, so the loop deals with those first and A4 waits until step 10. This is the greedy rule doing real work, and it is why the plain search settles 10 locations to answer a question about a location it found at step 5.

The settled count is 10 out of 52 — the search never had to look at most of the map. In steps 3 and 5 a distance gets *improved* (B2 from 11 to 8, B3 from 16 to 15), which is the whole point of the relaxation step: a location's distance is only best-so-far until it is settled.

The result is 18, running straight along the top row.

### Worked example: A1 → M4 (opposite corners)

This is the worst case for both algorithms — the full diagonal of the map. The route the program finds is:

```
A1 -> B1 -> B2 -> C2 -> C3 -> D3 -> E3 -> F3 -> G3 -> H3 -> I3 -> I4 -> J4 -> K4 -> L4 -> M4
cost 77
```

It zig-zags, and the cost is the largest on this map. The plain search settles all 52 locations before the goal's distance is final, which is the maximum work it could possibly do.

## 5. Bidirectional Dijkstra

**The idea.** A normal search from A1 spreads out in all directions. A search from M4 spreads too. Run both, and they meet in the middle. Each search then covers about half the radius, which on a big map is a large saving: the number of locations within radius 10 of a point is about four times the number within radius 5, so one search of radius 10 examines about four times as many as two searches of radius 5 *each* — with both halves included, roughly twice the saving.

**What we keep.** Two sets of arrays, one per direction:

- forward: `distF[]`, `prevF[]` — grown from the **start**
- backward: `distB[]`, `prevB[]` — grown from the **goal**

`prevB` points *towards* the goal, not away from it, so it is the parent in the reversed graph.

**Meeting point.** Any location `x` that both searches have reached gives a complete route of length

```
distF[x] + distB[x]        (start → x, then x → goal)
```

The best `x` over all reached locations lies on the shortest path.

**The loop each round:**

1. Find the cheapest unfinished location on each side: `u` (forward) and `v` (backward).
2. **Stopping test** — if `distF[u] + distB[v] >= best`, stop. Reason: any route through an unexplored location must pass through one of these two frontier locations and pay at least that much.
3. Expand the side whose frontier is closer — the smaller of `distF[u]` and `distB[v]`. This keeps the two searches balanced, so they meet in the middle rather than one racing ahead.
4. After every expansion, check all locations for a better meeting point and update `best` and `meet`.
5. When finished, rebuild the route in two halves: start → meet from `prevF`, then meet → goal from `prevB`.

**On 52 locations, here is what actually happens.** The program prints the settled counts, so these are measured, not estimated:

| Route | Plain settles | Bidirectional settles | Saving |
|---|---|---|---|
| `A1 → A4` (same row, short) | 10 / 52 | 4 + 3 = 7 | 30% |
| `M1 → M4` (same row, short) | 10 / 52 | 3 + 2 = 5 | 50% |
| `G3 → K2` (medium diagonal) | 34 / 52 | 14 + 12 = 26 | 24% |
| `A4 → M1` (opposite corners) | 52 / 52 | 27 + 21 = 48 | 8% |
| `A1 → M4` (opposite corners) | 52 / 52 | 26 + 25 = 51 | 2% |

**Be honest about this in your report, and use it.** The saving is real but much smaller than the "twice as fast" figure you will read online, and the reason is worth explaining:

- **This map is long and thin** — 13 rows but only 4 columns. The two searches can only meet in the narrow direction, so they both still sweep most of the *length* of the map. On a square map of the same 52 locations the meeting would be far more effective.
- **Worst case is opposite corners.** The searches start at the two most distant points, so each still has to cover about half the map, and 26 + 25 ≈ 52 is close to no saving at all. Bidirectional search is not a guaranteed speed-up; it is best when the two ends are close in the "width" of the graph, and worst when they are far apart in every direction.

That the saving collapses to almost nothing in the worst case is a genuine, checkable finding from your own program. Report the numbers rather than the party line.

### Worked example: G3 → K2

Forward from G3 meets backward from K2 at `I2` — four rows down and one column across, which is where you would expect the middle of a route to fall. Plain settles 34 locations; the two searches together settle 26 (14 forward, 12 backward). Both report cost 28, and the program prints the identical route for both. That agreement is the check that matters: the bidirectional version is not a different answer, it is the same answer found with less work.

### So does stopping early really matter?

Yes, and this map shows it happening. For **A1 → M4** the route found is 77 and the meeting location is `G3` — but the first location that *both* searches reach is `H1`. H1 is not on the shortest route: it is off to the left as the two searches sweep past each other. A program that stopped the moment the searches touched would take H1 as the junction and report a longer route than 77.

The stop test in the code is the one that is safe: keep going until `distF[u] + distB[v] >= best`, where `best` is the cheapest complete route found so far. On A1 → M4 that runs until both frontiers cost 26 and 25, and only then is 77 known to be final.

(For A1 → M4 the first-touched location happens to be off the route; for the other routes in the table the first touch *is* the meeting point. Both situations are normal, which is exactly why the test cannot be "stop when they touch".)

## 6. Talking points for the viva

- **"Is Dijkstra recursive?"** No — see section 2. It is greedy and iterative. Recursion needs a problem that shrinks; Dijkstra always needs a global minimum over the whole graph.
- **Why not BFS?** BFS gives the shortest path in *hops* (number of edges). On this map a 2-hop route can easily cost more than a 4-hop one, because a hop can cost 9 or as little as 3. BFS picks the wrong route. Dijkstra accounts for weights.
- **Why can't Dijkstra handle negative weights?** The "settled location is final" argument breaks: a longer route with a negative road could later come in cheaper. Bellman–Ford is used for negative edges.
- **Complexity.** Plain Dijkstra here: O(N²) ≈ 2700 steps for N = 52. With a heap: O((N + E) log N). Bidirectional: same cost per search, with fewer locations settled per search on most routes — and on this map the saving ranges from about 50% down to about 2% depending on the route (table in section 5).
- **The stopping rule** is the part most students get wrong. You may *not* stop the instant the two searches touch. You must stop when the summed frontier distances can no longer beat `best`. Otherwise you can report a route that is not the shortest. On this map the two searches touch at locations that are *not* on the shortest route, so stopping early really would give a wrong answer.
- **Memory trade-off.** The matrix is O(N²) and 94% zeros here (174 filled cells out of 2704); an adjacency list is O(N + E) and is the right structure for a sparse graph. With only 87 roads, this map is definitely sparse.
- **Where the 52 locations came from.** The map is synthetic — a grid with formula-generated distances, not real geography. Say so, and say what it does demonstrate: the algorithm's behaviour on a graph big enough that the two-search saving becomes visible and measurable.
- **Directed graphs.** Our roads are symmetric, which is why each distance is stored twice. For a one-way street you would store only `graph[u][v]`, and the backward search would need the **incoming** roads of the goal — the reversed graph. This is a common follow-up question.
