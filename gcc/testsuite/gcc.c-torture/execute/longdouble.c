/* { dg-do run } */
/* { dg-options "-O0 -fno-inline -fno-ipa-icf -fno-ipa-sra" } */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Prevent constant folding of our test functions.  */
__attribute__((noinline))
long double ld_add(long double a, long double b) { return a + b; }

__attribute__((noinline))
long double ld_sub(long double a, long double b) { return a - b; }

__attribute__((noinline))
long double ld_mul(long double a, long double b) { return a * b; }

__attribute__((noinline))
long double ld_div(long double a, long double b) { return a / b; }

/* Helper to compare long double values using GCC frontend constants.  */
static void check(const char *name, long double got, long double expect)
{
    if (got != expect)
    {
        printf("FAIL: %s\n", name);
        printf(" got    = %.40Lg\n", got);
        printf(" expect = %.40Lg\n", expect);
        abort();
    }
}

int main(void)
{
#if defined(__SIZEOF_LONG_DOUBLE__) && __SIZEOF_LONG_DOUBLE__ == 12
    /* Long double is 12 bytes (80-bit extended format with 2 bytes padding).  */
    /* Run the tests.  */

    /* Basic values */
    long double a = 3.1415926535897932384626433832795L;
    long double b = 2.7182818284590452353602874713527L;

    /* ADD */
    check("add", ld_add(a, b), a + b);

    /* SUB */
    check("sub", ld_sub(a, b), a - b);

    /* MUL */
    check("mul", ld_mul(a, b), a * b);

    /* DIV */
    check("div", ld_div(a, b), a / b);

    /* Zero, sign, small/large */
    check("zero-add", ld_add(0.0L, a), 0.0L + a);
    check("zero-sub", ld_sub(0.0L, a), 0.0L - a);
    check("zero-mul", ld_mul(0.0L, a), 0.0L * a);
    check("zero-div", ld_div(a, 1.0L), a / 1.0L);

    /* Large exponent range */
    long double x = 1.0e3000L;
    long double y = 1.0e-3000L;

    check("large-mul", ld_mul(x, y), x * y);
    check("large-div", ld_div(x, y), x / y);

    /* Negative values */
    check("neg-add", ld_add(-a, b), -a + b);
    check("neg-mul", ld_mul(-a, -b), -a * -b);

    /* Test with 80-bit specific values */
    long double pi = 3.141592653589793238462643383279502884197L;
    long double e = 2.718281828459045235360287471352662497757L;
    check("pi", ld_add(pi, 0.0L), pi);
    check("e", ld_add(e, 0.0L), e);
    check("pi-e", ld_sub(pi, e), pi - e);
    check("pi*e", ld_mul(pi, e), pi * e);

#else
    /* Long double is not 80-bit extended format (12 bytes).  */
    /* Skip tests and return success.  */
    printf("Skipping: long double is not 80-bit extended format (size=%d)\n",
           (int)sizeof(long double));
#endif

    return 0;
}

