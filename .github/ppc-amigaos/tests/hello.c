#include <stdio.h>
#include <proto/exec.h>
#include <proto/dos.h>
int main(void)
{
  APTR m = IExec->AllocVecTags(64, AVT_Type, MEMF_SHARED, TAG_DONE);
  IDOS->Printf("hello %ld\n", 42);
  IExec->FreeVec(m);
  printf("%f\n", 1.5);
  return 0;
}
