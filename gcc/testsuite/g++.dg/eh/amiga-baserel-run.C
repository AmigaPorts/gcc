/* Base-relative code must use absolute EH pointers and select a matching
   runtime multilib.  */
/* { dg-do run { target m68k-*-amigaos* } } */
/* { dg-options "-O2 -fbaserel -mcrt=nix20" } */

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
