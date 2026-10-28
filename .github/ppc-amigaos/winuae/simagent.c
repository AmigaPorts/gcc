/* The AmigaOS 4 side of amigaos-sim: runs the programs the host queues.

   The host shares a directory with the emulator as SIMSHARE:.  For each
   job it writes SIMSHARE:jobs/ID/ with the program and a script, run, and
   then an empty SIMSHARE:queue/ID.  This picks the queue entry up, deletes
   it and starts "Execute SIMSHARE:jobs/ID/run" without waiting for it; the
   script runs the program and leaves its return code behind for the host.
   A program that crashes only stops its own job.

   SIMSHARE:agent.alive is rewritten every two seconds, so that the host
   can tell that this is running.

   Build it with ppc-amigaos-gcc -O2 -o simagent simagent.c and start it
   from S:User-Startup:

	Run >NIL: SIMSHARE:simagent
*/

#include <proto/exec.h>
#include <proto/dos.h>
#include <stdio.h>
#include <string.h>

#define MAX_JOBS 64

static void
heartbeat (void)
{
  BPTR fh = IDOS->Open ("SIMSHARE:agent.alive", MODE_NEWFILE);

  if (fh)
    {
      IDOS->FPuts (fh, "alive\n");
      IDOS->Close (fh);
    }
}

/* The names in SIMSHARE:queue, at most MAX_JOBS of them. */
static int
read_queue (char names[MAX_JOBS][64])
{
  APTR context;
  struct ExamineData *data;
  int count = 0;

  context = IDOS->ObtainDirContextTags (EX_StringNameInput, "SIMSHARE:queue",
					EX_DataFields, EXF_NAME | EXF_TYPE,
					TAG_END);
  if (context == NULL)
    return 0;

  while (count < MAX_JOBS && (data = IDOS->ExamineDir (context)) != NULL)
    {
      if (EXD_IS_FILE (data) && strlen (data->Name) < 64)
	strcpy (names[count++], data->Name);
    }

  IDOS->ReleaseDirContext (context);

  return count;
}

static void
start_job (const char *name)
{
  char path[128], command[160];
  BPTR in, out;

  snprintf (path, sizeof path, "SIMSHARE:queue/%.63s", name);
  if (!IDOS->Delete (path))
    return;

  snprintf (command, sizeof command, "Execute SIMSHARE:jobs/%.63s/run", name);

  in = IDOS->Open ("NIL:", MODE_OLDFILE);
  out = IDOS->Open ("NIL:", MODE_NEWFILE);

  if (IDOS->SystemTags (command,
			SYS_Input, in,
			SYS_Output, out,
			SYS_Asynch, TRUE,
			NP_Name, "amigaos-sim job",
			NP_StackSize, 256 * 1024,
			TAG_END) == -1)
    {
      IDOS->Close (in);
      IDOS->Close (out);
    }
}

int
main (void)
{
  char names[MAX_JOBS][64];
  int ticks = 0;

  {
    BPTR lock;

    if ((lock = IDOS->CreateDir ("SIMSHARE:queue")) != ZERO)
      IDOS->UnLock (lock);
    if ((lock = IDOS->CreateDir ("SIMSHARE:jobs")) != ZERO)
      IDOS->UnLock (lock);
  }

  for (;;)
    {
      int count, i;

      if (ticks-- <= 0)
	{
	  heartbeat ();
	  ticks = 20;
	}

      count = read_queue (names);
      for (i = 0; i < count; i++)
	start_job (names[i]);

      /* Ctrl-C ends it. */
      if (IExec->SetSignal (0, SIGBREAKF_CTRL_C) & SIGBREAKF_CTRL_C)
	break;

      IDOS->Delay (count ? 1 : 5);
    }

  return 0;
}
