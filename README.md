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

For a detailed description of the program, please refer to the [Wiki for this repository](https://github.com/J-H-LEE-std/hsl/wiki).

---

## Implementation Details

* Lexer / Parser — Tokenizes and interprets HS-L syntax.
* Evaluator — Translates parsed expressions into evaluable objective functions.
* Optimizer Core — Implements Harmony Search algorithm with tunable parameters.
* CLI Interface — Built using CLI11.

---

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
| **Operators** | Supports `+`, `-`, `*`, `/`, `^` for arithmetic expressions. `^` will work for power operation. |

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

## Function Support

HS-L now supports **built-in mathematical and user-defined function calls** within expressions.  
Functions can be used in `[OBJ]`, `[ST]`, or nested within other expressions.
For more information, please refer documents about [Built-in Functions](https://github.com/J-H-LEE-std/hsl/wiki/Built%E2%80%90in-Function).

---
## Command Line Usage

From `src/main.cpp`, HS-L supports both file-based and command-line input execution.

### Basic Usage

Input:
```bash
.\hsl.exe -s input.hs -p parameter.hsparm # for Windows
./hsl-linux -s input.hs -p parameter.hsparm # for macOS
./hsl-linux -s input.hs -p parameter.hsparm # for Linux
```
Expected Output:
```
=== Harmony Search Result ===
Best value: -0.0663723
  x1 = -0.00438216
  x2 = -0.0136801
  x3 = -0.0113302
CPU Time: 1.22576 sec
```

---
## GUI support
HS-L now supports GUI. For more information, please refer please refer to the [GUI descriptions in Wiki](https://github.com/J-H-LEE-std/hsl/wiki/GUI-Interface).

---
##  How to Build

HS-L now provides two executables:

* **CLI version**: command-line solver (`hsl`)
* **GUI version**: wxWidgets-based graphical interface (`hsl_gui`)

Both are written in **C++20** and built using **CMake**.
The GUI version additionally requires **wxWidgets**. GUI cannot be built unless wxWidgets is installed.
Before you build HS-L, please install it beforehand following guide for [Installing wxWidgets](https://docs.wxwidgets.org/3.2/overview_install.html). 

### Build from Source

```bash
git clone https://github.com/J-H-LEE-std/hsl.git
cd hsl
cmake -B build -S .
cmake --build build --config Release
```

If build failed, try to make build directory while set wxWidgets directory manually by using **-DwxWidgets_ROOT_DIR** parameter.

---


## License
This software is distributed under the BSD 3-Clause License. See the [LICENSE](./LICENSE) file for details.

## Third-Party Software Notice
For details on third-party software notices, see [THIRD_PARTY_NOTICES.md](./THIRD_PARTY_NOTICES.md).

## Acknowledgement

This software is an implementation based on the design proposal by Prof. Zong Woo Geem.

---

## References
* Harmony Search

Zong Woo Geem, Joong Hoon Kim, and G. V. Loganathan, “A New Heuristic Optimization Algorithm: Harmony Search,” SIMULATION, vol. 76, no. 2, pp. 60–68, Feb. 2001, doi: 10.1177/003754970107600201.

http://www.harmonysearch.info/

* Interpreter Writing

https://craftinginterpreters.com/

R. Nystrom, Crafting Interpreters. Genever Benning, 2021.

T. Ball, Writing an Interpreter in Go. Ball Thorsten, 2020. 

