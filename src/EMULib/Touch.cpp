/** EMULib Emulation Library *********************************/
/**                                                         **/
/**                          Touch.c                        **/
/**                                                         **/
/** This file contains functions that simulate joystick and **/
/** dialpad with the touch screen. It is normally used from **/
/** the platform-dependent functions that know where to get **/
/** pen coordinates from and where to draw pen cues to.     **/
/**                                                         **/
/** Copyright (C) Marat Fayzullin 2008-2009                 **/
/**     You are not allowed to distribute this software     **/
/**     commercially. Please, notify me, if you make any    **/
/**     changes to this file.                               **/
/*************************************************************/
#if !defined(BPP32) && !defined(BPP24) && !defined(BPP16) && !defined(BPP8)
#include "TouchMux.h"
#else

#include "EMULib.h"
#include "Console.h"
#include "Touch.h"
#include <string.h>

#define CLR_NORMALF PIXEL(0,0,0)         /* Normal key foreground   */
#define CLR_NORMALB PIXEL(255,255,255)   /* Normal key background   */
#define CLR_ACTIVEF PIXEL(255,255,255)   /* Active key foreground   */
#define CLR_ACTIVEB PIXEL(255,64,64)     /* Active key background   */

#ifndef DEFINE_ONCE
#define DEFINE_ONCE

/* Currently selected virtual keyboard key */
static int KBDXPos = 0;
static int KBDYPos = 0;

/* Horizontal offsets of virtual keyboard lines */
static const int KBDOffsets[YKEYS] = { 0,0,4,-4,8,4 };

/* Characters printed on virtual keyboard keys */
static const unsigned char *KBDLines[YKEYS+1] =
{
  (unsigned char*)"\33\20\21\22\23\24\25\26\27\16\17\10",
  (unsigned char*)"1234567890-=",
  (unsigned char*)"\11QWERTYUIOP",
  (unsigned char*)"^ASDFGHJKL;\15",
  (unsigned char*)"ZXCVBNM,./",
  (unsigned char*)"[]     \\'",
  (unsigned char*)0
};

/* Characters returned from virtual keyboard */
static unsigned char KBDKeys[YKEYS][XKEYS] =
{
  {
    0x1B,CON_F1,CON_F2,CON_F3,CON_F4,CON_F5,
    CON_F6,CON_F7,CON_F8,CON_INSERT,CON_DELETE,CON_BS
  },
  { '1','2','3','4','5','6','7','8','9','0','-','=' },
  { CON_TAB,'Q','W','E','R','T','Y','U','I','O','P',0 },
  { '^','A','S','D','F','G','H','J','K','L',';',CON_ENTER },
  { 'Z','X','C','V','B','N','M',',','.','/',0,0 },
  { '[',']',' ',' ',' ',' ',' ','\\','\'',0,0,0 }
};

/** GenericPenJoystick() *************************************/
/** Get simulated joystick buttons from touch screen UI.    **/
/** Result compatible with GetJoystick().                   **/
/*************************************************************/
unsigned int GenericPenJoystick(int X,int Y,int W,int H)
{
  unsigned int J;

  /* Simulate joystick when pen touches the screen at X,Y */
  J = 0;

  /* Don't accept touches outside of the window frame */
  if((X<0)||(Y<0)||(X>=W)||(Y>=H)) return(0);
  W/= 3;

  /* Top 1/16 of the widget: FIREL and FIRER */
  if(Y<(H>>4))
  { if(X<W) J|=BTN_FIREL; else if(X>=(W<<1)) J|=BTN_FIRER; }

  /* Bottom 1/16 of the widget: SELECT/EXIT and START */
  if(!J&&(Y>=(H-(H>>4))))
  { if(X<W) J|=BTN_SELECT|BTN_EXIT; else if(X>=(W<<1)) J|=BTN_START; }

  /* Right 1/3 of the screen is the fire buttons */
  if(!J&&(X>=(W<<1)))
  {
    /* Fire buttons overlap */
    if(Y<=(H>>1)+(H>>4)) J|=BTN_FIREB;
    if(Y>=(H>>1)-(H>>4)) J|=BTN_FIREA;
  }

  /* Left 1/3 of the screen is the directional pad */
  if(!J&&(X<W))
  {
    H/=3;
    W/=3;
    if(X<W) J|=BTN_LEFT; else if(X>=(W<<1)) J|=BTN_RIGHT;
    if(Y<H) J|=BTN_UP;   else if(Y>=(H<<1)) J|=BTN_DOWN;
  }

  /* Done, return simulated "joystick" state */
  return(J);
}

/** GenericPenDialpad() **************************************/
/** Get simulated dialpad buttons from touch screen UI.     **/
/*************************************************************/
unsigned char GenericPenDialpad(int X,int Y,int W,int H)
{
  /* Dialpad is the middle 1/3 of the screen */
  W /= 3;
  return(
    (Y>=0)&&(Y<H)&&(X>=W)&&(X<(W<<1))?
    3*(X-W)/W+3*(Y/(H>>2)) : 0
  );
}

