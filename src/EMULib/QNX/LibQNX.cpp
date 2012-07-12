/** EMULib Emulation Library *********************************/
/**                                                         **/
/**                        LibUnix.c                        **/
/**                                                         **/
/** This file contains Unix-dependent implementation        **/
/** parts of the emulation library.                         **/
/**                                                         **/
/** Copyright (C) Marat Fayzullin 1996-2009                 **/
/**     You are not allowed to distribute this software     **/
/**     commercially. Please, notify me, if you make any    **/
/**     changes to this file.                               **/
/*************************************************************/
#include "../EMULib.h"
#include "../LibARM.h"
#include "../Console.h"
#include "../Sound.h"
#include "../Touch.h"
#include "../../ATI85/TI85.h"
#include "../../ATI85/QNX/QNXMenu.h"

#include <sys/keycodes.h>
#include <bps/screen.h>
#include <bps/bps.h>
#include <bps/navigator.h>
#include <bps/event.h>
#include <bps/dialog.h>

#include <ctype.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <fcntl.h>
#include <time.h>
#include <stdio.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>

#define FPS_COLOR PIXEL(255,0,255)

extern int MasterSwitch; /* Switches to turn channels on/off */
extern int MasterVolume; /* Master volume                    */

static volatile int TimerReady = 0;   /* 1: Sync timer ready */
static volatile unsigned int JoyState = 0; /* Joystick state */
static volatile unsigned int LastKey  = 0; /* Last key prsd  */
static volatile unsigned int KeyModes = 0; /* SHIFT/CTRL/ALT */

static int Effects    = EFF_SCALE|EFF_SAVECPU; /* EFF_* bits */
static int TimerON    = 0; /* 1: sync timer is running       */
int rect[4] = { 0, 0 };
static screen_context_t ctxt;
static screen_window_t window;
static Image OutImg;       /* Scaled output image buffer     */
static const char *AppTitle; /* Current window title         */
static int XSize,YSize;    /* Current window dimensions      */

static int FrameCount;      /* Frame counter for EFF_SHOWFPS */
static int FrameRate;       /* Last frame rate value         */
static struct timeval TimeStamp; /* Last timestamp           */

int  ARGC   = 0;
char **ARGV = 0;

/** TimerHandler() *******************************************/
/** The main timer handler used by SetSyncTimer().          **/
/*************************************************************/
static void TimerHandler(int Arg)
{
  /* Mark sync timer as "ready" */
  TimerReady=1;
  /* Repeat signal next time */
  signal(Arg,TimerHandler);
}

/** InitUnix() ***********************************************/
/** Initialize Unix/X11 resources and set initial window    **/
/** title and dimensions.                                   **/
/*************************************************************/
int InitQNX(const char *Title,int Width,int Height)
{
  /* Initialize variables */
  AppTitle    = Title;
  XSize       = Width;
  YSize       = Height;
  TimerON     = 0;
  TimerReady  = 0;
  JoyState    = 0;
  LastKey     = 0;
  KeyModes    = 0;
  FrameCount  = 0;
  FrameRate   = 0;

  /* Get initial timestamp */
  gettimeofday(&TimeStamp,0);

	screen_create_context(&ctxt, SCREEN_APPLICATION_CONTEXT);

	if (BPS_SUCCESS != screen_request_events(ctxt))
	{
		screen_destroy_context(ctxt);
		return 0;
	}

	screen_create_window(&window, ctxt);

	int usage = SCREEN_USAGE_NATIVE;
	screen_set_window_property_iv(window, SCREEN_PROPERTY_USAGE, &usage);

	screen_create_window_buffers(window, 1);
	screen_get_window_property_iv(window, SCREEN_PROPERTY_BUFFER_SIZE, rect+2);

  /* Done */
  return(1);
}

/** TrashUnix() **********************************************/
/** Free resources allocated in InitUnix()                  **/
/*************************************************************/
void TrashQNX(void)
{
  /* Remove sync timer */
  SetSyncTimer(0);
  /* Shut down audio */
  TrashAudio();
  /* Free output image buffer */
  FreeImage(&OutImg);
}

