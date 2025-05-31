/* The Computer Language Benchmarks Game
 * https://salsa.debian.org/benchmarksgame-team/benchmarksgame/
 *
 * Based on nbody #6 contributed by Christoph Bauer and modified by
 * Danny Angelo Carminati Grein.
 *
 * Idiomatic C99 version by Eric J. Whitney, May 2025.
 */

#define NDEBUG          // Comment-out to enable 1 / √(rᵢⱼ) error checking.
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

// -- Problem Definition -------------------------------------------------------

const double pi             = 3.141592653589793;
const double solar_mass     = (4 * pi * pi);
const double days_per_year  = 365.24;
const double dt             = 0.01; // Timestep.
enum {       N_BODIES       = 5 };  // Number of heavenly bodies (limit < 23).

typedef double          vec3_t[3];
typedef uint_fast8_t    index_t;

struct body
{
    vec3_t x, v;
    double mass;
};

static struct body bodies[N_BODIES] = {
    { // Sun.
        .x = {0, 0, 0},
        .v = {0, 0, 0},
        .mass = solar_mass
    },
    { // Jupiter.
        .x = {
            4.84143144246472090e+00,
            -1.16032004402742839e+00,
            -1.03622044471123109e-01
        },
        .v = {
            1.66007664274403694e-03 * days_per_year,
            7.69901118419740425e-03 * days_per_year,
            -6.90460016972063023e-05 * days_per_year
        },
        .mass = 9.54791938424326609e-04 * solar_mass
    },
    { // Saturn.
        .x = {
            8.34336671824457987e+00,
            4.12479856412430479e+00,
            -4.03523417114321381e-01
        },
        .v = {
            -2.76742510726862411e-03 * days_per_year,
            4.99852801234917238e-03 * days_per_year,
            2.30417297573763929e-05 * days_per_year
        },
        .mass = 2.85885980666130812e-04 * solar_mass
    },
    { // Uranus.
        .x = {
            1.28943695621391310e+01,
            -1.51111514016986312e+01,
            -2.23307578892655734e-01
        },
        .v = {
            2.96460137564761618e-03 * days_per_year,
            2.37847173959480950e-03 * days_per_year,
            -2.96589568540237556e-05 * days_per_year
        },
        .mass = 4.36624404335156298e-05 * solar_mass
    },
    { // Neptune.
        .x = {
            1.53796971148509165e+01,
            -2.59193146099879641e+01,
            1.79258772950371181e-01
        },
        .v = {
            2.68067772490389322e-03 * days_per_year,
            1.62824170038242295e-03 * days_per_year,
            -9.51592254519715870e-05 * days_per_year
        },
        .mass = 5.15138902046611451e-05 * solar_mass
    }
};

// -- Definitions & Helper Functions -------------------------------------------

enum {       NR_ITS = 1 };  // No. of 1 / √(rᵢⱼ) refinement iterations.
const double NR_TOL = 1e-7; // Required refinement tolerance (req < 1e-7).

// Some values for body-pairs (i, j) can be precomputed.  Only elements above
// the diagonal are required.
#define N_PAIRS  ((N_BODIES) * (N_BODIES - 1) / 2)
static vec3_t    dr[N_PAIRS]; // Δrᵢⱼ = rᵢ - rⱼ = (Δxᵢⱼ, Δyᵢⱼ, Δzᵢⱼ)
static double    r2[N_PAIRS]; // rᵢⱼ²
static double inv_r[N_PAIRS]; // 1 / rᵢⱼ

// Macros for vec3_t function arguments that are non-null and unaliased.
#define VEC3_ARG(name)       double name[static restrict 3]
#define VEC3_CONST_ARG(name) const double name[static const restrict 3]

#define VEC3_FOR(name) for (index_t name = 0; name < 3; name++)
#define INLINE static inline

INLINE void vec3_sub(VEC3_ARG(result), VEC3_CONST_ARG(x), VEC3_CONST_ARG(y))
{  VEC3_FOR(i) result[i] = x[i] - y[i];  }

INLINE void vec3_iadd(VEC3_ARG(result), VEC3_CONST_ARG(x))
{  VEC3_FOR(i) result[i] += x[i];  }

INLINE void vec3_isub(VEC3_ARG(result), VEC3_CONST_ARG(x))
{  VEC3_FOR(i) result[i] -= x[i];  }

INLINE double vec3_norm2(VEC3_CONST_ARG(x))
{  return x[0] * x[0] + x[1] * x[1] + x[2] * x[2];  }