/** GenericPenKeyboard() *************************************/
/** Get virtual on-screen keyboard buttons.                 **/
/*************************************************************/
unsigned char GenericPenKeyboard(int X,int Y,int W,int H)
{
  int J;

  /* Pen coordinates relative to keyboard's top left corner */
  X -= W-KEYSTEP*XKEYS-8;
  Y -= H-KEYSTEP*YKEYS-8;

  /* Pen must be inside the keyboard */
  if((X<0)||(Y<0)) return(0);

  /* Keyboard row index */
  Y/= KEYSTEP;
  if(Y>=YKEYS) return(0);

  /* Adjust for row position on screen */
  for(J=0;J<Y;++J) X-=KBDOffsets[J];
  if(X<0) return(0);

  /* Keyboard column index */
  X/= KEYSTEP;
  if(X>=XKEYS) return(0);

  /* Memorize last pressed key */
  KBDXPos = X;
  KBDYPos = Y;

  /* Return key */
  return(KBDKeys[Y][X]);
}

/** GenericDialKeyboard() ************************************/
/** Process dialpad input to the virtual keyboard. Returns  **/
/** virtual keyboard key if selected, or 0 if not.          **/
/*************************************************************/
unsigned char GenericDialKeyboard(unsigned char Key)
{
  /* Interpret input key */
  switch(Key)
  {
    case CON_LEFT:
      KBDXPos = (KBDXPos>0? KBDXPos:strlen((char*)KBDLines[KBDYPos]))-1;
      break;
    case CON_RIGHT:
      KBDXPos = KBDXPos<strlen((char*)KBDLines[KBDYPos])-1? KBDXPos+1:0;
      break;
    case CON_UP:
      KBDYPos = KBDYPos>0? KBDYPos-1:YKEYS-1;
      KBDXPos = KBDXPos<strlen((char*)KBDLines[KBDYPos])? KBDXPos:strlen((char*)KBDLines[KBDYPos])-1;
      break;
    case CON_DOWN:
      KBDYPos = KBDYPos<YKEYS-1? KBDYPos+1:0;
      KBDXPos = KBDXPos<strlen((char*)KBDLines[KBDYPos])? KBDXPos:strlen((char*)KBDLines[KBDYPos])-1;
      break;
    case CON_OK:
      /* Return ASCII character */
      return(KBDLines[KBDYPos][KBDXPos]);
  }

  /* Key has not been interpreted */
  return(0);
}

#endif /* DEFINE_ONCE */

/** DrawVLine()/DrawHLine() **********************************/
/** Draw dotted lines used to show cues for PenJoystick().  **/
/*************************************************************/
static void DrawVLine(Image *Img,int X,int Y1,int Y2,pixel Color)
{
  pixel *P;
  int J;

  if(Y1>Y2) { J=Y1;Y1=Y2;Y2=J; }
  P = (pixel *)Img->Data+Img->L*Y1+X;  
  for(J=Y1;J<=Y2;J+=4) { *P=Color;P+=Img->L<<2; }
}

static void DrawHLine(Image *Img,int X1,int X2,int Y,pixel Color)
{
  pixel *P;
  int J;

  if(X1>X2) { J=X1;X1=X2;X2=J; }
  P = (pixel *)Img->Data+Img->L*Y+X1;
  for(J=X1;J<=X2;J+=4) { *P=Color;P+=4; }
}