/** ShowVideo() **********************************************/
/** Show "active" image at the actual screen or window.     **/
/*************************************************************/
int ShowVideo(void)
{
  Image *Output;
  int SX,SY;

  /* Must have active video image, X11 display */
  if(!VideoImg||!VideoImg->Data) return(0);

  /* Allocate image buffer if none */
  if(!OutImg.Data&&!NewImage(&OutImg,XSize,YSize)) return(0);

  /* If not scaling or post-processing image, avoid extra work */
  if(!(Effects&(EFF_SOFTEN|EFF_SCALE|EFF_TVLINES)))
  {
		screen_buffer_t screen_buf[1];
		screen_get_window_property_pv(window, SCREEN_PROPERTY_RENDER_BUFFERS, (void **)screen_buf);

		int bg[] = { SCREEN_BLIT_END };
		screen_blit(ctxt, screen_buf[0], VideoImg->pbuf, bg);
		screen_post_window(window, screen_buf[0], 1, rect, 0);
		return 1;
  }

  /* By default, we will be showing OutImg */
  Output  = &OutImg;
  SX      = 0;
  SY      = 0;

  if(Effects&EFF_SOFTEN)
  {
    /* Apply softening */
    SoftenImage(&OutImg,VideoImg,VideoX,VideoY,VideoW,VideoH);
    /* Apply TV scanlines, if needed */
    if(Effects&EFF_TVLINES)
      TelevizeImage(&OutImg,0,0,OutImg.W,OutImg.H);
  }
  else if(Effects&EFF_TVLINES)
  {
    if(Effects&EFF_SCALE)
    {
      /* Scale VideoImg into OutImg */
      ScaleImage(&OutImg,VideoImg,VideoX,VideoY,VideoW,VideoH);
      /* Apply TV scanlines */
      TelevizeImage(&OutImg,0,0,OutImg.W,OutImg.H);
    }
    else
    {
      /* Center VideoImg in OutImg */
      IMGCopy(&OutImg,(OutImg.W-VideoW)>>1,(OutImg.H-VideoH)>>1,VideoImg,VideoX,VideoY,VideoW,VideoH,-1);
      /* Apply TV scanlines */
      TelevizeImage(&OutImg,(OutImg.W-VideoW)>>1,(OutImg.H-VideoH)>>1,VideoW,VideoH);
    }
  }
  else if((OutImg.W==VideoW)&&(OutImg.H==VideoH))
  {
    /* Use VideoImg directly */
    Output = VideoImg;
    SX     = VideoX;
    SY     = VideoY;
  }
  else if(Effects&EFF_SCALE)
  {
    /* Scale VideoImg to OutImg */
    ScaleImage(&OutImg,VideoImg,VideoX,VideoY,VideoW,VideoH);
  }
  else if((OutImg.W<=VideoW)&&(OutImg.H<=VideoH))
  {
    /* Use VideoImg directly */
    Output = VideoImg;
    SX     = VideoX+((VideoW-OutImg.W)>>1);
    SY     = VideoY+((VideoH-OutImg.H)>>1);
  }
  else
  {
    /* Center VideoImg in OutImg */
    IMGCopy(&OutImg,(OutImg.W-VideoW)>>1,(OutImg.H-VideoH)>>1,VideoImg,VideoX,VideoY,VideoW,VideoH,-1);
  }

  /* Show framerate if requested */
  if((Effects&EFF_SHOWFPS)&&(FrameRate>0))
  {
    char S[8];
    sprintf(S,"%dfps",FrameRate);
    PrintXY(
      &OutImg,S,
      ((OutImg.W-VideoW)>>1)+8,((OutImg.H-VideoH)>>1)+8,
      FPS_COLOR,-1
    );
  }

  /* Wait for sync timer if requested */
  if(Effects&EFF_SYNC) WaitSyncTimer();

	screen_buffer_t screen_buf[1];
	screen_get_window_property_pv(window, SCREEN_PROPERTY_RENDER_BUFFERS, (void **)screen_buf);

	int bg[] = { SCREEN_BLIT_END };
	screen_blit(ctxt, screen_buf[0], Output->pbuf, bg);
	screen_post_window(window, screen_buf[0], 1, rect, 0);
	return 1;
}

/** GetJoystick() ********************************************/
/** Get the state of joypad buttons (1="pressed"). Refer to **/
/** the BTN_* #defines for the button mappings.             **/
/*************************************************************/
unsigned int GetJoystick(void)
{
  /* Count framerate */
  if((Effects&EFF_SHOWFPS)&&(++FrameCount>=300))
  {
    struct timeval NewTS;
    int Time;

    gettimeofday(&NewTS,0);
    Time       = (NewTS.tv_sec-TimeStamp.tv_sec)*1000
               + (NewTS.tv_usec-TimeStamp.tv_usec)/1000;
    FrameRate  = 1000*FrameCount/(Time>0? Time:1); 
    TimeStamp  = NewTS;
    FrameCount = 0;
    FrameRate  = FrameRate>999? 999:FrameRate;
  }

  /* Process any pending events */
  BPSProcessEvents();

  /* Return current joystick state */
  return(JoyState|KeyModes);
}

