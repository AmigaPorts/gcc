/* Address 4 holds SysBase on AmigaOS, so reading it is the standard way to
   reach exec.library, not a null-page access.  -Wall used to report
   "array subscript 0 is outside array bounds" on every such read.
   With null-pointer deletion explicitly enabled, a dereference of a
   null pointer is still diagnosed.  */

/* { dg-do compile } */
/* { dg-skip-if "amiga SysBase" { ! { m68k-*-amigaos* } } } */
/* { dg-options "-O2 -Wall -Wnull-dereference -fdelete-null-pointer-checks" } */

struct ExecBase;
struct Task { int dummy; };
struct ExecBase { char pad[276]; struct Task *ThisTask; };

struct Task *
this_task (void)
{
  struct ExecBase *SysBase = *(struct ExecBase **) 4;
  return SysBase->ThisTask;
}

int
null_read (void)
{
  return *(int *) 0;	/* { dg-warning "null pointer dereference" } */
}
