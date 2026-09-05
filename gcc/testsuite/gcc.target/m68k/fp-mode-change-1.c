/* A caller-saved FPU register is saved and restored in XFmode even when
   the value being preserved is SFmode.  Late combine must not replace
   the SFmode use with the low 32 bits of that XFmode save slot.  */

/* { dg-do compile } */
/* { dg-options "-O2 -mcpu=68040 -mhard-float -fno-fast-math -fdump-rtl-late_combine2" } */

extern float get_value (void);
extern void clobber (void);

_Bool
negative (void)
{
  float x = get_value ();
  if (__builtin_fabsf (x) > 0.4f)
    {
      clobber ();
      return x < 0.0f;
    }
  return 0;
}

/* The XFmode restore to fp0 must survive late combine.  */
/* { dg-final { scan-rtl-dump "\\(set \\(reg:XF 16 fp0\\)" "late_combine2" } } */