/** GetMouse() ***********************************************/
/** Get mouse position and button states in the following   **/
/** format: RMB.LMB.Y[29-16].X[15-0].                       **/
/*************************************************************/
unsigned int GetMouse(void)
{
	return 1;
}

/** GetKey() *************************************************/
/** Get currently pressed key or 0 if none pressed. Returns **/
/** CON_* definitions for arrows and special keys.          **/
/*************************************************************/
unsigned int GetKey(void)
{
  unsigned int J;

  BPSProcessEvents();
  J=LastKey;
  LastKey=0;
  return(J);
}

/** WaitKey() ************************************************/
/** Wait for a key to be pressed. Returns CON_* definitions **/
/** for arrows and special keys.                            **/
/*************************************************************/
unsigned int WaitKey(void)
{
  unsigned int J;

  /* Swallow current keypress */
  GetKey();
  /* Wait in 100ms increments for a new keypress */
  while(!(J=GetKey())&&VideoImg) usleep(100000);
  /* Return key code */
  return(J);
}

/** WaitKeyOrMouse() *****************************************/
/** Wait for a key or a mouse button to be pressed. Returns **/
/** the same result as GetMouse(). If no mouse buttons      **/
/** reported to be pressed, do GetKey() to fetch a key.     **/
/*************************************************************/
unsigned int WaitKeyOrMouse(void)
{
  unsigned int J;

  /* Swallow current keypress */
  GetKey();
  /* Make sure mouse keys are not pressed */
  while(GetMouse()&0xC0000000) usleep(100000);
  /* Wait in 100ms increments for a key or mouse click */
  while(!(J=GetKey())&&!(GetMouse()&0xC0000000)&&VideoImg) usleep(100000);
  /* Place key back into the buffer and return mouse state */
  LastKey=J;
  return(GetMouse());
}

/** WaitSyncTimer() ******************************************/
/** Wait for the timer to become ready.                     **/
/*************************************************************/
void WaitSyncTimer(void)
{
  /* Wait in 1ms increments until timer becomes ready */
  while(!TimerReady&&TimerON&&VideoImg) usleep(1000);
  /* Warn of missed timer events */
  if((TimerReady>1)&&(Effects&EFF_VERBOSE))
    printf("WaitSyncTimer(): Missed %d timer events.\n",TimerReady-1);
  /* Reset timer */
  TimerReady=0;
}

/** SyncTimerReady() *****************************************/
/** Return 1 if sync timer ready, 0 otherwise.              **/
/*************************************************************/
int SyncTimerReady(void)
{
  /* Return whether timer is ready or not */
  return(TimerReady||!TimerON||!VideoImg);
}

/** SetSyncTimer() *******************************************/
/** Set synchronization timer to a given frequency in Hz.   **/
/*************************************************************/
int SetSyncTimer(int Hz)
{
  struct itimerval TimerValue;

  /* Compute and set timer period */
  TimerValue.it_interval.tv_sec  =
  TimerValue.it_value.tv_sec     = 0;
  TimerValue.it_interval.tv_usec =
  TimerValue.it_value.tv_usec    = Hz? 1000000L/Hz:0;

  /* Set timer */
  if(setitimer(ITIMER_REAL,&TimerValue,NULL)) return(0);

  /* Set timer signal */
  signal(SIGALRM,Hz? TimerHandler:SIG_DFL);

  /* Done */
  TimerON=Hz;
  return(1);
}

/** ChangeDir() **********************************************/
/** This function is a wrapper for chdir().                 **/
/*************************************************************/
int ChangeDir(const char *Name) { return(chdir(Name)); }

