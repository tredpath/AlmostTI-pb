/** AlmostTI: portable TI calcs emulator *********************/
/**                                                         **/
/**                         Unix.c                          **/
/**                                                         **/
/** This file contains Unix-dependent subroutines and       **/
/** drivers. It includes screen drivers via Display.h.      **/
/**                                                         **/
/** Copyright (C) Marat Fayzullin 2009                      **/
/**     You are not allowed to distribute this software     **/
/**     commercially. Please, notify me, if you make any    **/
/**     changes to this file.                               **/
/*************************************************************/
#include "../TI85.h"
#include "../../EMULib/Console.h"
#include "../../EMULib/EMULib.h"
#include "../../EMULib/Sound.h"

#include <sys/keycodes.h>
#include <bps/dialog.h>
#include <bps/bps.h>
#include <bps/event.h>
#include <bps/screen.h>

#include <string>
#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include <png.h>

#define WIDTH       600                   						/* Buffer width      */
#define HEIGHT      1024                  						/* Buffer height     */
#define TIWIDTH		(TI83_FAMILY?96:128)						/* TI display width  */
#define TIHEIGHT	64					  						/* TI display height */
#define XOFFSET     (WIDTH-(TIWIDTH)*(TI83_FAMILY?5:4))/2		/* Display offset    */
#define YOFFSET     63                    						/* Display offset    */
#define PORTRAIT

/* Combination of EFF_* bits */
int UseEffects  = 0;//EFF_SCALE|EFF_SAVECPU|EFF_MITSHM|EFF_VARBPP|EFF_SYNC;

int InMenu;                /* 1: In MenuTI85(), ignore keys  */
int UseStatic   = 1;       /* 1: Use static palette          */
int UseZoom     = 1;       /* Zoom factor (1=no zoom)        */
int UseSound    = 0;//22050;   /* Audio sampling frequency (Hz)  */
int SyncFreq    = 0;//60;      /* Sync frequency (0=sync off)    */
int FastForward;           /* Fast-forwarded UPeriod backup  */
int SndSwitch;             /* Mask of enabled sound channels */
int SndVolume;             /* Master volume for audio        */
byte KeyReady;             /* 1: Key has been pressed        */
Image OutImage;            /* Unix display image buffer      */
Image ScrImage;            /* TI85 display image buffer      */
unsigned int XPal[2];      /* Referenced from Common.h       */
unsigned int Black,White;
byte lastKeyTouch;

const char *Title = "AlmostTI-QNX 1.0"; /* Program version  */

void HandleKeys(unsigned int Key);
void HandleMouse(int X, int Y, int State);
int LoadPNG(Image *Img,const char *FileName);

/** TouchMap[] ***********************************************/
/** This map translates on-screen rectangles into keys.     **/
/*************************************************************/
struct
{
	byte KeyCode;
	int X,Y,W,H;
} TouchMap[] =
{
	{ KBD_RIGHT,  504,482,36,64 },
	{ KBD_LEFT,   364,482,35,64 },
	{ KBD_UP,     405,448,96,35 },
	{ KBD_DOWN,   405,544,96,36 },

	{ KBD_F5,     464,378,81,22 },
	{ KBD_F4,     362,378,81,22 },
	{ KBD_F3,     259,378,81,22 },
	{ KBD_F2,     156,378,81,22 },
	{ KBD_F1,     54,378,81,22 },

	{ KBD_CLEAR,  464,599,81,36 },
	{ KBD_POWER,  464,655,81,36 },
	{ KBD_DIV,    464,711,81,36 },
	{ KBD_MUL,    464,768,81,36 },
	{ KBD_MINUS,  464,824,81,36 },
	{ KBD_PLUS,   464,880,81,36 },
	{ KBD_ENTER,  464,937,81,47 },

	{ KBD_CUSTOM, 362,599,81,36 },
	{ KBD_TAN,    362,655,81,36 },
	{ KBD_RPARENT,362,711,81,36 },
	{ KBD_9,      362,768,81,36 },
	{ KBD_6,      362,824,81,36 },
	{ KBD_3,      362,880,81,36 },
	{ KBD_SIGN,   362,937,81,43 },

	{ KBD_MORE,   259,486,81,36 },
	{ KBD_DEL,    259,543,81,36 },
	{ KBD_PRGM,   259,599,81,36 },
	{ KBD_COS,    259,655,81,36 },
	{ KBD_LPARENT,259,711,81,36 },
	{ KBD_8,      259,768,81,36 },
	{ KBD_5,      259,824,81,36 },
	{ KBD_2,      259,880,81,36 },
	{ KBD_DOT,    259,937,81,43 },

	{ KBD_EXIT,   156,486,81,36 },
	{ KBD_XVAR,   156,543,81,36 },
	{ KBD_STAT,   156,599,81,36 },
	{ KBD_SIN,    156,655,81,36 },
	{ KBD_EE,     156,711,81,36 },
	{ KBD_7,      156,768,81,36 },
	{ KBD_4,      156,824,81,36 },
	{ KBD_1,      156,880,81,36 },
	{ KBD_0,      156,937,81,43 },

	{ KBD_2ND,    54,486,81,36 },
	{ KBD_ALPHA,  54,543,81,36 },
	{ KBD_GRAPH,  54,599,81,36 },
	{ KBD_LOG,    54,655,81,36 },
	{ KBD_LN,     54,711,81,36 },
	{ KBD_SQR,    54,768,81,36 },
	{ KBD_COMMA,  54,824,81,36 },
	{ KBD_STO,    54,880,81,36 },
	{ KBD_ON,     54,937,81,36 },

	{ 0,0,0,0,0 }
};

