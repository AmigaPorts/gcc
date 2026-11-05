#ifndef SPE_BENCH_H
#define SPE_BENCH_H

#define BENCH_COUNT 4
#define VECTOR_COUNT 1024
#define IMAGE_WIDTH 160
#define IMAGE_HEIGHT 120
#define ITERATION_LIMIT 256

/* All public interfaces use pointers/integers across the AmigaOS4 ABI.  */
void bench_setup (void);
void bench_run (unsigned);
int bench_check (unsigned, unsigned *);
void reference_matrix_f (float *, const float *, const float *);
void reference_matrix_d (double *, const double *, const double *);
unsigned reference_mandel_f (float, float);
unsigned reference_mandel_d (double, double);
#endif
