# Final Project 2025 - Movhex

[![Build](https://github.com/dragonero2704/API-Progetto/actions/workflows/build.yml/badge.svg)](https://github.com/dragonero2704/API-Progetto/actions/workflows/build.yml)
[![Tests](https://github.com/dragonero2704/API-Progetto/actions/workflows/tests.yml/badge.svg)](https://github.com/dragonero2704/API-Progetto/actions/workflows/tests.yml)

Final project for "Algorithms and Principles of Computer Science" course of Politecnico di Milano, 2025

## Build

Navigate to this repository, then run

```bash
mkdir build
cd build
cmake ..
```

This will create a build directory to put all build files.

To build the program type
```bash
cmake --build .
```
or
```bash
make
```

## Run
To run the program cd into the build directory then type

```bash
./movhex
```

## Test

In the `test` folder there are some test and a `cpp` source to compile to be able to execute the test. This file is built together with the main program into another executable in `build/test`.

To run the tests, cd into the `build` directory, then type

```bash
ctest
```

Use `-j <n>` option to specify the number of jobs to run the tests in parallel (recommended)

## Description

The [project specification](specifiche.pdf) requires to code a `C` program that reads from `stdin` the following commands:

- `init <n° columns> <n° rows>`: creates a rectangular map of hexagons
- `change_cost <x> <y> <v> <radius>`: increment the cost of the hexagon located at the coordinates $`(x,y)`$ of $`v`$ and propagates the cost change
- `toggle_air_route <x1> <y1> <x2> <y2>`: activates an *air route* between hexagon $`(x_1,y_1)`$ and hexagon $`(x_2,y_2)`$
- `travel_cost <xp> <yp> <xd> <yd>`: calculates the best travel path (**minimum** cost) between hexagon $`p`$ and hexagon $`d`$

The [project specification](specifiche.pdf) states that `toggle_air_route` command is rarely called, offering optimization oppurtunities.

## Implementation

Since the map has $x$ "columns" e $y$ "rows", it's possible to use cartesian coordinates. The most convenient implementation of the map graph is a matrix. Each hexagon has 6 adiacent hexagon, except the ones at the border of the map. These adjacencies can be represented as 6 displacement vectors:

```math
\{ \begin{pmatrix}0 \\\ 1\end{pmatrix}, \begin{pmatrix}1 \\\ 0\end{pmatrix}, \begin{pmatrix}0 \\\ -1\end{pmatrix}, \begin{pmatrix}-1 \\\ 0\end{pmatrix}\}
```

The last 2 adjacencies change depending on the row:

- **even** row: $`\set{\begin{pmatrix}-1 \\\ 1\end{pmatrix},\begin{pmatrix}-1 \\\ -1\end{pmatrix}}`$
- **odd** row: $`\set{ \begin{pmatrix}1 \\\ -1\end{pmatrix}, \begin{pmatrix}1 \\\ 1\end{pmatrix} }`$

Here is the `C` implementation:
```C
const char adiacenze[2][6][2] = {
    {{1, 0}, {-1, 0}, {0, 1}, {0, -1}, {-1, 1}, {-1, -1}},  // even
    {{1, 0}, {-1, 0}, {0, 1}, {0, -1}, {1, 1}, {1, -1}}     // odd
};
```

From the definition of air route in the [project specification](specifiche.pdf), we can notice that the air route cost is equal to the ground traversal cost of the starting hexagon.

```C
typedef struct Air_route
{
    int hexagon_index;          // index of the hexagon linearizing the matrix coordinates
    struct Air_route *next;     // pointer to the next air route
} Air_route;

typedef struct Hexagon
{
    int cost;                   // traversal cost
    Air_route *air_routes_head; // pointer to the HEAD of air_routes list
} Hexagon;
```

In this implementation Dijkstra algorithm is used to find the **minimum cost** path between two hexagons, since the
project specification allows strictly positive traversal costs. The Dijkstra priority queue is implemented using a *min-heap*.

To optimize the program, `travel_cost` function calls are cached using a [hashmap](src/main.c#L54), invaliditing the cache at each map reset or air_route creation/deletion.

## Grade

This project received a final grade of 30L/30.