/** InitMachine() ********************************************/
/** Allocate resources needed by machine-dependent code.    **/
/*************************************************************/
int InitMachine(void)
{
  int J;

  /* Initialize variables */
  UseZoom       = UseZoom<1? 1:UseZoom>5? 5:UseZoom;
  InMenu        = 0;
  FastForward   = 0;
  OutImage.Data = 0;
  KeyReady      = 0;

  /* Initialize system resources */
  if (!InitQNX(Title,UseZoom*WIDTH,UseZoom*HEIGHT))
	  return 0;

  /* Set visual effects */
  BPSSetEffects(UseEffects);

  /* Create main image buffer */
  if(!NewImage(&OutImage,WIDTH,HEIGHT)) { TrashQNX(); return(0); }
  ClearImage(&OutImage,PIXEL(0,0,0));
  CropImage(&ScrImage,&OutImage,XOFFSET,YOFFSET,(TI83_FAMILY?5:4)*TIWIDTH,4*TIHEIGHT);

  /* Initialize video to main image */
  SetVideo(&OutImage,0,0,WIDTH,HEIGHT);

  /* Set colors */
  XPal[0] = White = PIXEL(255,255,255);
  XPal[1] = Black = PIXEL(0,0,0);

  /* Attach keyboard handler */
  SetKeyHandler(HandleKeys);

  /* Attach mouse handler */
  SetMouseHandler(HandleMouse);

  /* Initialize sound */
  InitSound(UseSound,150);
  SndSwitch=(1<<4)-1;
  SndVolume=255/4;
  SetChannels(SndVolume,SndSwitch);

  /* Initialize sync timer if needed */
  if((SyncFreq>0)&&!SetSyncTimer(SyncFreq*UPeriod/100)) SyncFreq=0;

  /* Done */
  return(1);
}

/** TrashMachine() *******************************************/
/** Deallocate all resources taken by InitMachine().        **/
/*************************************************************/
void TrashMachine(void)
{
  FreeImage(&OutImage);
  TrashAudio();
  TrashQNX();
}

/** SetColor() ***********************************************/
/** Allocate new color.                                     **/
/*************************************************************/
void SetColor(byte N,byte R,byte G,byte B)
{
  /* Set requested color */
  XPal[N&1]=PIXEL(R,G,B);
}

/** PutImage() ***********************************************/
/** Put an image on the screen.                             **/
/*************************************************************/
void PutImage(void)
{
  /* Show rendered screen buffer */
  ShowVideo();
}

/** Keypad() *************************************************/
/** Poll the keyboard.                                      **/ 
/*************************************************************/
byte Keypad(void)
{
  BPSProcessEvents();
  return(IS_KBD(KBD_ON));
}

/** ShowBackdrop() *******************************************/
/** Show backdrop image with calculator faceplate.          **/
/*************************************************************/
int ShowBackdrop(const char *FileName)
{
	char *P;
	int J;
	char* imgDir = "app/native/Backdrops";

	/* Load background image */
	if(!(P = (char*)malloc((imgDir ? strlen(imgDir) : 8) + 16)))
		J=0;
	else
	{
		strcpy(P,imgDir? imgDir:".");
		strcat(P,"/");
		strcat(P,FileName);
		J=LoadPNG(&OutImage,P);
		free(P);
	}

	return(J);
}

