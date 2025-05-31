# -- Setup ---------------------------------------------------------------------

# Source files. -6 is the baseline benchmark, and -7 is the improved version.
SOURCE = nbody.gcc-6.c nbody.gcc-7.c

# Number of iterations for checking and benchmarking.
N_QUICK = 1000
N_FULL = 50000000

EXE_SUFFIX = gcc_run
OUT_SUFFIX = output.txt

TARGETS = $(patsubst %.c, %.$(EXE_SUFFIX), $(SOURCE))
OUTPUTS = $(patsubst %.c, %.$(OUT_SUFFIX), $(SOURCE))

# -- Compiler Options ----------------------------------------------------------

CC=gcc

# Optimised.  Added -std=c99.
C_FLAGS=-pipe -Wall -std=c99 -O3 -fomit-frame-pointer -march=ivybridge

# Debugging. Added -std=c99, -Wextra, and -pedantic.
#C_FLAGS=-g -pipe -std=c99 -Wall -Wextra -pedantic -Wextra -march=ivybridge

LIBS=-lm

# -- Targets -------------------------------------------------------------------

all: $(TARGETS) $(OUTPUTS)

build: $(TARGETS)
	@echo "$(SEP)\nBuilding targets: $(TARGETS)"

check: $(OUTPUTS)

clean:
	rm -f $(TARGETS) $(OUTPUTS)

time: $(TARGETS)
	@$(foreach target,$(TARGETS),$(call run-time,$(target)))

hyperfine: $(TARGETS)
	@$(foreach target,$(TARGETS),$(call run-hyperfine,$(target)))

.PHONY: all build check clean time hyperfine

# -- Functions & Pattern Rules -------------------------------------------------

SEP = --------------------------------------------------------------------------

define run-time =
  echo "$(SEP)\nTiming $(1) with N = $(N_FULL)..."
  time ./$(1) $(N_FULL)

endef

define run-hyperfine =
  echo "$(SEP)\nTiming $(1) using hyperfine with N = $(N_FULL)..."
  hyperfine --warmup 2 --runs 5 './$(1) $(N_FULL)'

endef

%.$(EXE_SUFFIX): %.c
	$(CC) $(C_FLAGS) -o $@ $< $(LIBS)

%.output.txt: %.$(EXE_SUFFIX)
	@echo "$(SEP)\nChecking $< with N = $(N_QUICK)..."
	./$< $(N_QUICK) > $@
	ndiff -abserr 1.0e-8 nbody-output.txt $@
