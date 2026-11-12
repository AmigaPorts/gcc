/* { dg-do run } */
/* { dg-require-effective-target amigaos_file_dates } */

/* The modification time in stat() and fstat(): a file just written was
   modified now, by the clock that time() reads, and utime() sets another
   time.  */

#include <fcntl.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <utime.h>
#include <sys/stat.h>

#define NAME "T:gcc-stat-mtime-test"

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

/* Within a minute: a time zone applied to one clock and not to the other
   is off by a quarter of an hour at least.  */
static int
close_to (time_t t, time_t now)
{
  return t >= now - 60 && t <= now + 60;
}

int
main (void)
{
  struct stat st, other;
  struct utimbuf ut;
  time_t now;
  int fd;

  fd = open (NAME, O_WRONLY | O_CREAT | O_TRUNC, 0666);
  check (fd >= 0, "open for writing");
  if (fd < 0)
    return 1;
  check (write (fd, "x", 1) == 1, "write");
  check (close (fd) == 0, "close");
  now = time (NULL);

  check (stat (NAME, &st) == 0, "stat");
  if (!close_to (st.st_mtime, now))
    printf ("st_mtime %ld, time() %ld\n", (long) st.st_mtime, (long) now);
  check (close_to (st.st_mtime, now), "stat: modified now");

  fd = open (NAME, O_RDONLY);
  check (fd >= 0 && fstat (fd, &other) == 0 && other.st_mtime == st.st_mtime,
	 "fstat: the same time as stat");
  close (fd);

  /* 2001-09-09 01:46:40 UTC */
  ut.actime = ut.modtime = 1000000000;
  check (utime (NAME, &ut) == 0, "utime");
  check (stat (NAME, &st) == 0 && st.st_mtime == 1000000000,
	 "stat: the time utime set");

  check (remove (NAME) == 0, "remove");
  return failures;
}