INLINE void vec3_scale(VEC3_ARG(result), VEC3_CONST_ARG(x), const double k)
{  VEC3_FOR(i) result[i] = k * x[i];  }

INLINE void vec3_iscale(vec3_t result, const double k)
{  VEC3_FOR(i) result[i] *= k;  }

// -- Main N-Body Calculations -------------------------------------------------

// Calculate distances between bodies.
void calc_distances()
{
    for (index_t i = 0, p = 0; i < N_BODIES; i++)
    {
        const struct body* a = &bodies[i];
        for (index_t j = i + 1; j < N_BODIES; j++, p++)
        {
            const struct body* b = &bodies[j];
            vec3_sub(dr[p], a->x, b->x);
            r2[p] = vec3_norm2(dr[p]);
        }
    }
}

// Calculate 1/rᵢⱼ from between bodies using full sqrt().  Called after
// calc_distances().
void calc_inv_r()
{
    for (index_t p = 0; p < N_PAIRS; p++)
        inv_r[p] = 1.0 / sqrt(r2[p]);
}

// Update 1 / rᵢⱼ between bodies using previous value and some polishing
// step/s.  Called after calc_distances().
void update_inv_r()
{
    for (index_t p = 0; p < N_PAIRS; p++)
    {
        // Typically two Newton steps or just one Halley step is required.
        for (index_t step = 0; step < NR_ITS; step++)
        {
            const double y = r2[p] * inv_r[p] * inv_r[p];
            // inv_r[p] *= 1.5 - 0.5 * y; // Newton step.
            inv_r[p] *= 0.125 * (15.0 + y * (3.0 * y - 10.0)); // Halley step.
        }

#ifndef NDEBUG
        const double error = fabs(inv_r[p] - 1.0 / sqrt(r2[p]));
        assert(error <= NR_TOL);
#endif
    }
}

// Advance bodies by 'dt'.  Called after calc_inv_r() or update_inv_r().
void advance()
{
    for (index_t i = 0, p = 0; i < N_BODIES; i++)
    {
        struct body* restrict a = &bodies[i];
        for (index_t j = i + 1; j < N_BODIES; j++, p++)
        {
            struct body* restrict b = &bodies[j];
            const double mag = dt * inv_r[p] / r2[p];

            vec3_t dv_a, dv_b;
            vec3_scale(dv_a, dr[p], b->mass * mag);
            vec3_scale(dv_b, dr[p], a->mass * mag);
            vec3_isub(a->v, dv_a);
            vec3_iadd(b->v, dv_b);
        }
    }

    for (index_t i = 0; i < N_BODIES; i++)
    {
        struct body* restrict a = &bodies[i];
        vec3_t del_x;
        vec3_scale(del_x, a->v, dt);
        vec3_iadd(a->x, del_x);
    }
}

// Calculate total energy. Called after calc_inv_r() or update_inv_r().
double energy()
{
    double e = 0.0;
    for (index_t i = 0, p = 0; i < N_BODIES; i++)
    {
        const struct body* a = &bodies[i];
        e += 0.5 * a->mass * vec3_norm2(a->v); // Kinetic Energy.

        for (index_t j = i + 1; j < N_BODIES; j++, p++)
        {
            const struct body* b = &bodies[j];
            e -= (a->mass * b->mass) * inv_r[p]; // Potential Energy.
        }
    }

    return e;
}

// Adjust Sun's momentum to offset remainder of the system.  Called after
// calc_inv_r().
void offset_momentum()
{
    vec3_t p = {0.0, 0.0, 0.0};
    for (index_t i = 0; i < N_BODIES; i++)
    {
        vec3_t del_p;
        vec3_scale(del_p, bodies[i].v, bodies[i].mass);
        vec3_iadd(p, del_p);
    }

    vec3_iscale(p, 1.0 / solar_mass);
    vec3_isub(bodies[0].v, p);
}

// -- Driver -------------------------------------------------------------------

int main(int argc, char **argv)
{
    const size_t n = argc > 1 ? atoi(argv[1]) : 1000;

    calc_distances();
    calc_inv_r(); // Use sqrt() on the first pass only.
    offset_momentum();
    printf("%.9f\n", energy());

    for (size_t i = 0; i < n; i++)
    {
        advance();
        calc_distances();
        update_inv_r(); // Update 1 / rᵢⱼ by refining existing value.
    }

    printf("%.9f\n", energy());
    return 0;
}
