/* A static constructor must run in a base-relative program.  The startup
   code walks __CTOR_LIST__ + 1, a far object in a .list_* section kept in
   the text hunk; when the baserel pass read it through a4 the walker found
   a stray zero in data and ran nothing, with no diagnostic anywhere.  */
/* { dg-do run } */
/* { dg-options "-fbaserel" } */

static int flag;

struct Init
{
  Init () { flag = 42; }
};

static Init init;

int
main ()
{
  return flag == 42 ? 0 : 1;
}
