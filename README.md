# Faster, Idiomatic C99 N-Body Benchmark

## Overview

*The N-Body Problem* is a standard benchmark based on modelling the orbits of 
Jovian planets using a simple forward-Euler integration method.  See *The 
Computer Language Benchmarks Game* for a complete 
[description](https://benchmarksgame-team.pages.debian.net/benchmarksgame/description/nbody.html) 
as well as the current 
[performance comparisons](https://benchmarksgame-team.pages.debian.net/benchmarksgame/performance/nbody.html). 

For fun I implemented a version using standard, idiomatic C99 with a 
few additional performance improvements.  Speed of this version is about 
**7% faster** than the 
[fastest C benchmark that didn't use hand-coded or non-standard vector instructions](https://benchmarksgame-team.pages.debian.net/benchmarksgame/program/nbody-gcc-6.html)
at the time of writing.

## Modifications

### Speed Improvements

The main speed improvements are: 
  1. I avoid recalculating the inverse square-root of the distance between 
     bodies on each iteration, which means that only a single division 
     operation is required for each iteration of `advance()`. To do this I save 
     intermediate values relating to body-body distances from the previous 
     iterations and use a *root refinement* method instead.

     > Saved values form strictly upper triangular matricies, with each one
     having $N (N - 1) / 2$ entries.  For the benchmark five-body problem
     ($N = 5$) in double precision (64-bit / 8 byte floating point), each matrix
     therefore requires $[5 (5 - 1) / 2] * 8 = 80$ bytes.  Three of these
     are stored (for $\Delta r_{ij}$, $r_{ij}^2$ and $1 / r_{ij}$), meaning an 
     additional $80 \times 3 = 240$ bytes is required.  Because the original 
     storage requirement for the bodies is $(3 + 3 + 1) * 8 = 56$ bytes each 
     or $5 * 56 = 280$ bytes total, the total required data for the problem 
     is only $280 + 240 = 520$ bytes.  This still easily fits in the L1 cache 
     giving very fast access.

  2. For root refinement I use just *one iteration* of 
     [Halley's Method](https://en.wikipedia.org/wiki/Square_root_algorithms#Iterative_methods_for_reciprocal_square_roots) 
     which is slightly faster than using two passes of the more commonly used 
     *Newton's Method* and gives a slightly better accuracy overall in this 
     case.

### Idiomatic and Standard C99

I tired to adopt a more common, readable and typesafe layout.  For example 
vector components are grouped into single entities using `typedef double 
vec3_t[3]`.  This allows vector operations to be applied directly, such as 
$\Delta x_a = v_a \Delta t$ as `vec3_scale(del_x, a->v, dt)`.

I only used standard C99 types and constructs such as `enum` for constants
(where possible), `uint_fast8_t` and so on. Any vectorizations are done only 
by the compiler for the target architecture.

## Compile and Run

Source files: 
- `nbody.gcc-6.c`: [Fastest C benchmark at the time of writing](https://benchmarksgame-team.pages.debian.net/benchmarksgame/program/nbody-gcc-6.html) 
  without hand-coded or non-standard vector instructions.
- `nbody.gcc-7.c`: Improved version as described above.  This is normally about 
  *7% faster* than the baseline version.

Available make targets:

| Target             | Description                                           |
|--------------------|-------------------------------------------------------|
| `make all`         | Build executables and run error checks.               |
| `make build`       | Build executables.                                    |
| `make check`       | Run N = 1000 error checks using *ndiff* [^1].         |  
| `make time`        | Time executables using N = 50000000.                  | 
| `make hyperfine`   | *(Optional)* Time executables using *hyperfine* [^2]. |

[^1]: This checks program output values against reference values in 
      `nbody-output.txt` for absolute errors < 1.0e-8.  Installation of 
      [ndiff](https://www.math.utah.edu/~beebe/software/ndiff/) is required.

[^2]: Installation of [hyperfine](https://github.com/sharkdp/hyperfine) is 
      required.

Individual benchmarks can be checked and run using the `nbody.*.gcc_run` 
executables.  Examples are as follows:
```bash
# Check the improved benchmark output for errors with N = 1000.
./nbody.gcc-7.gcc_run 1000 > nbody.gcc-7.output.txt
ndiff -abserr 1.0e-8 nbody-output.txt nbody.gcc-7.output.txt

# Time the improved benchmark with N = 50000000.
time ./nbody.gcc-7.gcc_run 50000000
```
