// { dg-do compile }
// { dg-options "-Wno-pedantic -Wplacement-new=1" }
// Struct tail padding is 1 byte with the 2-byte int alignment of m68k, not
// 3, so the flexible array member warnings come out differently.
// { dg-skip-if "int alignment is 2" { m68k*-*-* } { "*" } { "-malign-int" } }

typedef __typeof__ (sizeof 0) size_t;

void* operator new (size_t, void *p) { return p; }
void* operator new[] (size_t, void *p) { return p; }

struct Ax { char n, a []; };

typedef __INT16_TYPE__ Int16;
typedef __INT32_TYPE__ Int32;

struct BAx { int i; Ax ax; };

void fBx1 ()
{
  static BAx bax1 = { 1, /* Ax = */ { 2, /* a[] = */ { 3 } } }; // { dg-error "initialization of flexible array member in a nested context" }

  // The first three bytes of the flexible array member live in the padding.
  new (bax1.ax.a) char;     // { dg-warning "placement" "" { target default_packed } }
  new (bax1.ax.a) char[2];  // { dg-warning "placement" "" { target default_packed } }
  new (bax1.ax.a) Int16;    // { dg-warning "placement" "" { target default_packed } }
  new (bax1.ax.a) Int32;    // { dg-warning "placement" }
}

void fBx2 ()
{
  static BAx bax2 = { 1, /* Ax = */ { 2, /* a[] = */ { 3, 4 } } }; // { dg-error "initialization of flexible array member in a nested context" }

  // The first three bytes of the flexible array member live in the padding.
  new (bax2.ax.a) char;       // { dg-warning "placement" "" { target default_packed } }
  new (bax2.ax.a) char[2];    // { dg-warning "placement" "" { target default_packed } }
  new (bax2.ax.a) char[3];    // { dg-warning "placement" "" { target default_packed } }
  new (bax2.ax.a) Int16;      // { dg-warning "placement" "" { target default_packed } }
  new (bax2.ax.a) char[4];    // { dg-warning "placement" }
  new (bax2.ax.a) Int32;      // { dg-warning "placement" }
}

void fBx3 ()
{
  static BAx bax2 = { 1, /* Ax = */ { 3, /* a[] = */ { 4, 5, 6 } } }; // { dg-error "initialization of flexible array member in a nested context" }

  // The first three bytes of the flexible array member live in the padding.
  new (bax2.ax.a) char;       // { dg-warning "placement" "" { target default_packed } }
  new (bax2.ax.a) char[2];    // { dg-warning "placement" "" { target default_packed } }
  new (bax2.ax.a) Int16;      // { dg-warning "placement" "" { target default_packed } }
  new (bax2.ax.a) char[3];    // { dg-warning "placement" "" { target default_packed } }
  new (bax2.ax.a) char[4];    // { dg-warning "placement" }
  new (bax2.ax.a) Int32;      // { dg-warning "placement" }
}
