/** AlmostTI: portable TI calcs emulator *********************/
/**                                                         **/
/**                          Help.h                         **/
/**                                                         **/
/** This file contains help information printed out by the  **/
/** main() routine when started with option "-help".        **/
/**                                                         **/
/** Copyright (C) Marat Fayzullin 1994-2009                 **/
/**     You are not allowed to distribute this software     **/
/**     commercially. Please, notify me, if you make any    **/
/**     changes to this file.                               **/
/*************************************************************/

char *HelpText[] =
{
  (char*)"Usage: ati85 [-option1 [-option2...]] [filename]",
  (char*)"[filename] = name of a RAM image to load",
  (char*)"[-option]   =",
  (char*)"  -ti83/-ti84         - Run in TI83/TI84+ mode [-ti85]",
  (char*)"  -ti83+/-ti84+       - Run in TI83+/TI84+ mode [-ti85]",
  (char*)"  -ti83se/-ti84se     - Run in TI83+/TI84+ Silver Edition mode [-ti85]",
  (char*)"  -ti85/-ti86         - Run in TI85/TI86 mode [-ti85]",
  (char*)"  -verbose <level>    - Select debugging messages [1]",
  (char*)"  -vperiod <period>   - Set interrupt period [24000 cycles]",
  (char*)"  -uperiod <period>   - Number of interrupts per screen update [2]",
  (char*)"  -home <dirname>     - Set directory with system ROM files [off]",
  (char*)"  -help               - Print this help page",
#ifdef MITSHM
  (char*)"  -shm/-noshm         - Use/don't use MIT SHM extensions for X [-shm]",
#endif
#ifdef DEBUG
  (char*)"  -trap <address>     - Trap execution when PC reaches address [FFFFh]",
#endif
#ifdef UNIX
  (char*)"  -saver/-nosaver     - Save/don't save CPU when inactive [-saver]",
#endif
  (char*)0
};
