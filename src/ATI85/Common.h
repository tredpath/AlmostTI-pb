/** AlmostTI: portable TI calcs emulator *********************/
/**                                                         **/
/**                        Common.h                         **/
/**                                                         **/
/** This file contains parts of the drivers which are       **/
/** common for all 256-color byte-per-pixel systems.        **/
/**                                                         **/
/** Copyright (C) Marat Fayzullin 1996-2009                 **/
/**     You are not allowed to distribute this software     **/
/**     commercially. Please, notify me, if you make any    **/   
/**     changes to this file.                               **/
/*************************************************************/
void TI85RefreshScreen(Image *Img)
{
  static const byte Widths[4] = { 10,12,16,20 };

  register byte X,Y,W,Mask,*T;
  register pixel *P;

  if(!LCD_ON)
    ClearImage(Img,XPal[0]);
  else
  {
#ifdef PORTRAIT
    for (Y = 0; Y < TIHEIGHT; Y++) {
      for (X = 0; X < TIWIDTH; X++) {
        Mask = 0x80 >> (X & 7);
        P = (pixel *)Img->Data + X*4 + Y*Img->L*4;
        T = SCREEN_BUFFER + (X / 8) + (Y * 16);
        P[0] = P[1] = P[2] = P[3] = XPal[*T & Mask ? 1:0];
      }

      /* Copy just drawn line twice */
      P = (pixel *)Img->Data + Y*Img->L*4;
      memcpy(P+Img->L, P, 4*TIWIDTH*sizeof(pixel));
      memcpy(P+2*Img->L, P, 4*TIWIDTH*sizeof(pixel));
      memcpy(P+3*Img->L, P, 4*TIWIDTH*sizeof(pixel));
    }
#else
    W    = Widths[(PORT_LCDCTRL>>3)&0x03];
    P    = (pixel *)Img->Data+Img->L*((Img->H-3*(W<<3))>>1);
    T    = SCREEN_BUFFER+W-1;
    Mask = 0x01;

    for(X=0;X<(W<<3);++X,P+=Img->L*3,T-=W*64)
    {
      /* Draw a vertical line, horizontally */
      for(Y=0;Y<64;++Y,T+=W,P+=3)
        P[0]=P[1]=P[2]=XPal[*T&Mask? 1:0];

      /* Copy just drawn line twice */
      P-=3*64;
      memcpy(P+Img->L,P,3*64*sizeof(pixel));
      memcpy(P+2*Img->L,P,3*64*sizeof(pixel));

      /* Shift mask and move screen buffer pointer */
      if(Mask<0x80) Mask<<=1; else { Mask=0x01;--T; }
    }
#endif
  }
}

void TI83RefreshScreen(Image *Img)
{
  register byte X,Y,Mask,*T;
  register pixel *P;
  register int Z;

  if(!(LCD.Status&TI83LCD_ON))
    ClearImage(Img,XPal[0]);
  else
  {
#ifdef PORTRAIT
    // Img->L = 480
    // Img->W = 384
    for (Y = 0; Y < TIHEIGHT; Y++) {
      for (X = 0; X < TIWIDTH; X++) {
        Mask = 0x80 >> (X & 7);
        P = (pixel *)Img->Data + X*5 + Y*Img->L*4;
        T = LCD.Buffer + (X / 8) + (Y * 16);
        P[0] = P[1] = P[2] = P[3] = P[4] = XPal[*T & Mask ? 1:0];
      }

      /* Copy just drawn line twice */
      P = (pixel *)Img->Data + Y*Img->L*4;
      memcpy(P+Img->L, P, 5*TIWIDTH*sizeof(pixel));
      memcpy(P+2*Img->L, P, 5*TIWIDTH*sizeof(pixel));
      memcpy(P+3*Img->L, P, 5*TIWIDTH*sizeof(pixel));
    }
#else
    P    = (pixel *)Img->Data+Img->L*((Img->H-3*96)>>1);
    T    = LCD.Buffer+11;
    Mask = 0x01;

    for(X=0;X<12*8;++X,P+=Img->L*3)
    {
      /* Draw a vertical line, horizontally */
      for(Y=0,Z=(int)LCD.Scroll<<4;Y<64;++Y,P+=3,Z=(Z+16)&0x3F0)
        P[0]=P[1]=P[2]=XPal[T[Z]&Mask? 1:0];

      /* Copy just drawn line twice */
      P-=3*64;
      memcpy(P+Img->L,P,3*64*sizeof(pixel));
      memcpy(P+2*Img->L,P,3*64*sizeof(pixel));

      /* Shift mask and move screen buffer pointer */
      if(Mask<0x80) Mask<<=1; else { Mask=0x01;--T; }
    }
#endif
  }
}

void RefreshScreen()
{
  if(TI85_FAMILY) TI85RefreshScreen(&ScrImage);
  else            TI83RefreshScreen(&ScrImage);
  PutImage();
}
