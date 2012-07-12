/** AlmostTI: portable TI calcs emulator *********************/
/**                                                         **/
/**                          ATI85.c                        **/
/**                                                         **/
/** This file contains generic main() procedure statrting   **/
/** the emulation.                                          **/
/**                                                         **/
/** Copyright (C) Marat Fayzullin 1994-2009                 **/
/**     You are not allowed to distribute this software     **/
/**     commercially. Please, notify me, if you make any    **/
/**     changes to this file.                               **/
/*************************************************************/

#include "../EMULib/EMULib.h"
#include "TI85.h"
#include "Help.h"
#include "QNX/QNXMenu.h"

#include <bps/bps.h>
#include <bps/navigator.h>
#include <bps/dialog.h>

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

char *Options[]=
{ 
  (char*)"ti82",(char*)"ti83",(char*)"ti83+",(char*)"ti83se",(char*)"ti84",(char*)"ti84+",(char*)"ti84se",(char*)"ti85",(char*)"ti86",
  (char*)"verbose",(char*)"uperiod",(char*)"help",(char*)"home",(char*)"link",(char*)"trap",
  (char*)"shm",(char*)"noshm",(char*)"saver",(char*)"nosaver",
  (char*)0
};

extern int   UseEffects; /* Platform-dependent effects          */
extern int   SyncScreen; /* Sync screen updates (#ifdef MSDOS)  */
extern int   FullScreen; /* Use 320x240 VGA (#ifdef MSDOS)      */

int main(int argc,char *argv[])
{
	bps_initialize();

	mkdir("shared/misc/TI", 0777);
	mkdir("shared/misc/TI/ROM/", 0777);
	mkdir("shared/misc/TI/RAM/", 0777);

	dialog_request_events(0);
	UpdateRomList();

    int M, T;
	if ((M = AutoLoadRom(T)) < 0)
	{
		bps_shutdown();
		return 0;
	}
    Mode=(Mode&~ATI_MODEL)|M;

  int N,J,I;

#ifdef DEBUG
  CPU.Trap  = 0xFFFF;
  CPU.Trace = 0;
#endif

#if defined(UNIX) || defined(MAEMO) || defined(Q_OS_QNX)
  ARGC = argc;
  ARGV = argv;
#endif

#if defined(MSDOS) || defined(WINDOWS)
  /* No separate console, so no messages */
  Verbose=0;
  /* Figure out program directory name */
  ProgDir=GetFilePath(argv[0]);
#else
  Verbose=5;
  ProgDir = "shared/misc/TI/ROM";
#endif

  if (navigator_request_events(0) != BPS_SUCCESS)
  {
	  bps_shutdown();
	  return 0;
  }

  RAMFile = (char*)malloc(sizeof(char)*strlen(Config[T].RAMFile));
  strcpy(RAMFile, Config[T].RAMFile);
  if(!InitMachine()) return(1);
  StartTI85();
  TrashTI85();
  TrashMachine();

  free(RAMFile);

  bps_shutdown();

  return(0);
}
