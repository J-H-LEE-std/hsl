# HS-L (Harmony Search Language)

**HS-L** is a lightweight domain-specific language (DSL) designed to model and solve mathematical optimization problems using the **Harmony Search (HS)** algorithm.  
It provides a clear and compact syntax for defining objectives, variables, and constraints, and can be executed directly from the command line for benchmarking or experimentation.

---

## Overview

Harmony Search (HS) is a metaheuristic optimization algorithm inspired by the improvisation process of musicians seeking a perfect harmony.  
HS-L provides a simple way to describe optimization problems, automatically parse them into an internal model, and run the Harmony Search solver.

Typical use cases include:
- Non-linear optimization
- Constrained or unconstrained mathematical problems
- Metaheuristic algorithm research and benchmarking

---

Each HS-L project typically consists of two files:

| File | Purpose |
|------|----------|
| `input.hs` | Defines the problem structure (objective, variables, constraints) |
| `parameter.hsparm` | Defines Harmony Search parameters (HMS, HMCR, PAR, etc.) |

## Example Input

Example HS-L script (`example.hs`):

```
[OBJ] max 10 * x + 4 * y
[VAR] x, 0, 10, int
[VAR] y, 0, 10, any
[ST] 10 * x + 5 * y <= 300
[END]
```

Example HS-L parameter file (`parameter.hsparm`):

```
HMS,30
HMCR,0.95
PAR,0.7
MaxImp,30000
N_Seg,300
```

or more examples, refer to the files `input.hs` and `parameter.hsparm` included in the repository root or under `/bin/`.

---

## Syntax Rules

| Token | Description |
|--------|-------------|
| `[OBJ]` | Defines the optimization objective. Syntax: `[OBJ] max <expr>` or `[OBJ] min <expr>` |
| `[VAR]` | Declares a variable. Syntax: `[VAR] <name>, <lower>, <upper>, <type>` <br>Type can be `int` or `any` (continuous). |
| `[ST]` | Defines a constraint (statement). Multiple constraints can be declared. |
| `[END]` | Marks the end of the problem definition. |
| **Operators** | Supports `+`, `-`, `*`, `/`, `^` for arithmetic expressions. ^ will work for power operation. |

---

##  Harmony Search Parameters

HS-L uses a configuration file (`parameter.hsparm`) to define algorithm behavior.  
Each line contains a key–value pair separated by a comma.

| Parameter | Description |
|------------|-------------|
| **HMS** | Harmony Memory Size (number of candidate solutions) |
| **HMCR** | Harmony Memory Consideration Rate |
| **PAR** | Pitch Adjustment Rate |
| **MaxImp** | Maximum improvisations (iterations) |
| **N_Seg** | Number of segmentations (optional, used for reporting or iteration grouping) |

These parameters are automatically loaded from `parameter.hsparm` unless overridden by CLI arguments.

---

## Command Line Usage

From `src/main.cpp`, HS-L supports both file-based and command-line input execution.

### Basic Usage

Input:
```bash
./hsl-linux -i input.hs -p parameter.hsparm
```
Expected Output:
```
Best value: -0.201976
x1 = 0.786037
x2 = 0.619003
Elapsed CPU time: 0.052 s
```

---
##  How to Build

HS-L is written in **C++20** and uses **CMake** for building.

```bash
git clone https://github.com/J-H-LEE-std/hsl.git
cd hsl
cmake -B build -S .
cmake --build build --config Release
```

---

## Implementation Details

* Lexer / Parser — Tokenizes and interprets HS-L syntax.
* Evaluator — Translates parsed expressions into evaluable objective functions.
* Optimizer Core — Implements Harmony Search algorithm with tunable parameters.
* CLI Interface — Built using CLI11.
  
---

## License
This software is distributed under the BSD 3-Clause License. See the [LICENSE](./LICENSE) file for details.

## Third-Party Software Notice
For details on third-party software notices, see [THIRD_PARTY_NOTICES.md](./THIRD_PARTY_NOTICES.md).