/** DrawPenCues() ********************************************/
/** Overlay dotted cue lines for using PenJoystick() onto a **/
/** given image. Show dialpad cues if requested.            **/
/*************************************************************/
void DrawPenCues(Image *Img,int ShowDialpad,pixel Color)
{
  pixel *P;
  int W,H,W9,W3,H3;

  P  = (pixel *)Img->Data;
  W  = Img->W;
  H  = Img->H;
  W9 = W/9;
  W3 = W/3;
  H3 = H/3;

  /* Vertical edges */
  DrawVLine(Img,W3,0,H-1,Color);
  DrawVLine(Img,W-W3,0,H-1,Color);

  /* Corner buttons */
  DrawHLine(Img,0,W3,H>>4,Color);
  DrawHLine(Img,W-W3,W-1,H>>4,Color);
  DrawHLine(Img,0,W3,H-(H>>4),Color);
  DrawHLine(Img,W-W3,W-1,H-(H>>4),Color);

  /* Fire buttons (with overlap) */
  DrawHLine(Img,W-W3,W-1,(H>>1)-(H>>4),Color);
  DrawHLine(Img,W-W3,W-1,(H>>1)+(H>>4),Color);

  /* Directional buttons */
  DrawVLine(Img,W9,H>>4,H-(H>>4),Color);
  DrawVLine(Img,W3-W9,H>>4,H-(H>>4),Color);
  DrawHLine(Img,0,W3,H3,Color);
  DrawHLine(Img,0,W3,H-H3,Color);

  /* Button labels */
  PrintXY(Img,"L",2,2,Color,-1);
  PrintXY(Img,"R",W-W3+2,2,Color,-1);
  PrintXY(Img,"B",W-W3+2,(H>>4)+2,Color,-1);
  PrintXY(Img,"A+B",W-W3+2,(H>>1)-(H>>4)+2,Color,-1);
  PrintXY(Img,"A",W-W3+2,(H>>1)+(H>>4)+2,Color,-1);
  PrintXY(Img,"SELECT",2,H-(H>>4)+2,Color,-1);
  PrintXY(Img,"START",W-W3+2,H-(H>>4)+2,Color,-1);

  /* If requested, show on-screen dialpad */
  if(ShowDialpad)
  {
    DrawHLine(Img,W3,W-W3,H>>2,Color);
    DrawHLine(Img,W3,W-W3,H>>1,Color);
    DrawHLine(Img,W3,W-W3,H-(H>>2),Color);
    DrawVLine(Img,W3+W9,0,H-1,Color);
    DrawVLine(Img,W-W3-W9,0,H-1,Color);
    PrintXY(Img,"1",W3+2,2,Color,-1);
    PrintXY(Img,"2",W3+W9+2,2,Color,-1);
    PrintXY(Img,"3",W-W3-W9+2,2,Color,-1);
    PrintXY(Img,"4",W3+2,(H>>2)+2,Color,-1);
    PrintXY(Img,"5",W3+W9+2,(H>>2)+2,Color,-1);
    PrintXY(Img,"6",W-W3-W9+2,(H>>2)+2,Color,-1);
    PrintXY(Img,"7",W3+2,(H>>1)+2,Color,-1);
    PrintXY(Img,"8",W3+W9+2,(H>>1)+2,Color,-1);
    PrintXY(Img,"9",W-W3-W9+2,(H>>1)+2,Color,-1);
    PrintXY(Img,"*",W3+2,H-(H>>2)+2,Color,-1);
    PrintXY(Img,"0",W3+W9+2,H-(H>>2)+2,Color,-1);
    PrintXY(Img,"#",W-W3-W9+2,H-(H>>2)+2,Color,-1);
  }
}

/** DrawKeyboard() *******************************************/
/** Draw virtual keyboard in a given image. Key modifiers   **/
/** and the key code passed in CurKey are highlighted.      **/
/*************************************************************/
void DrawKeyboard(Image *Img,unsigned int CurKey)
{
  unsigned int X,Y,J,I,K,L;
  char S[2];
  pixel *P;

  /* Keyboard in the right-bottom corner by default */
  X=Img->W-KEYSTEP*XKEYS-8;
  Y=Img->H-KEYSTEP*YKEYS-8;

  /* Draw modifiers */
  if(CurKey&CON_MODES)
  {
    J=X;
    if(CurKey&CON_SHIFT)   { PrintXY(Img,"SHIFT",J,Y-8,CLR_ACTIVEB,-1);J+=48; }
    if(CurKey&CON_CONTROL) { PrintXY(Img,"CTRL",J,Y-8,CLR_ACTIVEB,-1);J+=40; }
    if(CurKey&CON_ALT)     { PrintXY(Img,"ALT",J,Y-8,CLR_ACTIVEB,-1);J+=32; }
  }

  /* Draw keys */
  for(I=J=0,S[1]='\0';KBDLines[I];++I,Y+=KEYSTEP,X+=KBDOffsets[I]-J*KEYSTEP)
    for(J=0;KBDLines[I][J];++J,X+=KEYSTEP)
    {
      /* Draw key frame */
      P = (pixel *)Img->Data
        + Img->L*(Y+(KEYSTEP-KEYSIZE)/2)
        + X+(KEYSTEP-KEYSIZE)/2;

      /* Highlight current key */
      if(KBDKeys[I][J]==(CurKey&CON_KEYCODE))
      {
        for(K=1;K<KEYSIZE-1;++K) P[K]=CLR_ACTIVEB;
        for(K=1,P+=Img->L;K<KEYSIZE-1;++K,P+=Img->L)
          for(L=0;L<KEYSIZE;++L) P[L]=CLR_ACTIVEB;
        for(K=1;K<KEYSIZE-1;++K) P[K]=CLR_ACTIVEB;
        K=CLR_ACTIVEF;
      }
      else
      {
        for(K=1;K<KEYSIZE-1;++K) P[K]=CLR_NORMALF;
        for(K=1,P+=Img->L;K<KEYSIZE-1;++K,P+=Img->L)
        {
          for(L=1;L<KEYSIZE-1;++L) P[L]=CLR_NORMALB;
          P[0]=P[KEYSIZE-1]=CLR_NORMALF;
        }
        for(K=1;K<KEYSIZE-1;++K) P[K]=CLR_NORMALF;
        K=CLR_NORMALF;
      }

      /* Draw key label */
      S[0]=KBDLines[I][J];
      PrintXY(Img,S,X+(KEYSTEP-8)/2,Y+(KEYSTEP-8)/2,K,-1);
    }
}

#undef CLR_NORMALF
#undef CLR_NORMALB
#undef CLR_ACTIVEF
#undef CLR_ACTIVEB

#endif /* BPP32||BPP24||BPP16||BPP8 */
