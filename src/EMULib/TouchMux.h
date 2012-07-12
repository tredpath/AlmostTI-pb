/** EMULib Emulation Library *********************************/
/**                                                         **/
/**                        TouchMux.h                       **/
/**                                                         **/
/** This file wraps Touch.c routines for multiple display   **/
/** depths (BPP8,BPP16,BPP32). It is used automatically     **/
/** when none of BPP* values are defined.                   **/
/**                                                         **/
/** Copyright (C) Marat Fayzullin 2008-2009                 **/
/**     You are not allowed to distribute this software     **/
/**     commercially. Please, notify me, if you make any    **/
/**     changes to this file.                               **/
/*************************************************************/
#ifndef TOUCHMUX_H
#define TOUCHMUX_H

#include "EMULib.h"
#include "Touch.h"

#undef BPP8
#undef BPP16
#undef BPP24
#undef BPP32
#undef PIXEL

#define BPP8
#define pixel           unsigned char
#define PIXEL(R,G,B)    (pixel)(((7*(R)/255)<<5)|((7*(G)/255)<<2)|(3*(B)/255))
#define DrawHLine       DrawHLine_8
#define DrawVLine       DrawVLine_8
#define DrawPenCues     DrawPenCues_8
#define DrawKeyboard    DrawKeyboard_8
#define PrintXY         PrintXY_8
#include "Touch.c"
#undef pixel
#undef PIXEL
#undef DrawHLine
#undef DrawVLine
#undef DrawPenCues
#undef DrawKeyboard
#undef PrintXY
#undef BPP8

#define BPP16
#define pixel           unsigned short
#if defined(UNIX) || defined(S60) || defined(UIQ) || defined(NXC2600) || defined(STMP3700) || defined(Q_OX_QNX)
/* Symbian and Unix use true 16BPP color */
#define PIXEL(R,G,B)    (pixel)(((31*(R)/255)<<11)|((63*(G)/255)<<5)|(31*(B)/255))
#else
/* Other platforms use 15BPP color */
#define PIXEL(R,G,B)    (pixel)(((31*(R)/255)<<10)|((31*(G)/255)<<5)|(31*(B)/255))
#endif	 
#define DrawHLine       DrawHLine_16
#define DrawVLine       DrawVLine_16
#define DrawPenCues     DrawPenCues_16
#define DrawKeyboard    DrawKeyboard_16
#define PrintXY         PrintXY_16
#include "Touch.c"
#undef pixel
#undef PIXEL
#undef DrawHLine
#undef DrawVLine
#undef DrawPenCues
#undef DrawKeyboard
#undef PrintXY
#undef BPP16

#define BPP32
#define pixel           unsigned int
#define PIXEL(R,G,B)    (pixel)(((int)R<<16)|((int)G<<8)|B)
#define DrawHLine       DrawHLine_32
#define DrawVLine       DrawVLine_32
#define DrawPenCues     DrawPenCues_32
#define DrawKeyboard    DrawKeyboard_32
#define PrintXY         PrintXY_32
#include "Touch.c"
#undef pixel
#undef PIXEL
#undef DrawHLine
#undef DrawVLine
#undef DrawPenCues
#undef DrawKeyboard
#undef PrintXY
#undef BPP32

/** pixel ****************************************************/
/** When no BPP* specified, we assume the pixel to have the **/
/** largest size and default to GetColor().                 **/
/*************************************************************/
#define pixel        unsigned int
#define PIXEL(R,G,B) GetColor(R,G,B)

/** DrawPenCues() ********************************************/
/** Overlay dotted cue lines for using PenJoystick() onto a **/
/** given image. Show dialpad cues if requested.            **/
/*************************************************************/
void DrawPenCues(Image *Img,int ShowDialpad,pixel Color)
{
  switch(Img->D)
  {
    case 8:  DrawPenCues_8(Img,ShowDialpad,Color);break;
    case 16: DrawPenCues_16(Img,ShowDialpad,Color);break;
    case 24:
    case 32: DrawPenCues_32(Img,ShowDialpad,Color);break;
  }
}

/** DrawKeyboard() *******************************************/
/** Draw virtual keyboard in a given image. Key modifiers   **/
/** and the key code passed in CurKey are highlighted.      **/
/*************************************************************/
void DrawKeyboard(Image *Img,unsigned int CurKey)
{
  switch(Img->D)
  {
    case 8:  DrawKeyboard_8(Img,CurKey);break;
    case 16: DrawKeyboard_16(Img,CurKey);break;
    case 24:
    case 32: DrawKeyboard_32(Img,CurKey);break;
  }
}

#endif /* TOUCHMUX_H */
