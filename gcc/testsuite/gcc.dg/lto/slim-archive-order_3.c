/* third archive member, needed by main, references the second one */
extern int f2 (int);

int
f3 (int a)
{
  return f2 (a) * 3;
}