/** NewImage() ***********************************************/
/** Create a new image of the given size. Returns pointer   **/
/** to the image data on success, 0 on failure.             **/
/*************************************************************/
pixel *NewImage(Image *Img,int Width,int Height)
{
	/* Set data fields to their defaults */
	Img->Data    = 0;
	Img->W       = 0;
	Img->H       = 0;
	Img->L       = 0;
	Img->D       = 0;
	Img->Cropped = 0;

    screen_create_pixmap(&Img->img, ctxt);

	int format = SCREEN_FORMAT_RGBA8888;
	screen_set_pixmap_property_iv(Img->img, SCREEN_PROPERTY_FORMAT, &format);

	int usage = SCREEN_USAGE_WRITE | SCREEN_USAGE_NATIVE;
	screen_set_pixmap_property_iv(Img->img, SCREEN_PROPERTY_USAGE, &usage);

	int size[2] = { 600, 1024 };
	screen_set_pixmap_property_iv(Img->img, SCREEN_PROPERTY_BUFFER_SIZE, size);

	screen_create_pixmap_buffer(Img->img);
	screen_get_pixmap_property_pv(Img->img, SCREEN_PROPERTY_RENDER_BUFFERS, (void **)&Img->pbuf);

	unsigned char *ptr = NULL;
	screen_get_buffer_property_pv(Img->pbuf, SCREEN_PROPERTY_POINTER, (void **)&ptr);
	Img->Data = (pixel*)ptr;

	screen_get_buffer_property_iv(Img->pbuf, SCREEN_PROPERTY_STRIDE, &Img->stride);

	Img->W     = Width;
	Img->H     = Height;
	Img->L     = Img->stride * 8.0 / 32.0;
	Img->D     = 32;

	return(Img->Data);
}

/** FreeImage() **********************************************/
/** Free previously allocated image.                        **/
/*************************************************************/
void FreeImage(Image *Img)
{
	if (Img->img) screen_destroy_pixmap(Img->img);
	Img->Data = 0;
	Img->W = 0;
	Img->H = 0;
	Img->L = 0;
	Img->D = 0;
}

/** CropImage() **********************************************/
/** Create a subimage Dst of the image Src. Returns Dst.    **/
/*************************************************************/
Image *CropImage(Image *Dst,const Image *Src,int X,int Y,int W,int H)
{
  Dst->Data    = (pixel *)(Src->Data+Src->L*Y+X);
  Dst->Cropped = 1;
  Dst->W       = W;
  Dst->H       = H;
  Dst->L       = Src->L;
  Dst->D       = Src->D;
  return(Dst);
}

int BPSResizeVideo(int Width, int Height)
{
	int SX, SY, NX, NY, GX, GY;
	Image *img;

	XSize = Width;
	YSize = Height;

	GX = 600;
	GY = 1024;

	if (Width > 0)
	{
		SX = VideoImg && (VideoW > 0) ? VideoW : Width;
		NX = (Width + SX - 1) / SX;
	}
	else
	{
		SX = VideoImg && (VideoW > 0) && (VideoW < GX) ? VideoW : GX;
		for (NX = 1; SX * (NX + 1) <= GX; ++NX);
	}

	if (Height > 0)
	{
		SY = VideoImg && (VideoH > 0) ? VideoH : Height;
		NY = (Height + SY - 1) / SY;
	}
	else
	{
		SY = VideoImg && (VideoH > 0) && (VideoH < GY) ? VideoH : GY;
		for (NY = 1; SY * (NY + 1) <= GY; ++NY);
	}

	if (NX > NY)	NX = NY;
	else			NY = NX;
	if (Width <= 0)	Width = SX * NX;
	if (Height <= 0) Height = SY * NY;

	CropImage(&OutImg, VideoImg, (VideoImg->W - Width) >> 1, (VideoImg->H - Height) >> 1, Width, Height);

	return 1;
}

/** SetVideo() ***********************************************/
/** Set part of the image as "active" for display.          **/
/*************************************************************/
void SetVideo(Image *Img,int X,int Y,int W,int H)
{
  /* Call default SetVideo() function */
  GenericSetVideo(Img,X,Y,W,H);
}

/** BPSSetEffects() ******************************************/
/** Set visual effects applied to video in ShowVideo().     **/
/*************************************************************/
void BPSSetEffects(int NewEffects)
{
  /* Set new effects */
  Effects=NewEffects;
}

extern Image OutImage;
extern Image ScrImage;

