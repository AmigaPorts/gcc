#include "bench.h"

/* Dyadic inputs keep matrix results exact, including after reassociation.  */
static float matrix_f[16] __attribute__((aligned(8)));
static double matrix_d[16] __attribute__((aligned(8)));
static float input_f[VECTOR_COUNT * 4] __attribute__((aligned(8)));
static double input_d[VECTOR_COUNT * 4] __attribute__((aligned(8)));
static float output_f[VECTOR_COUNT * 4] __attribute__((aligned(8)));
static double output_d[VECTOR_COUNT * 4] __attribute__((aligned(8)));
static float expected_f[VECTOR_COUNT * 4];
static double expected_d[VECTOR_COUNT * 4];
static unsigned image_f[IMAGE_WIDTH * IMAGE_HEIGHT];
static unsigned image_d[IMAGE_WIDTH * IMAGE_HEIGHT];
static unsigned expected_image_f[IMAGE_WIDTH * IMAGE_HEIGHT];
static unsigned expected_image_d[IMAGE_WIDTH * IMAGE_HEIGHT];

/* A row-major matrix transforms independent vectors.  No intrinsics,
   alignment assumptions, fast-math, or hand-written assembly are needed.  */
#define MATRIX(NAME, TYPE) \
void NAME (TYPE *restrict out, const TYPE *restrict m, \
	   const TYPE *restrict in, unsigned count) \
{ \
  for (unsigned i = 0; i < count; ++i) \
    for (unsigned r = 0; r < 4; ++r) \
      out[4*i+r] = m[4*r] * in[4*i] + m[4*r+1] * in[4*i+1] \
		   + m[4*r+2] * in[4*i+2] + m[4*r+3] * in[4*i+3]; \
}
MATRIX (transform_f, float)
MATRIX (transform_d, double)

#define MANDEL(NAME, TYPE) \
void NAME (unsigned *out) \
{ \
  for (unsigned row = 0; row < IMAGE_HEIGHT; ++row) \
    for (unsigned col = 0; col < IMAGE_WIDTH; ++col) \
      { \
	TYPE cr = (TYPE) ((int) col - 120) * (TYPE) 0.015625; \
	TYPE ci = (TYPE) ((int) row - 60) * (TYPE) 0.015625; \
	TYPE x = 0, y = 0, xx = 0, yy = 0; \
	unsigned n = 0; \
	while (n < ITERATION_LIMIT && xx + yy < (TYPE) 4) \
	  { \
	    y = (TYPE) 2 * x * y + ci; \
	    x = xx - yy + cr; \
	    xx = x * x; yy = y * y; \
	    ++n; \
	  } \
	out[row * IMAGE_WIDTH + col] = n; \
      } \
}
MANDEL (mandelbrot_f, float)
MANDEL (mandelbrot_d, double)

void
bench_setup (void)
{
  for (unsigned i = 0; i < 16; ++i)
    {
      matrix_f[i] = ((int) (i * 7 % 19) - 9) * 0.125f;
      matrix_d[i] = matrix_f[i];
    }
  for (unsigned i = 0; i < VECTOR_COUNT * 4; ++i)
    {
      input_f[i] = ((int) (i * 13 % 251) - 125) * 0.0625f;
      input_d[i] = input_f[i];
    }
  for (unsigned i = 0; i < VECTOR_COUNT; ++i)
    {
      reference_matrix_f (expected_f + 4*i, matrix_f, input_f + 4*i);
      reference_matrix_d (expected_d + 4*i, matrix_d, input_d + 4*i);
    }
  for (unsigned row = 0; row < IMAGE_HEIGHT; ++row)
    for (unsigned col = 0; col < IMAGE_WIDTH; ++col)
      {
	float cr = ((int) col - 120) * 0.015625f;
	float ci = ((int) row - 60) * 0.015625f;
	unsigned i = row * IMAGE_WIDTH + col;
	expected_image_f[i] = reference_mandel_f (cr, ci);
	expected_image_d[i] = reference_mandel_d (cr, ci);
      }
}

void
bench_run (unsigned kind)
{
  switch (kind)
    {
    case 0: transform_f (output_f, matrix_f, input_f, VECTOR_COUNT); break;
    case 1: transform_d (output_d, matrix_d, input_d, VECTOR_COUNT); break;
    case 2: mandelbrot_f (image_f); break;
    case 3: mandelbrot_d (image_d); break;
    }
}

int
bench_check (unsigned kind, unsigned *hash)
{
  unsigned h = 2166136261u;
  unsigned count = kind < 2 ? VECTOR_COUNT * 4 : IMAGE_WIDTH * IMAGE_HEIGHT;
  for (unsigned i = 0; i < count; ++i)
    {
      unsigned value;
      switch (kind)
	{
	case 0:
	  if (output_f[i] != expected_f[i]) return 1;
	  value = (unsigned) (int) (output_f[i] * 1024.0f); break;
	case 1:
	  if (output_d[i] != expected_d[i]) return 1;
	  value = (unsigned) (int) (output_d[i] * 1024.0); break;
	case 2:
	  if (image_f[i] != expected_image_f[i]) return 1;
	  value = image_f[i]; break;
	default:
	  if (image_d[i] != expected_image_d[i]) return 1;
	  value = image_d[i]; break;
	}
      h = (h ^ value) * 16777619u;
    }
  *hash = h;
  return 0;
}
