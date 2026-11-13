/* { dg-do run } */
/* { dg-skip-if "fstat() is a stub in nix13" { *-*-* } { "-mcrt=nix13" } { "" } } */

/* stat() and fstat() in the board's C runtime: the type and size of a file
   and of a directory, and the error for a file that is not there.
   stat-mtime-run.c has the times and lstat-run.c has lstat().  */

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

#define NAME "T:gcc-stat-test"

static int failures;

static void
check (int ok, const char *what)
{
  if (!ok)
    {
      printf ("FAIL: %s\n", what);
      failures++;
    }
}

int
main (void)
{
  struct stat st, fst;
  int fd;

  fd = open (NAME, O_WRONLY | O_CREAT | O_TRUNC, 0666);
  check (fd >= 0, "open for writing");
  if (fd < 0)
    return 1;
  check (write (fd, "0123456789", 10) == 10, "write");

  memset (&st, 0x55, sizeof st);
  check (fstat (fd, &st) == 0, "fstat of the open file");
  check (S_ISREG (st.st_mode), "fstat: a regular file");
  /* The size is the file system's to report once the file is closed.  */
  check (close (fd) == 0, "close");

  memset (&st, 0x55, sizeof st);
  check (stat (NAME, &st) == 0, "stat of the file");
  check (S_ISREG (st.st_mode) && !S_ISDIR (st.st_mode), "stat: a regular file");
  check (st.st_size == 10, "stat: 10 bytes");
  check (st.st_nlink >= 1, "stat: at least one link");

  fd = open (NAME, O_RDONLY);
  memset (&fst, 0x55, sizeof fst);
  check (fd >= 0 && fstat (fd, &fst) == 0, "fstat of the file reopened");
  check (fst.st_mode == st.st_mode && fst.st_size == st.st_size,
	 "fstat agrees with stat");
  close (fd);

  memset (&st, 0x55, sizeof st);
  check (stat ("T:", &st) == 0, "stat of a directory");
  check (S_ISDIR (st.st_mode) && !S_ISREG (st.st_mode), "stat: a directory");

  check (remove (NAME) == 0, "remove");
  errno = 0;
  check (stat (NAME, &st) == -1 && errno == ENOENT,
	 "stat of a missing file fails with ENOENT");
  errno = 0;
  check (fstat (-1, &st) == -1 && errno == EBADF,
	 "fstat of a bad descriptor fails with EBADF");

  return failures;
}
