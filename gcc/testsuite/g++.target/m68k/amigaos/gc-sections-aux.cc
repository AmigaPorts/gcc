// The other file of gc-sections-run.C.

extern int flag;

struct Init
{
  Init () { flag = 42; }
};

static Init init;
