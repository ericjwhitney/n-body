# Faster Idiomatic C99 N-Body Problem

# Overview

Short burst xxxx Jovian planets

Description: https://benchmarksgame-team.pages.debian.net/benchmarksgame/description/nbody.html
Performance: https://benchmarksgame-team.pages.debian.net/benchmarksgame/performance/nbody.html

xxx Main improvmenets

## Setup and Run

Available make targets:
- `make all` - Build executables and run N = 1000 error checks.
- `make build` - Build executables.
- `make check` - Run N = 1000 error checks.  This checks probgram output values against file `nbody-output.txt` for absolute errors < 1.0e-8.
  * Requires `ndiff`, for installation see https://www.math.utah.edu/~beebe/software/ndiff/
- `make time` - Time executables using N = 50000000. 
- `make hyperfine` - *(Optional)* Time executables using Hyperfine.
  * Requires `hyperfine`, for installation see https://github.com/sharkdp/hyperfine 

Examples of checking and running individual benchmarks are as follows:
```bash
# Check the improved benchmark output for errors with N = 1000.
./nbody.gcc-7.gcc_run 1000 > nbody.gcc-7.output.txt
ndiff -abserr 1.0e-8 nbody-output.txt nbody.gcc-7.output.txt

# Time the improved benchmark with N = 50000000.
time ./nbody.gcc-7.gcc_run 50000000
```


Benchmark is nbody 6 and improved is 7 - show basic run with time


xxxx

