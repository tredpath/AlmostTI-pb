/** EMULib Emulation Library *********************************/
/**                                                         **/
/**                        LibUnix.h                        **/
/**                                                         **/
/** This file contains Unix-dependent definitions and       **/
/** declarations for the emulation library.                 **/
/**                                                         **/
/** Copyright (C) Marat Fayzullin 1996-2009                 **/
/**     You are not allowed to distribute this software     **/
/**     commercially. Please, notify me, if you make any    **/
/**     changes to this file.                               **/
/*************************************************************/
#ifndef LIBQNX_H
#define LIBQNX_H

#include <bps/navigator.h>
#include <bps/event.h>
#include <bps/screen.h>

/* X11 defines "Status" to be "int" but this may shadow some */
/* of our application variables!                             */
#undef Status

#ifdef __cplusplus
extern "C" {
#endif

#define SND_CHANNELS    16     /* Number of sound channels   */
#define SND_BITS        8
#define SND_BUFSIZE     (1<<SND_BITS)

                               /* X11SetEffects() arguments  */
#define EFF_NONE        0x000  /* No special effects         */
#define EFF_SCALE       0x001  /* Scale video to fill screen */
#define EFF_SOFTEN      0x002  /* Scale + soften video       */
#define EFF_TVLINES     0x004  /* Apply TV scanlines effect  */
#define EFF_SAVECPU     0x008  /* Suspend when inactive      */
#define EFF_SYNC        0x010  /* Wait for sync timer        */
#define EFF_MITSHM      0x020  /* Use MITSHM X11 extension   */
#define EFF_VARBPP      0x040  /* X11 determines Image depth */
#define EFF_VERBOSE     0x080  /* Report problems via printf */
#define EFF_SHOWFPS     0x100  /* Show current framerate     */

/** PIXEL() **************************************************/
/** Unix uses the 16bpp 5:6:5 pixel format.                 **/
/*************************************************************/
#if defined(BPP32) || defined(BPP24)
#define PIXEL(R,G,B)  (pixel)(((int)R<<16)|((int)G<<8)|B)
#define PIXEL2MONO(P) (((P>>16)&0xFF)+((P>>8)&0xFF)+(P&0xFF))/3)
#define RMASK 0xFF0000
#define GMASK 0x00FF00
#define BMASK 0x0000FF

#elif defined(BPP16)
#define PIXEL(R,G,B)  (pixel)(((31*(R)/255)<<11)|((63*(G)/255)<<5)|(31*(B)/255))
#define PIXEL2MONO(P) (522*(((P)&31)+(((P)>>5)&63)+(((P)>>11)&31))>>8)
#define RMASK 0xF800
#define GMASK 0x07E0
#define BMASK 0x001F

#elif defined(BPP8)
#define PIXEL(R,G,B)  (pixel)(((7*(R)/255)<<5)|((7*(G)/255)<<2)|(3*(B)/255))
#define PIXEL2MONO(P) (3264*((((P)<<1)&7)+(((P)>>2)&7)+(((P)>>5)&7))>>8)
#define RMASK 0xE0
#define GMASK 0x1C
#define BMASK 0x03
#endif

extern int  ARGC;
extern char **ARGV;

/** InitUnix() ***********************************************/
/** Initialize Unix/X11 resources and set initial window.   **/
/** title and dimensions.                                   **/
/*************************************************************/
int InitQNX(const char *Title,int Width,int Height);

/** TrashUnix() **********************************************/
/** Free resources allocated in InitUnix()                  **/
/*************************************************************/
void TrashQNX(void);

/** InitAudio() **********************************************/
/** Initialize sound. Returns rate (Hz) on success, else 0. **/
/** Rate=0 to skip initialization (will be silent).         **/
/*************************************************************/
unsigned int InitAudio(unsigned int Rate,unsigned int Latency);

/** TrashAudio() *********************************************/
/** Free resources allocated by InitAudio().                **/
/*************************************************************/
void TrashAudio(void);

/** PauseAudio() *********************************************/
/** Pause/resume audio playback.                            **/
/*************************************************************/
int PauseAudio(int Switch);

/** X11ProcessEvents() ***************************************/
/** Process X11 event messages.                             **/
/*************************************************************/
void BPSProcessEvents(void);

/** BPSSetEffects() ******************************************/
/** Set visual effects applied to video in ShowVideo().     **/
/*************************************************************/
void BPSSetEffects(int NewEffects);
int BPSGetEffects();
void BPSToggleEffects(int eff);

#ifdef __cplusplus
}
#endif
#endif /* LIBQNX_H */
