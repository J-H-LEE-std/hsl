# HS-L (Harmony Search Language)

**HS-L** is a lightweight domain-specific language (DSL) designed to model and solve mathematical optimization problems using the **Harmony Search (HS)** algorithm.  
It provides a clear and compact syntax for defining objectives, variables, and constraints, and can be executed from the command line or GUI for benchmarking and experimentation.

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
* GUI Interface — Provides a wxWidgets-based desktop frontend.
* Logging — Supports TXT, CSV, and JSONL experiment logs.

---

## Example Input

Example HS-L script (`example.hs`):

```
[OBJ] max 10 * x + 4 * y
[VAR] x, 0, 10, int
[VAR] y, 0, 10, any
[CONST] C1 = 100
[FUNC] fxy = 10 * x + 5 * y
[ST] 10 * x + 5 * y <= 300
[ST] fxy <= C1 * 3
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

For more examples, refer to the files `input.hs` and `parameter.hsparm` included under `./bin/`.

---

## Syntax Rules

| Token | Description |
|--------|-------------|
| `[OBJ]` | Defines the optimization objective. Syntax: `[OBJ] max <expr>` or `[OBJ] min <expr>` |
| `[VAR]` | Declares a variable. Syntax: `[VAR] <name>, <lower>, <upper>, <type>` <br>Type can be `int` or `any` (continuous). Range-style names such as `x[1..3]` are also supported. |
| `[CONST]` | Declares a named constant or fixed expression alias. Syntax: `[CONST] <name> = <expr>` |
| `[FUNC]` | Declares a reusable named expression alias. Syntax: `[FUNC] <name> = <expr>` |
| `[ST]` | Defines a constraint (statement). Multiple constraints can be declared. |
| `[END]` | Marks the end of the problem definition. |
| **Operators** | Supports `+`, `-`, `*`, `/`, `^` for arithmetic expressions. `^` will work for power operation. |

---

For more details about internal structure and parsing flow, please refer to the [Architecture](https://github.com/J-H-LEE-std/hsl/wiki/Architecture) document in the Wiki.

---

##  Harmony Search Parameters

HS-L uses a configuration file (`parameter.hsparm`) to define algorithm behavior.  
Each line contains a key-value pair separated by a comma.

| Parameter | Description |
|------------|-------------|
| **HMS** | Harmony Memory Size (number of candidate solutions) |
| **HMCR** | Harmony Memory Consideration Rate |
| **PAR** | Pitch Adjustment Rate |
| **MaxImp** | Maximum improvisations (iterations) |
| **N_Seg** | Number of segmentations (optional, used for reporting or iteration grouping) |

These parameters can be loaded from a `.hsparm` file and adjusted further through CLI arguments.

---

## Function Support

HS-L supports built-in mathematical function calls within expressions (e.g., `sqrt`, `sin`, `cos`, `sum`, `product`).  
In addition, `[FUNC]` provides **named expression aliases**, not parameterized user-defined functions.

Valid alias declaration examples:

```hs
[CONST] P = 6000
[FUNC] M = P * (L + x2 / 2)
```

Current limitation:
- `[FUNC]` is not callable-function syntax and does not support parameters.
- Invalid forms include `tau(x1,x2)=...`, `x[i]=...`, `a+b=...`.

For built-in functions, please refer documents about [Built-in Functions](https://github.com/J-H-LEE-std/hsl/wiki/Built%E2%80%90in-Function).

---
## Command Line Usage

HS-L provides a CLI executable for file-based execution. For detailed CLI parameters, please refer to the [CLI-Parameter](https://github.com/J-H-LEE-std/hsl/wiki/CLI-Parameter) document in the Wiki.

### Basic Usage

Input:
```bash
./bin/Windows/hsl.exe -s ./bin/input.hs -p ./bin/parameter.hsparm # for Windows
./bin/Linux/hsl -s ./bin/input.hs -p ./bin/parameter.hsparm # for Linux
./bin/macOS/hsl -s ./bin/input.hs -p ./bin/parameter.hsparm # for macOS
```
Expected Output:
```
[INFO] Starting HS-L...
[HS-L] Run start | ...
Best value: -0.0663723
x[1] = -0.00438216
x[2] = -0.0136801
x[3] = -0.0113302
```

---
## GUI support
HS-L now supports GUI. For more information, please refer to the [GUI descriptions in Wiki](https://github.com/J-H-LEE-std/hsl/wiki/GUI-Interface).

---
##  How to Build

HS-L now provides two executables:

* **CLI version**: command-line solver (`hsl`)
* **GUI version**: wxWidgets-based graphical interface (`hsl_gui`)

Both are written in **C++20** and built using **CMake**.
The GUI version additionally requires **wxWidgets**. GUI cannot be built unless wxWidgets is installed.
Before you build HS-L, please install it beforehand following guide for [Installing wxWidgets](https://docs.wxwidgets.org/3.2/overview_install.html). We recommend building and installing wxWidgets as a static library for this project.

### Build from Source

```bash
git clone https://github.com/J-H-LEE-std/hsl.git
cd hsl
cmake -B build -S .
cmake --build build --config Release
```

If the build fails, try setting the wxWidgets directory manually by using the `-DwxWidgets_ROOT_DIR` parameter.

---

### Logging quick start

```bash
# Summary: header + new_best only
./bin/Windows/hsl.exe --source ./bin/input.hs --max_iter 3000 --seed 42 --json_mode summary

# Snapshot: hm_snapshot every 50 iters
./bin/Windows/hsl.exe --source ./bin/input.hs --max_iter 3000 --seed 42 --json_mode snapshot --json_stride 50

# Full: hm_snapshot every iteration
./bin/Windows/hsl.exe --source ./bin/input.hs --max_iter 3000 --seed 42 --json_mode full

# Verified: Full JSONL (HMS=30, MaxIter=30000, dim~4) produced ~66MB; hm_snapshot lines are buffered and flushed every 100 iters or 4MB (no per-iteration I/O).
```



## License
This software is distributed under the BSD 3-Clause License. See the [LICENSE](./LICENSE) file for details.

## Third-Party Software Notice
For details on third-party software notices, see [THIRD_PARTY_NOTICES.md](./THIRD_PARTY_NOTICES.md).

## Acknowledgement

This software is an implementation based on the design proposal and the [original VBA-based HS-L implementation(Excel-HSL)](https://github.com/ghlee490/Excel-HSL) by Prof. Zong Woo Geem and research team, which is distributed under the MIT License.
Please Refer [THIRD_PARTY_NOTICES.md](./THIRD_PARTY_NOTICES.md) for VBA-based software license.


---

## References
* Harmony Search

Zong Woo Geem, Joong Hoon Kim, and G. V. Loganathan, “A New Heuristic Optimization Algorithm: Harmony Search,” SIMULATION, vol. 76, no. 2, pp. 60–68, Feb. 2001, doi: 10.1177/003754970107600201.

http://www.harmonysearch.info/

* Interpreter Writing

https://craftinginterpreters.com/

R. Nystrom, Crafting Interpreters. Genever Benning, 2021.

T. Ball, Writing an Interpreter in Go. Ball Thorsten, 2020. 
