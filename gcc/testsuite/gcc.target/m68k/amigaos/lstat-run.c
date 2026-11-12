/* { dg-do run } */
/* { dg-xfail-run-if "clib2 locks the file with a packet, which volamos does not answer" { *-*-* } { "-mcrt=clib2" } { "" } } */

/* lstat() in the board's C runtime: on a file that is no link it reports
   what stat() does, and a file that is not there is ENOENT.  */

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

#define NAME "T:gcc-lstat-test"

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
  struct stat st, lst;
  int fd;

  fd = open (NAME, O_WRONLY | O_CREAT | O_TRUNC, 0666);
  check (fd >= 0, "open for writing");
  if (fd < 0)
    return 1;
  check (write (fd, "0123456789", 10) == 10, "write");
  check (close (fd) == 0, "close");

  check (stat (NAME, &st) == 0, "stat of the file");
  memset (&lst, 0x55, sizeof lst);
  check (lstat (NAME, &lst) == 0, "lstat of the file");
  check (S_ISREG (lst.st_mode) && !S_ISLNK (lst.st_mode), "lstat: a regular file");
  check (lst.st_mode == st.st_mode, "lstat: the mode stat reports");
  check (lst.st_size == 10, "lstat: 10 bytes");
  check (lst.st_mtime == st.st_mtime, "lstat: the time stat reports");

  memset (&lst, 0x55, sizeof lst);
  check (lstat ("T:", &lst) == 0 && S_ISDIR (lst.st_mode), "lstat of a directory");

  check (remove (NAME) == 0, "remove");
  errno = 0;
  check (lstat (NAME, &lst) == -1 && errno == ENOENT,
	 "lstat of a missing file fails with ENOENT");

  return failures;
}