void handleNavigatorEvent(bps_event_t* event)
{
	bps_event_t* activation_event = NULL;
	unsigned int code = bps_event_get_code(event);
	if (code == NAVIGATOR_SWIPE_DOWN)
	{
		int T;
		int M = AutoLoadRom(T, 1);
		if (M >= 0 && M != (ATI_MODEL & Mode))
		{
			TrashTI85();
			Mode=(Mode&~ATI_MODEL)|M;
			free(RAMFile);
			RAMFile = (char*)malloc(sizeof(char)*strlen(Config[T].RAMFile));
			strcpy(RAMFile, Config[T].RAMFile);
			FreeImage(&OutImg);
			NewImage(&OutImage,600,1024);
			ClearImage(&OutImage,PIXEL(0,0,0));
			CropImage(&ScrImage,&OutImage,(600-((TI83_FAMILY?96:128))*(TI83_FAMILY?5:4))/2,63,4*(TI83_FAMILY?96:128),4*64);
			SetVideo(&OutImage,0,0,600,1024);
			StartTI85();
		}
	}
	else if (code == NAVIGATOR_WINDOW_INACTIVE)
	{
		//int J;
		//if (Effects & EFF_SAVECPU)
		//{
		//	J = MasterVolume;
		//	SetChannels(MasterVolume, 0);
		//	PauseAudio(1);
		//}
		for (;;)
		{
			bps_get_event(&activation_event, -1);
			if (event && (bps_event_get_code(activation_event) == NAVIGATOR_WINDOW_ACTIVE))
				break;
		}
		//if (Effects & EFF_SAVECPU)
		//{
		//	PauseAudio(0);
		//	SetChannels(MasterVolume, J);
		//}
	}
	else if (code == NAVIGATOR_EXIT)
	{
		ExitNow = 1;
	}
}

