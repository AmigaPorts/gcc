// The other file of tls-extern-init-run.C.

int f ();

thread_local int x = f ();
thread_local int y = 7;
