/* -resident is -fbaserel plus a relocated data segment per invocation,
   and the only way to the base-relative multilibs without an explicit
   -fbaserel: a wrong multilib match linked this against the absolute
   libgcc and libstdc++, which crashed before reaching the throw.  */
/* { dg-do run { target m68k-*-amigaos* } } */
/* { dg-options "-O2 -resident -mcrt=nix20" } */

__attribute__((noinline)) static void
raise_value (int value)
{
  throw value;
}

int
main ()
{
  try
    {
      raise_value (42);
    }
  catch (int value)
    {
      return value != 42;
    }
  return 1;
}