void handleScreenEvent(bps_event_t* event)
{
	screen_event_t screen_event = screen_event_get_event(event);
	int screen_val;
	screen_get_event_property_iv(screen_event, SCREEN_PROPERTY_TYPE, &screen_val);
	int pair[2];
	screen_get_event_property_iv(screen_event, SCREEN_PROPERTY_SOURCE_POSITION, pair);
	int x = pair[0];
	int y = pair[1];
	int J, type;
	switch(screen_val)
	{
	case SCREEN_EVENT_KEYBOARD:
		screen_get_event_property_iv(screen_event, SCREEN_PROPERTY_KEY_FLAGS, &type);
		screen_get_event_property_iv(screen_event, SCREEN_PROPERTY_KEY_SYM, &J);
		if(type & KEY_DOWN)
		{
			/* Process ASCII keys */
			if((J>=' ')&&(J<0x7F)) LastKey=toupper(J);

			/* Special keys pressed... */
			switch(J)
			{
			case KEYCODE_LEFT:         JoyState|=BTN_LEFT;LastKey=CON_LEFT;break;
			case KEYCODE_RIGHT:        JoyState|=BTN_RIGHT;LastKey=CON_RIGHT;break;
			case KEYCODE_UP:           JoyState|=BTN_UP;LastKey=CON_UP;break;
			case KEYCODE_DOWN:         JoyState|=BTN_DOWN;LastKey=CON_DOWN;break;
			case KEYCODE_RIGHT_SHIFT:
			case KEYCODE_LEFT_SHIFT:   KeyModes|=CON_SHIFT;break;
			case KEYCODE_RIGHT_ALT:
			case KEYCODE_LEFT_ALT:     KeyModes|=CON_ALT;break;
			case KEYCODE_RIGHT_CTRL:
			case KEYCODE_LEFT_CTRL:    KeyModes|=CON_CONTROL;break;
			case KEYCODE_CAPS_LOCK:    KeyModes|=CON_CAPS;break;
			case KEYCODE_ESCAPE:       JoyState|=BTN_EXIT;LastKey=CON_EXIT;break;
			case KEYCODE_TAB:          JoyState|=BTN_SELECT;break;
			case KEYCODE_KP_ENTER:
			case KEYCODE_RETURN:       JoyState|=BTN_START;LastKey=CON_OK;break;
			case KEYCODE_BACKSPACE:    LastKey=8;break;

			case KEYCODE_Q:
			case KEYCODE_E:
			case KEYCODE_T:
			case KEYCODE_U:
			case KEYCODE_O:
				JoyState|=BTN_FIREL;break;
			case KEYCODE_W:
			case KEYCODE_R:
			case KEYCODE_Y:
			case KEYCODE_I:
			case KEYCODE_P:
				JoyState|=BTN_FIRER;break;
			case KEYCODE_A:
			case KEYCODE_S:
			case KEYCODE_D:
			case KEYCODE_F:
			case KEYCODE_G:
			case KEYCODE_H:
			case KEYCODE_J:
			case KEYCODE_K:
			case KEYCODE_L:
			case KEYCODE_SPACE:
				JoyState|=BTN_FIREA;break;
			case KEYCODE_Z:
			case KEYCODE_X:
			case KEYCODE_C:
			case KEYCODE_V:
			case KEYCODE_B:
			case KEYCODE_N:
			case KEYCODE_M:
				JoyState|=BTN_FIREB;break;

			case KEYCODE_PG_UP:
				if(KeyModes&CON_ALT)
				{
					/* Volume up */
					SetChannels(MasterVolume<247? MasterVolume+8:255,MasterSwitch);
					/* Key swallowed */
					J=0;
				}
				break;

			case KEYCODE_PG_DOWN:
				if(KeyModes&CON_ALT)
				{
					/* Volume down */
					SetChannels(MasterVolume>8? MasterVolume-8:0,MasterSwitch);
					/* Key swallowed */
					J=0;
				}
				break;
			}

			/* Call user handler */
			if(J&&KeyHandler) (*KeyHandler)(J|KeyModes);
		}
		/* If key released... */
		else
		{
			/* Special keys released... */
			switch(J)
			{
			case KEYCODE_LEFT:         JoyState&=~BTN_LEFT;break;
			case KEYCODE_RIGHT:        JoyState&=~BTN_RIGHT;break;
			case KEYCODE_UP:           JoyState&=~BTN_UP;break;
			case KEYCODE_DOWN:         JoyState&=~BTN_DOWN;break;
			case KEYCODE_RIGHT_SHIFT:
			case KEYCODE_LEFT_SHIFT:   KeyModes&=~CON_SHIFT;break;
			case KEYCODE_RIGHT_ALT:
			case KEYCODE_LEFT_ALT:     KeyModes&=~CON_ALT;break;
			case KEYCODE_RIGHT_CTRL:
			case KEYCODE_LEFT_CTRL:    KeyModes&=~CON_CONTROL;break;
			case KEYCODE_CAPS_LOCK:    KeyModes&=~CON_CAPS;break;
			case KEYCODE_ESCAPE:       JoyState&=~BTN_EXIT;break;
			case KEYCODE_TAB:          JoyState&=~BTN_SELECT;break;
			case KEYCODE_KP_ENTER:
			case KEYCODE_RETURN:       JoyState&=~BTN_START;break;

			case KEYCODE_Q:
			case KEYCODE_E:
			case KEYCODE_T:
			case KEYCODE_U:
			case KEYCODE_O:
				JoyState&=~BTN_FIREL;break;
			case KEYCODE_W:
			case KEYCODE_R:
			case KEYCODE_Y:
			case KEYCODE_I:
			case KEYCODE_P:
				JoyState&=~BTN_FIRER;break;
			case KEYCODE_A:
			case KEYCODE_S:
			case KEYCODE_D:
			case KEYCODE_F:
			case KEYCODE_G:
			case KEYCODE_H:
			case KEYCODE_J:
			case KEYCODE_K:
			case KEYCODE_L:
			case KEYCODE_SPACE:
				JoyState&=~BTN_FIREA;break;
			case KEYCODE_Z:
			case KEYCODE_X:
			case KEYCODE_C:
			case KEYCODE_V:
			case KEYCODE_B:
			case KEYCODE_N:
			case KEYCODE_M:
				JoyState&=~BTN_FIREB;break;
			}

			/* Call user handler */
			if(J&&KeyHandler) (*KeyHandler)(J|CON_RELEASE|KeyModes);
		}
		break;
	case SCREEN_EVENT_MTOUCH_TOUCH:
		(*MouseHandler)(x, y, 1);
		break;
	case SCREEN_EVENT_MTOUCH_RELEASE:
		(*MouseHandler)(x, y, 0);
		break;
	}
}

/** BPSProcessEvents() ***************************************/
/** Process BPS event messages.                             **/
/*************************************************************/
void BPSProcessEvents(void)
{
	int rc = BPS_SUCCESS;

	for (;;)
	{
		bps_event_t* event = NULL;
		rc = bps_get_event(&event, 0);

		if (event)
		{
			int domain = bps_event_get_domain(event);
			if (domain == navigator_get_domain())
				handleNavigatorEvent(event);
			else if (domain == screen_get_domain())
				handleScreenEvent(event);
		}
		else
			break;
	}
}

