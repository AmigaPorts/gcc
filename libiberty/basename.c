/* Return the basename of a pathname.
   This file is in the public domain. */

/*

@deftypefn Supplemental char* basename (const char *@var{name})

Returns a pointer to the last component of pathname @var{name}.
Behavior is undefined if the pathname ends in a directory separator.

@end deftypefn

*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "ansidecl.h"
#include "libiberty.h"
#include "safe-ctype.h"
#include "filenames.h"

char *
basename (const char *name)
{
  const char *base;

#if defined (HAVE_DOS_BASED_FILE_SYSTEM)
  /* Skip over the disk name in MSDOS pathnames. */
  if (ISALPHA (name[0]) && name[1] == ':') 
    name += 2;
#endif

  for (base = name; *name; name++)
    {
      if (IS_DIR_SEPARATOR (*name))
	{
	  base = name + 1;
	}
    }
  return (char *) base;
}