/** HandleKeys() *********************************************/
/** Key handler.                                            **/
/*************************************************************/
void HandleKeys(unsigned int Key)
{
  if(InMenu||CPU.Trace) return;

  if(Key&CON_RELEASE)
    switch(Key&CON_KEYCODE)
    {
    case KEYCODE_F9:
	case KEYCODE_PG_UP:
        if(FastForward)
        {
          BPSSetEffects(UseEffects);
          UPeriod=FastForward;
          FastForward=0;
        }
        break;

	case KEYCODE_Q:
	case KEYCODE_CAPITAL_Q:          KBD_RES(KBD_ON);KeyReady=1;break;
	case KEYCODE_F1:		         KBD_RES(KBD_F1);KeyReady=1;break;
	case KEYCODE_F2:		         KBD_RES(KBD_F2);KeyReady=1;break;
	case KEYCODE_F3:		         KBD_RES(KBD_F3);KeyReady=1;break;
	case KEYCODE_F4:		         KBD_RES(KBD_F4);KeyReady=1;break;
	case KEYCODE_F5:		         KBD_RES(KBD_F5);KeyReady=1;break;
	case KEYCODE_RETURN:
	case KEYCODE_KP_ENTER: 		     KBD_RES(KBD_ENTER);KeyReady=1;break;
	case KEYCODE_LEFT_SHIFT:
	case KEYCODE_RIGHT_SHIFT: 		 KBD_RES(KBD_2ND);KeyReady=1;break;
	case KEYCODE_ESCAPE:		     KBD_RES(KBD_EXIT);KeyReady=1;break;
	case KEYCODE_LEFT:		         KBD_RES(KBD_LEFT);KeyReady=1;break;
	case KEYCODE_RIGHT:			     KBD_RES(KBD_RIGHT);KeyReady=1;break;
	case KEYCODE_UP:				 KBD_RES(KBD_UP);KeyReady=1;break;
	case KEYCODE_DOWN:				 KBD_RES(KBD_DOWN);KeyReady=1;break;
	case KEYCODE_GREATER_THAN:		 KBD_RES(KBD_STO);KeyReady=1;break;
	case KEYCODE_DELETE:
	case KEYCODE_BACKSPACE:			 KBD_RES(KBD_DEL);KeyReady=1;break;
	case KEYCODE_HOME:				 KBD_RES(KBD_CLEAR);KeyReady=1;break;
	case KEYCODE_LEFT_ALT:
	case KEYCODE_RIGHT_ALT:			 KBD_RES(KBD_ALPHA);KeyReady=1;break;
	default:
        Key&=CON_KEYCODE;
        if((Key>=' ')&&(Key<0x80)) { KBD_RES(Key);KeyReady=1; }
        break;
    }
  else
    switch(Key&CON_KEYCODE)
    {
    case KEYCODE_F6:				 LoadSTA("DEFAULT.STA");break;
    case KEYCODE_F7:        		 SaveSTA("DEFAULT.STA");break;
    case KEYCODE_F9:
	case KEYCODE_PG_UP:
        if(!FastForward)
        {
          FastForward=UPeriod;
          UPeriod=10;
        }
        break;
	case KEYCODE_Q:
	case KEYCODE_CAPITAL_Q:			 KBD_SET(KBD_ON);KeyReady=1;break;
	case KEYCODE_F11:				 ResetTI85(Mode);break;
	case KEYCODE_F12:				 ExitNow=1;break;
	case KEYCODE_F1:				 KBD_SET(KBD_F1);KeyReady=1;break;
	case KEYCODE_F2:				 KBD_SET(KBD_F2);KeyReady=1;break;
	case KEYCODE_F3:				 KBD_SET(KBD_F3);KeyReady=1;break;
	case KEYCODE_F4:				 KBD_SET(KBD_F4);KeyReady=1;break;
	case KEYCODE_F5:				 KBD_SET(KBD_F5);KeyReady=1;break;
	case KEYCODE_RETURN:
	case KEYCODE_KP_ENTER:			 KBD_SET(KBD_ENTER);KeyReady=1;break;
	case KEYCODE_LEFT_SHIFT:
	case KEYCODE_RIGHT_SHIFT:		 KBD_SET(KBD_2ND);KeyReady=1;break;
	case KEYCODE_ESCAPE:			 KBD_SET(KBD_EXIT);KeyReady=1;break;
	case KEYCODE_LEFT:				 KBD_SET(KBD_LEFT);KeyReady=1;break;
	case KEYCODE_RIGHT:				 KBD_SET(KBD_RIGHT);KeyReady=1;break;
	case KEYCODE_UP:				 KBD_SET(KBD_UP);KeyReady=1;break;
	case KEYCODE_DOWN:				 KBD_SET(KBD_DOWN);KeyReady=1;break;
	case KEYCODE_GREATER_THAN:		 KBD_SET(KBD_STO);KeyReady=1;break;
	case KEYCODE_DELETE:
	case KEYCODE_BACKSPACE:			 KBD_SET(KBD_DEL);KeyReady=1;break;
	case KEYCODE_HOME:				 KBD_SET(KBD_CLEAR);KeyReady=1;break;
	case KEYCODE_LEFT_ALT:
	case KEYCODE_RIGHT_ALT:			 KBD_SET(KBD_ALPHA);KeyReady=1;break;
	default:
        Key&=CON_KEYCODE;
        if((Key>=' ')&&(Key<0x80)) { KBD_SET(Key);KeyReady=1; }
        break; 
    }
}

/** HandleMouse() *********************************************/
/** Mouse click/unclick handler.                             **/
/*************************************************************/
void HandleMouse(int X, int Y, int State)
{
	if (lastKeyTouch)
	{
		KBD_RES(lastKeyTouch);
		return;
	}
    int J;
    int Flags = State ? 0:CON_RELEASE;
    for(J=0;TouchMap[J].W;++J)
      if((X>=TouchMap[J].X)&&(Y>=TouchMap[J].Y))
        if((X<TouchMap[J].X+TouchMap[J].W)&&(Y<TouchMap[J].Y+TouchMap[J].H))
        {
          if (State)
          {
            KBD_SET(TouchMap[J].KeyCode);
          }
          else
          {
            KBD_RES(TouchMap[J].KeyCode);
          }
          break;
        }
}

/** Common.h *************************************************/
/** Common display drivers.                                 **/
/*************************************************************/
#include "../Common.h"


