/** EMULib Emulation Library *********************************/
/**                                                         **/
/**                       Console.c                         **/
/**                                                         **/
/** This file contains platform-independent implementation  **/
/** for the EMULib-based console.                           **/
/**                                                         **/
/** Copyright (C) Marat Fayzullin 2005-2009                 **/
/**     You are not allowed to distribute this software     **/
/**     commercially. Please, notify me, if you make any    **/
/**     changes to this file.                               **/
/*************************************************************/
#if !defined(BPP32) && !defined(BPP24) && !defined(BPP16) && !defined(BPP8)
#include "ConsoleMux.h"
#else

#include "Console.h"
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>

#ifdef _MSC_VER
#include "dirent.h"
#define S_ISDIR(Attrs) ((Attrs)&S_IFDIR)
#else
#include <unistd.h>
#endif

#if defined(MSDOS) || defined(WINDOWS)
#include <direct.h>
#else
#include <dirent.h>
#endif

static pixel FG = PIXEL(255,255,255);
static pixel BG = PIXEL(0,0,0);

#ifndef DEFINE_ONCE
#define DEFINE_ONCE

#define EOLN(C)      (!(C)||((C)=='\n'))
#define TAG_SELEFILE 0x5E7EF17E

static const unsigned char NormalFont[] =
{
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, /* 0x00 */
  0x00,0xC0,0xF0,0xFC,0xFF,0xFC,0xF0,0xC0,
  0x03,0x03,0x06,0x06,0xCC,0x6C,0x38,0x18,
  0x00,0x00,0xF0,0xFE,0x82,0x82,0x82,0xFE,
  0x00,0xFE,0x82,0xBA,0x82,0xBA,0x82,0xFE,
  0x00,0x00,0x10,0x38,0x7C,0x00,0x00,0x00,
  0x00,0x00,0x7C,0x38,0x10,0x00,0x00,0x00,
  0x00,0x00,0x00,0x78,0x78,0xFC,0x00,0x00,
  0x00,0x10,0x30,0x7E,0x30,0x10,0x00,0x00, /* 0x08 */
  0x00,0x48,0x4C,0x7E,0x4C,0x48,0x00,0x00,
  0x00,0x02,0x12,0x32,0x7E,0x30,0x10,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xAA,
  0x88,0x89,0x8A,0xAA,0xDB,0x8A,0x8A,0x00,
  0x00,0x02,0x12,0x32,0x7E,0x30,0x10,0x00,
  0x00,0x08,0x24,0x42,0x42,0x24,0x08,0x00,
  0x00,0x42,0x24,0x08,0x08,0x24,0x42,0x00,
  0x00,0x64,0x4C,0x44,0x64,0x44,0x4E,0x00, /* 0x10 */
  0x00,0x64,0x4A,0x42,0x64,0x48,0x4E,0x00,
  0x00,0x6C,0x42,0x44,0x62,0x42,0x4C,0x00,
  0x00,0x6A,0x4A,0x4E,0x62,0x42,0x42,0x00,
  0x00,0x6E,0x48,0x4C,0x62,0x42,0x4C,0x00,
  0x00,0x66,0x48,0x4C,0x6A,0x4A,0x44,0x00,
  0x00,0x6E,0x4A,0x42,0x64,0x48,0x48,0x00,
  0x00,0x6E,0x4A,0x44,0x6A,0x4A,0x4E,0x00,
  0x07,0x04,0x04,0x07,0x04,0x04,0x07,0x00, /* 0x18 */
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x41,0x42,0x21,0x1B,0x06,0x03,0x0F,0x33,
  0x00,0x70,0x60,0x50,0x08,0x04,0x02,0x00,
  0x91,0x51,0x51,0x8F,0x41,0x51,0x8E,0x80,
  0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,
  0x41,0x21,0xC2,0x6C,0x30,0x60,0x78,0xE6,
  0xD1,0xC9,0xC5,0xC4,0x84,0x84,0x40,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x10,0x10,0x10,0x10,0x00,0x10,0x00,
  0x00,0x24,0x24,0x00,0x00,0x00,0x00,0x00,0x00,0x24,0x7E,0x24,0x24,0x7E,0x24,0x00,
  0x00,0x08,0x3E,0x28,0x3E,0x0A,0x3E,0x08,0x00,0x62,0x64,0x08,0x10,0x26,0x46,0x00,
  0x00,0x10,0x28,0x10,0x2A,0x44,0x3A,0x00,0x00,0x08,0x10,0x00,0x00,0x00,0x00,0x00,
  0x00,0x04,0x08,0x08,0x08,0x08,0x04,0x00,0x00,0x20,0x10,0x10,0x10,0x10,0x20,0x00,
  0x00,0x00,0x14,0x08,0x3E,0x08,0x14,0x00,0x00,0x00,0x08,0x08,0x3E,0x08,0x08,0x00,
  0x00,0x00,0x00,0x00,0x00,0x08,0x08,0x10,0x00,0x00,0x00,0x00,0x3E,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x18,0x18,0x00,0x00,0x00,0x02,0x04,0x08,0x10,0x20,0x00,
  0x00,0x3C,0x46,0x4A,0x52,0x62,0x3C,0x00,0x00,0x18,0x28,0x08,0x08,0x08,0x3E,0x00,
  0x00,0x3C,0x42,0x02,0x3C,0x40,0x7E,0x00,0x00,0x3C,0x42,0x0C,0x02,0x42,0x3C,0x00,
  0x00,0x08,0x18,0x28,0x48,0x7E,0x08,0x00,0x00,0x7E,0x40,0x7C,0x02,0x42,0x3C,0x00,
  0x00,0x3C,0x40,0x7C,0x42,0x42,0x3C,0x00,0x00,0x7E,0x02,0x04,0x08,0x10,0x10,0x00,
  0x00,0x3C,0x42,0x3C,0x42,0x42,0x3C,0x00,0x00,0x3C,0x42,0x42,0x3E,0x02,0x3C,0x00,
  0x00,0x00,0x00,0x10,0x00,0x00,0x10,0x00,0x00,0x00,0x10,0x00,0x00,0x10,0x10,0x20,
  0x00,0x00,0x04,0x08,0x10,0x08,0x04,0x00,0x00,0x00,0x00,0x3E,0x00,0x3E,0x00,0x00,
  0x00,0x00,0x10,0x08,0x04,0x08,0x10,0x00,0x00,0x3C,0x42,0x04,0x08,0x00,0x08,0x00,
  0x00,0x3C,0x4A,0x56,0x5E,0x40,0x3C,0x00,0x00,0x3C,0x42,0x42,0x7E,0x42,0x42,0x00,
  0x00,0x7C,0x42,0x7C,0x42,0x42,0x7C,0x00,0x00,0x3C,0x42,0x40,0x40,0x42,0x3C,0x00,
  0x00,0x78,0x44,0x42,0x42,0x44,0x78,0x00,0x00,0x7E,0x40,0x7C,0x40,0x40,0x7E,0x00,
  0x00,0x7E,0x40,0x7C,0x40,0x40,0x40,0x00,0x00,0x3C,0x42,0x40,0x4E,0x42,0x3C,0x00,
  0x00,0x42,0x42,0x7E,0x42,0x42,0x42,0x00,0x00,0x3E,0x08,0x08,0x08,0x08,0x3E,0x00,
  0x00,0x02,0x02,0x02,0x42,0x42,0x3C,0x00,0x00,0x44,0x48,0x70,0x48,0x44,0x42,0x00,
  0x00,0x40,0x40,0x40,0x40,0x40,0x7E,0x00,0x00,0x42,0x66,0x5A,0x42,0x42,0x42,0x00,
  0x00,0x42,0x62,0x52,0x4A,0x46,0x42,0x00,0x00,0x3C,0x42,0x42,0x42,0x42,0x3C,0x00,
  0x00,0x7C,0x42,0x42,0x7C,0x40,0x40,0x00,0x00,0x3C,0x42,0x42,0x52,0x4A,0x3C,0x00,
  0x00,0x7C,0x42,0x42,0x7C,0x44,0x42,0x00,0x00,0x3C,0x40,0x3C,0x02,0x42,0x3C,0x00,
  0x00,0xFE,0x10,0x10,0x10,0x10,0x10,0x00,0x00,0x42,0x42,0x42,0x42,0x42,0x3C,0x00,
  0x00,0x42,0x42,0x42,0x42,0x24,0x18,0x00,0x00,0x42,0x42,0x42,0x42,0x5A,0x24,0x00,
  0x00,0x42,0x24,0x18,0x18,0x24,0x42,0x00,0x00,0x82,0x44,0x28,0x10,0x10,0x10,0x00,
  0x00,0x7E,0x04,0x08,0x10,0x20,0x7E,0x00,0x00,0x0E,0x08,0x08,0x08,0x08,0x0E,0x00,
  0x00,0x00,0x40,0x20,0x10,0x08,0x04,0x00,0x00,0x70,0x10,0x10,0x10,0x10,0x70,0x00,
  0x00,0x10,0x38,0x54,0x10,0x10,0x10,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF,
  0x00,0x1C,0x22,0x78,0x20,0x20,0x7E,0x00,0x00,0x00,0x38,0x04,0x3C,0x44,0x3C,0x00,
  0x00,0x20,0x20,0x3C,0x22,0x22,0x3C,0x00,0x00,0x00,0x1C,0x20,0x20,0x20,0x1C,0x00,
  0x00,0x04,0x04,0x3C,0x44,0x44,0x3C,0x00,0x00,0x00,0x38,0x44,0x78,0x40,0x3C,0x00,
  0x00,0x0C,0x10,0x18,0x10,0x10,0x10,0x00,0x00,0x00,0x3C,0x44,0x44,0x3C,0x04,0x38,
  0x00,0x40,0x40,0x78,0x44,0x44,0x44,0x00,0x00,0x10,0x00,0x30,0x10,0x10,0x38,0x00,
  0x00,0x04,0x00,0x04,0x04,0x04,0x24,0x18,0x00,0x20,0x28,0x30,0x30,0x28,0x24,0x00,
  0x00,0x10,0x10,0x10,0x10,0x10,0x0C,0x00,0x00,0x00,0x68,0x54,0x54,0x54,0x54,0x00,
  0x00,0x00,0x78,0x44,0x44,0x44,0x44,0x00,0x00,0x00,0x38,0x44,0x44,0x44,0x38,0x00,
  0x00,0x00,0x78,0x44,0x44,0x78,0x40,0x40,0x00,0x00,0x3C,0x44,0x44,0x3C,0x04,0x06,
  0x00,0x00,0x1C,0x20,0x20,0x20,0x20,0x00,0x00,0x00,0x38,0x40,0x38,0x04,0x78,0x00,
  0x00,0x10,0x38,0x10,0x10,0x10,0x0C,0x00,0x00,0x00,0x44,0x44,0x44,0x44,0x38,0x00,
  0x00,0x00,0x44,0x44,0x28,0x28,0x10,0x00,0x00,0x00,0x44,0x54,0x54,0x54,0x28,0x00,
  0x00,0x00,0x44,0x28,0x10,0x28,0x44,0x00,0x00,0x00,0x44,0x44,0x44,0x3C,0x04,0x38,
  0x00,0x00,0x7C,0x08,0x10,0x20,0x7C,0x00,0x00,0x0E,0x08,0x30,0x08,0x08,0x0E,0x00,
  0x00,0x08,0x08,0x08,0x08,0x08,0x08,0x00,0x00,0x70,0x10,0x0C,0x10,0x10,0x70,0x00,
  0x00,0x14,0x28,0x00,0x00,0x00,0x00,0x00,0x3C,0x42,0x99,0xA1,0xA1,0x99,0x42,0x3C,
};

static const unsigned char BoldFont[] =
{
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, /* 0x00 */
0x00,0xC0,0xF0,0xFC,0xFF,0xFC,0xF0,0xC0,
0x03,0x03,0x06,0x06,0xCC,0x6C,0x38,0x18,
0x00,0x00,0xF0,0xFE,0x82,0x82,0x82,0xFE,
0x00,0xFE,0x82,0xBA,0x82,0xBA,0x82,0xFE,
0x00,0x00,0x10,0x38,0x7C,0x00,0x00,0x00,
0x00,0x00,0x7C,0x38,0x10,0x00,0x00,0x00,
0x00,0x00,0x00,0x78,0x78,0xFC,0x00,0x00,
0xBE,0x08,0x08,0x08,0x08,0x08,0x08,0x00,
0x00,0xFC,0xFC,0xFE,0xFE,0xFF,0x7F,0x7F,
0x7B,0x7B,0x79,0x79,0x78,0xFC,0xFC,0xFC,
0xFC,0x00,0xFC,0xFC,0xFC,0x00,0xFC,0xFC,
0x88,0x89,0x8A,0xAA,0xDB,0x8A,0x8A,0x00,
0x00,0x3F,0x3F,0x7F,0x7F,0xFF,0xFE,0xFE,
0xDE,0xDE,0x9E,0x9E,0x1E,0x3F,0x3F,0x3F,
0x3F,0x00,0x3F,0x3F,0x3F,0x00,0x3F,0x3F,
0xEF,0x28,0x28,0x2F,0xEA,0x29,0x28,0x00, /* 0x10 */
0x00,0x07,0x0F,0x0F,0x1E,0x1E,0x1E,0x0F,
0x0F,0x07,0x03,0x00,0x1E,0x1E,0x1F,0x0F,
0x07,0x00,0x07,0x07,0x07,0x00,0x07,0x07,
0x3E,0xA0,0xA0,0x38,0x20,0x20,0xBE,0x00,
0x00,0xFC,0xFE,0xFF,0x0F,0x0F,0x00,0xF8,
0xFC,0xFE,0xFE,0x0F,0x0F,0x0F,0xFE,0xFE,
0xFC,0x00,0xFC,0xFC,0xFC,0x00,0xFC,0xFC,
0x07,0x04,0x04,0x07,0x04,0x04,0x07,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x41,0x42,0x21,0x1B,0x06,0x03,0x0F,0x33,
0x45,0x49,0x51,0x11,0x10,0x10,0x01,0x00,
0x91,0x51,0x51,0x8F,0x41,0x51,0x8E,0x80,
0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,
0x41,0x21,0xC2,0x6C,0x30,0x60,0x78,0xE6,
0xD1,0xC9,0xC5,0xC4,0x84,0x84,0x40,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, /* 0x20 */
0x00,0x00,0x04,0x00,0x00,0x00,0x40,0x00,
0x00,0x36,0x36,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x08,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x40,0x00,
0x00,0x20,0x00,0x02,0x00,0x00,0x00,0x00,
0x00,0x20,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x02,0x00,0x00,
0x6C,0xBE,0xBE,0xFE,0x7C,0x38,0x10,0x00,
0x00,0x00,0x20,0x00,0x00,0x10,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x08,0x00,0x00,
0x00,0x00,0x18,0x18,0x7E,0x7E,0x18,0x18,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x40,
0x00,0x00,0x00,0x00,0x7E,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x18,0x18,
0x00,0x02,0x06,0x0C,0x18,0x30,0x60,0x40, /* / */
0x00,0x1C,0x22,0x63,0x63,0x63,0x22,0x1C, /* 0x30 */
0x00,0x18,0x38,0x18,0x18,0x18,0x18,0x7E,
0x00,0x3E,0x63,0x03,0x0E,0x3C,0x70,0x7F,
0x00,0x3E,0x63,0x03,0x0E,0x03,0x63,0x3E,
0x00,0x0E,0x1E,0x36,0x66,0x66,0x7F,0x06,
0x00,0x7F,0x60,0x7E,0x63,0x03,0x63,0x3E,
0x00,0x3E,0x63,0x60,0x7E,0x63,0x63,0x3E,
0x00,0x7F,0x63,0x06,0x0C,0x18,0x18,0x18,
0x00,0x3E,0x63,0x63,0x3E,0x63,0x63,0x3E,
0x00,0x3E,0x63,0x63,0x3F,0x03,0x63,0x3E,
0x00,0x18,0x18,0x00,0x00,0x00,0x18,0x18, /* : */
0xF0,0xF0,0xF0,0xF8,0xF8,0x7E,0x3F,0x0F,
0xFF,0xFF,0x00,0x00,0x00,0x00,0xFF,0xFF,
0xFF,0xFE,0x00,0x00,0x00,0x01,0xFF,0xFF,
0x00,0x60,0x30,0x18,0x0C,0x18,0x30,0x60,
0x1E,0x33,0x33,0x06,0x0C,0x00,0x0C,0x0C,
0xDB,0xBD,0xA5,0x66,0x3C,0x18,0x18,0x18, /* 0x40 */
0x00,0x1C,0x36,0x63,0x63,0x7F,0x63,0x63,
0x00,0x7E,0x63,0x63,0x7E,0x63,0x63,0x7E,
0x00,0x3E,0x63,0x60,0x60,0x60,0x63,0x3E,
0x00,0x7C,0x62,0x63,0x63,0x63,0x62,0x7C,
0x00,0x7F,0x60,0x60,0x7E,0x60,0x60,0x7F,
0x00,0x7F,0x60,0x60,0x7E,0x60,0x60,0x60,
0x00,0x3E,0x63,0x60,0x67,0x63,0x63,0x3E,
0x00,0x63,0x63,0x63,0x7F,0x63,0x63,0x63,
0x00,0x3C,0x18,0x18,0x18,0x18,0x18,0x3C,
0x00,0x1F,0x06,0x06,0x06,0x06,0x66,0x3C,
0x00,0x63,0x66,0x6C,0x78,0x7C,0x6E,0x67,
0x00,0x60,0x60,0x60,0x60,0x60,0x60,0x7F,
0x00,0x63,0x77,0x7F,0x7F,0x6B,0x63,0x63,
0x00,0x63,0x73,0x7B,0x7F,0x6F,0x67,0x63,
0x00,0x3E,0x63,0x63,0x63,0x63,0x63,0x3E,
0x00,0x7E,0x63,0x63,0x63,0x7E,0x60,0x60, /* 0x50 */
0x00,0x3E,0x63,0x63,0x63,0x6F,0x66,0x3D,
0x00,0x7E,0x63,0x63,0x62,0x7C,0x66,0x63,
0x00,0x3E,0x63,0x60,0x3E,0x03,0x63,0x3E,
0x00,0x7E,0x18,0x18,0x18,0x18,0x18,0x18,
0x00,0x63,0x63,0x63,0x63,0x63,0x63,0x3E,
0x00,0x63,0x63,0x63,0x63,0x36,0x1C,0x08,
0x00,0x63,0x63,0x6B,0x6B,0x7F,0x77,0x22,
0x00,0x63,0x76,0x3C,0x1C,0x1E,0x37,0x63,
0x00,0x63,0x63,0x33,0x1E,0x0C,0x18,0x70,
0x00,0x7F,0x07,0x0E,0x1C,0x38,0x70,0x7F,
0x38,0x44,0xBA,0xA2,0xA2,0xBA,0x44,0x38,
0x00,0x40,0x60,0x30,0x18,0x0C,0x06,0x02, /* \ */
0x70,0x10,0x10,0x10,0x10,0x10,0x70,0x00,
0x20,0x50,0x88,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x7F,0x7F,
0x34,0x34,0x34,0x34,0x34,0x34,0x3C,0x18, /* 0x60 */
0x00,0x1C,0x36,0x63,0x63,0x7F,0x63,0x63,
0x00,0x7E,0x63,0x63,0x7E,0x63,0x63,0x7E,
0x00,0x3E,0x63,0x60,0x60,0x60,0x63,0x3E,
0x00,0x7C,0x62,0x63,0x63,0x63,0x62,0x7C,
0x00,0x7F,0x60,0x60,0x7E,0x60,0x60,0x7F,
0x00,0x7F,0x60,0x60,0x7E,0x60,0x60,0x60,
0x00,0x3E,0x63,0x60,0x67,0x63,0x63,0x3E,
0x00,0x63,0x63,0x63,0x7F,0x63,0x63,0x63,
0x00,0x3C,0x18,0x18,0x18,0x18,0x18,0x3C,
0x00,0x1F,0x06,0x06,0x06,0x06,0x66,0x3C,
0x00,0x63,0x66,0x6C,0x78,0x7C,0x6E,0x67,
0x00,0x60,0x60,0x60,0x60,0x60,0x60,0x7F,
0x00,0x63,0x77,0x7F,0x7F,0x6B,0x63,0x63,
0x00,0x63,0x73,0x7B,0x7F,0x6F,0x67,0x63,
0x00,0x3E,0x63,0x63,0x63,0x63,0x63,0x3E,
0x00,0x7E,0x63,0x63,0x63,0x7E,0x60,0x60, /* 0x70 */
0x00,0x3E,0x63,0x63,0x63,0x6F,0x66,0x3D,
0x00,0x7E,0x63,0x63,0x62,0x7C,0x66,0x63,
0x00,0x3E,0x63,0x60,0x3E,0x03,0x63,0x3E,
0x00,0x7E,0x18,0x18,0x18,0x18,0x18,0x18,
0x00,0x63,0x63,0x63,0x63,0x63,0x63,0x3E,
0x00,0x63,0x63,0x63,0x63,0x36,0x1C,0x08,
0x00,0x63,0x63,0x6B,0x6B,0x7F,0x77,0x22,
0x00,0x63,0x76,0x3C,0x1C,0x1E,0x37,0x63,
0x00,0x63,0x63,0x33,0x1E,0x0C,0x18,0x70,
0x00,0x7F,0x07,0x0E,0x1C,0x38,0x70,0x7F,
0x66,0x3C,0x00,0x00,0x66,0x66,0x00,0xFF,
0x3F,0x40,0x80,0x98,0x9C,0x8C,0x80,0x80,
0x80,0x80,0x8C,0x9C,0x98,0x80,0x40,0x3F,
0x00,0x00,0x36,0x7E,0x6C,0x00,0x00,0x00,
0x01,0x01,0x31,0x39,0x19,0x01,0x02,0xFC,
0x00,0x08,0x50,0x20,0x04,0x45,0x02,0x00, /* 0x80 */
0x04,0x50,0x20,0x01,0x01,0x4A,0x04,0x00,
0x3C,0x5E,0xBF,0xBF,0xFF,0xFF,0x7E,0x3C,
0x04,0x21,0x44,0x00,0x09,0x40,0x02,0x91,
0xFF,0x80,0xA2,0x80,0x88,0x81,0x90,0x84,
0x80,0x88,0x80,0xA1,0x84,0xA8,0x80,0xFF,
0xFF,0x01,0x21,0x05,0x81,0x11,0x49,0x21,
0x01,0x25,0x41,0x11,0x01,0x45,0x01,0xFF,
0x80,0xA0,0x82,0x80,0x90,0x89,0xA0,0x80,
0xFF,0x00,0x08,0x81,0x20,0x02,0x48,0x00,
0x01,0x11,0x85,0x09,0x01,0x21,0x05,0x41,
0x00,0x0A,0x40,0x08,0x12,0x00,0x00,0xFF,
0x88,0x44,0x22,0x11,0x88,0x44,0x22,0x11,
0xFE,0x7C,0x38,0x10,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x10,0x38,0x7C,0xFE,
0x80,0xC0,0xE0,0xF0,0xE0,0xC0,0x80,0x00,
0x01,0x03,0x07,0x0F,0x07,0x03,0x01,0x00,
0xFF,0x7E,0x3C,0x18,0x18,0x3C,0x7E,0xFF,
0x81,0xC3,0xE7,0xFF,0xFF,0xE7,0xC3,0x81,
0xF0,0xF0,0xF0,0xF0,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x0F,0x0F,0x0F,0x0F,
0x0F,0x0F,0x0F,0x0F,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0xF0,0xF0,0xF0,0xF0,
0x33,0x33,0xCC,0xCC,0x33,0x33,0xCC,0xCC,
0x00,0x20,0x20,0x50,0x50,0x88,0xF8,0x00,
0x20,0x20,0x70,0x20,0x70,0x20,0x20,0x00,
0x00,0x00,0x00,0x50,0x88,0xA8,0x50,0x00,
0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
0x00,0x00,0x00,0x00,0xFF,0xFF,0xFF,0xFF,
0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,
0x0F,0x0F,0x0F,0x0F,0x0F,0x0F,0x0F,0x0F,
0xFF,0xFF,0xFF,0xFF,0x00,0x00,0x00,0x00,
0x00,0x00,0x68,0x90,0x90,0x90,0x68,0x00,
0x30,0x48,0x48,0x70,0x48,0x48,0x70,0xC0,
0xF8,0x88,0x80,0x80,0x80,0x80,0x80,0x00,
0xF8,0x50,0x50,0x50,0x50,0x50,0x98,0x00,
0xF8,0x88,0x40,0x20,0x40,0x88,0xF8,0x00,
0x00,0x00,0x78,0x90,0x90,0x90,0x60,0x00,
0x00,0x50,0x50,0x50,0x50,0x68,0x80,0x80,
0x00,0x50,0xA0,0x20,0x20,0x20,0x20,0x00,
0xF8,0x20,0x70,0xA8,0xA8,0x70,0x20,0xF8,
0x20,0x50,0x88,0xF8,0x88,0x50,0x20,0x00,
0x70,0x88,0x88,0x88,0x50,0x50,0xD8,0x00,
0x30,0x40,0x40,0x20,0x50,0x50,0x50,0x20,
0x00,0x00,0x00,0x50,0xA8,0xA8,0x50,0x00,
0x08,0x70,0xA8,0xA8,0xA8,0x70,0x80,0x00,
0x38,0x40,0x80,0xF8,0x80,0x40,0x38,0x00,
0x70,0x88,0x88,0x88,0x88,0x88,0x88,0x00,
0x00,0xF8,0x00,0xF8,0x00,0xF8,0x00,0x00,
0x20,0x20,0xF8,0x20,0x20,0x00,0xF8,0x00,
0xC0,0x30,0x08,0x30,0xC0,0x00,0xF8,0x00,
0x18,0x60,0x80,0x60,0x18,0x00,0xF8,0x00,
0x10,0x28,0x20,0x20,0x20,0x20,0x20,0x20,
0x20,0x20,0x20,0x20,0x20,0x20,0xA0,0x40,
0x00,0x20,0x00,0xF8,0x00,0x20,0x00,0x00,
0x00,0x50,0xA0,0x00,0x50,0xA0,0x00,0x00,
0x00,0x18,0x24,0x24,0x18,0x00,0x00,0x00,
0x00,0x30,0x78,0x78,0x30,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x30,0x00,0x00,0x00,
0x3E,0x20,0x20,0x20,0xA0,0x60,0x20,0x00,
0xA0,0x50,0x50,0x50,0x00,0x00,0x00,0x00,
0x40,0xA0,0x20,0x40,0xE0,0x00,0x00,0x00,
0x00,0x38,0x38,0x38,0x38,0x38,0x38,0x00,
0x88,0x70,0x88,0x88,0x70,0x88,0x00,0x00,
0x07,0x1C,0x30,0x67,0x48,0xD4,0x91,0x93,
0x93,0x91,0xD4,0x48,0x67,0x30,0x1C,0x07,
0xE0,0x38,0x0C,0xE6,0x12,0x2B,0x89,0xC9,
0xC9,0x89,0x2B,0x12,0xE6,0x0C,0x38,0xE0,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x41,0xC1,0x19,0x31,0x26,0x08,0x0C,0xF3,
0xF1,0x09,0x0A,0x26,0x31,0x19,0xC1,0x41,
0x82,0x83,0x98,0x8C,0x64,0x50,0x90,0x8F,
0xCF,0x30,0x10,0x64,0x8C,0x98,0x83,0x82,
0x00,0x24,0x5A,0x3C,0x3C,0x5A,0x24,0x00,
0x00,0x00,0x88,0xD8,0xA8,0x88,0x88,0x00,
0x00,0x00,0x88,0x88,0xF8,0x88,0x88,0x00,
0x00,0x00,0x70,0x88,0x88,0x88,0x70,0x00,
0x02,0x48,0x00,0x82,0x10,0x00,0x44,0x10,
0x00,0x41,0x04,0x11,0x40,0x04,0x41,0x08,
0xC3,0x99,0x2C,0x5E,0x7E,0x3C,0x99,0xC3,
0xC3,0x99,0x2C,0x62,0x46,0x34,0x99,0xC3,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x40,0x40,0x70,0x48,0x70,0x00,
0x00,0x00,0x88,0x88,0xC8,0xA8,0xC8,0x00,
0x00,0x00,0xF0,0x08,0x70,0x08,0xF0,0x00,
0x00,0x00,0xA8,0xA8,0xA8,0xA8,0xF8,0x00,
0x00,0x00,0x70,0x88,0x38,0x88,0x70,0x00,
0x00,0x00,0xA8,0xA8,0xA8,0xF8,0x08,0x00,
0x00,0x00,0x48,0x48,0x78,0x08,0x08,0x00,
0x00,0x00,0xC0,0x40,0x70,0x48,0x70,0x00,
0x90,0xA8,0xA8,0xE8,0xA8,0xA8,0x90,0x00,
0x20,0x50,0x88,0x88,0xF8,0x88,0x88,0x00,
0xF8,0x88,0x80,0xF0,0x88,0x88,0xF0,0x00,
0x90,0x90,0x90,0x90,0x90,0xF8,0x08,0x00,
0x38,0x28,0x28,0x48,0x48,0xF8,0x88,0x00,
0xF8,0x80,0x80,0xF0,0x80,0x80,0xF8,0x00,
0x20,0x70,0xA8,0xA8,0xA8,0x70,0x20,0x00,
0xF8,0x88,0x88,0x80,0x80,0x80,0x80,0x00,
0x88,0x88,0x50,0x20,0x50,0x88,0x88,0x00,
0x88,0x88,0x98,0xA8,0xC8,0x88,0x88,0x00,
0x50,0x20,0x88,0x98,0xA8,0xC8,0x88,0x00,
0x88,0x90,0xA0,0xC0,0xA0,0x90,0x88,0x00,
0x18,0x28,0x48,0x48,0x48,0x48,0x88,0x00,
0x88,0xD8,0xA8,0xA8,0x88,0x88,0x88,0x00,
0x88,0x88,0x88,0xF8,0x88,0x88,0x88,0x00,
0x70,0x88,0x88,0x88,0x88,0x88,0x70,0x00,
0xF8,0x88,0x88,0x88,0x88,0x88,0x88,0x00,
0x78,0x88,0x88,0x78,0x28,0x48,0x88,0x00,
0xF0,0x88,0x88,0xF0,0x80,0x80,0x80,0x00,
0x70,0x88,0x80,0x80,0x80,0x88,0x70,0x00,
0xF8,0x20,0x20,0x20,0x20,0x20,0x20,0x00,
0x88,0x88,0x88,0x50,0x20,0x40,0x80,0x00,
0xA8,0xA8,0x70,0x20,0x70,0xA8,0xA8,0x00,
0xF0,0x48,0x48,0x70,0x48,0x48,0xF0,0x00,
0x80,0x80,0x80,0xF0,0x88,0x88,0xF0,0x00,
0x88,0x88,0x88,0xC8,0xA8,0xA8,0xC8,0x00,
0xF0,0x08,0x08,0x30,0x08,0x08,0xF0,0x00,
0xA8,0xA8,0xA8,0xA8,0xA8,0xA8,0xF8,0x00,
0x70,0x88,0x08,0x78,0x08,0x88,0x70,0x00,
0xA8,0xA8,0xA8,0xA8,0xA8,0xF8,0x08,0x00,
0x88,0x88,0x88,0x88,0x78,0x08,0x08,0x00,
0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF
};

static const unsigned char *CurFont = NormalFont;
static char Result[256];

static const char *nth(const char *S,int N)
{
  while(N) { if(!*S++) { N--;if(!*S) return(0); } }
  return(S);  
}

#if !defined(_MSC_VER) && !defined(Q_OS_QNX)
static int stricmp(const char *S1,const char *S2)
{
  while(*S1&&(toupper(*S1)==toupper(*S2))) { ++S1;++S2; }
  return(toupper(*S1)-toupper(*S2));
}
#endif /* _MSC_VER */

static int smartcpy(char *D,const char *S,int N)
{
  int J;

  if(N<=0) return(0);
  for(J=0;(J<N-1)&&*S;J++) *D++=*S++;
  *D='\0';
  /* If copied everything, exit */
  if(!*S) return(J);
  /* Draw up to 3 dots */
  for(N=N>3? 3:N;N>0;--N) *(--D)='.';
  return(J);
}

static int chomp(char *D,int N)
{
  int J;

  J=strlen(D);
  if(J<=N) return(J);
  D[N]='\0';
  for(J=N-1;(J>0)&&(N-J<=3);--J) D[J]='.';
  return(N);
}

void CONSetFont(const unsigned char *Font)
{
  CurFont = Font==FNT_NORMAL? NormalFont
          : Font==FNT_BOLD?   BoldFont
          : Font;
}

#endif /* DEFINE_ONCE */

void CONSetColor(pixel FGColor,pixel BGColor)
{
  FG=FGColor;
  BG=BGColor;
}

void CONClear(pixel BGColor)
{
  pixel *P;
  int X,Y;

  if(!VideoImg) return;

  P=(pixel *)VideoImg->Data+VideoY*VideoImg->L+VideoX;
  for(Y=VideoH;Y;--Y,P+=VideoImg->L)
    for(X=0;X<VideoW;X++) 
      P[X]=BGColor;
}

void CONBox(int X,int Y,int Width,int Height,pixel BGColor)
{
  if(!VideoImg) return;

  X       = X<0? 0:X>=VideoW? VideoW-1:X;
  Y       = Y<0? 0:Y>=VideoH? VideoH-1:Y;
  Width   = Width>VideoW-X? VideoW-X:Width;
  Height  = Height>VideoH-Y? VideoH-Y:Height;
  IMGFillRect(VideoImg,VideoX+X,VideoY+Y,Width,Height,BGColor);
}

void CONFrame(int X,int Y,int Width,int Height,pixel FGColor)
{
  if(!VideoImg) return;

  X       = X<0? 0:X>=VideoW? VideoW-1:X;
  Y       = Y<0? 0:Y>=VideoH? VideoH-1:Y;
  Width   = Width>VideoW-X? VideoW-X:Width;
  Height  = Height>VideoH-Y? VideoH-Y:Height;
  IMGDrawRect(VideoImg,VideoX+X,VideoY+Y,Width,Height,FGColor);
}

void PrintXY(Image *Img,const char *S,int X,int Y,pixel FG,int BG)
{
  const unsigned char *C;
  pixel *P;
  int I,J,K,N;

  X = X<0? 0:X>Img->W-8? Img->W-8:X;
  Y = Y<0? 0:Y>Img->H-8? Img->H-8:Y;

  for(K=X;*S;S++)
    switch(*S)
    {
      case '\n':
        K=X;Y+=8;
        if(Y>Img->H-8) Y=0;
        break;
      default:
        P=(pixel *)Img->Data+Img->L*Y+K;
        if(BG<0)
        {
          for(C=CurFont+(*S<<3),J=8;J;P+=Img->L,++C,--J)
            for(I=0,N=(int)*C<<24;N&&(I<8);++I,N<<=1)
              if(N&0x80000000) P[I]=FG;
        }
        else
        {
          for(C=CurFont+(*S<<3),J=8;J;P+=Img->L,++C,--J)
            for(I=0,N=*C;I<8;++I)
              P[I]=N&(0x80>>I)? FG:BG;
        }
        K+=8;
        if(X>Img->W-8)
        {
          K=0;Y+=8;
          if(Y>Img->H-8) Y=0;
        }
        break;
    }
}

void CONChar(int X,int Y,char V)
{
  const unsigned char *C;
  pixel *P;
  int I,J,K;

  if(!VideoImg) return;

  X<<= 3;
  Y<<= 3;
  X  = X<0? 0:X>=VideoW-8? VideoW-8:X;
  Y  = Y<0? 0:Y>=VideoH-8? VideoH-8:Y;
  P  = (pixel *)VideoImg->Data+VideoImg->L*(VideoY+Y)+VideoX+X;
  for(C=CurFont+(V<<3),J=8;J;P+=VideoImg->L,++C,--J)
    for(I=0,K=*C;I<8;++I) P[I]=K&(0x80>>I)? FG:BG;
}

void CONPrintN(int X,int Y,const char *S,int N)
{
  int J,X1;

  /* Truncate N to accommodate the screen size */
  J = (VideoW>>3)-X;
  N = N<=J? N:J;

  for(;*S&&(Y<(VideoH>>3));++Y,S+=J)
  {
    /* Print N-1 characters */
    for(X1=X,J=0;!EOLN(S[J])&&(J<N-1);++J) CONChar(X1++,Y,S[J]);
    /* If string longer than N-1... */
    if(!EOLN(S[J]))
    {
      /* If string longer than N, print dots */
      CONChar(X1,Y,EOLN(S[J+1])? S[J]:CON_DOTS);
      /* Skip rest of the line */
      while(!EOLN(S[J])) ++J;
      /* Skip control character */
      if(S[J]) ++J;
    }
  } 
}

void CONPrint(int X,int Y,const char *S)
{
  const unsigned char *C;
  pixel *P;
  int I,J,K;

  if(!VideoImg) return;

  X<<= 3;
  Y<<= 3;
  X  = X<0? 0:X>=VideoW-8? VideoW-8:X;
  Y  = Y<0? 0:Y>=VideoH-8? VideoH-8:Y;

  for(K=X;*S;S++)
    switch(*S)
    {
      case '\n':
        K=X;Y+=8;
        if(Y>=VideoH) Y=0;
        break;
      default:
        P=(pixel *)VideoImg->Data+VideoImg->L*(VideoY+Y)+VideoX+K;
        for(C=CurFont+(*S<<3),J=8;J;P+=VideoImg->L,++C,--J)
          for(I=0;I<8;++I) P[I]=*C&(0x80>>I)? FG:BG;
        K+=8;
        if(X>=VideoW)
        {
          K=0;Y+=8;
          if(Y>=VideoH) Y=0;
        }
        break;
    }
}

void CONWindow(int X,int Y,int W,int H,pixel FGColor,pixel BGColor,const char *Title)
{
  int J;

  if(!VideoImg) return;

  /* Draw menu box */
  CONBox(X<<3,Y<<3,W<<3,8,FGColor);
  CONBox(X<<3,(Y+1)<<3,W<<3,(H-1)<<3,BGColor);

  /* Draw title */
  CONSetColor(BGColor,FGColor);
  J=strlen(Title);
  if(J>W-2) J=W-2;
  CONPrintN(X+((W-J)>>1),Y,Title,J);

  /* Draw frame */
  CONFrame(X<<3,(Y+1)<<3,W<<3,(H-1)<<3,FGColor);
  CONFrame((X<<3)-1,(Y<<3)-1,(W<<3)+2,(H<<3)+2,BGColor);

  /* Set "normal" colors for window body */
  CONSetColor(FGColor,BGColor);
}

void CONMsg(int X,int Y,int W,int H,pixel FGColor,pixel BGColor,const char *Title,const char *Text)
{
  const char *P;
  int I,J,Total,Wait;

  if(!VideoImg) return;

  /* Check if it is a no-wait message */
  if(Title[0]=='~') { ++Title;Wait=0; } else Wait=1;

  /* Compute message dimensions */
  for(P=Text,I=strlen(Title),Total=0;*P;Total++,P++)
  {
    for(J=0;*P;P++,J++);
    if(J>I) I=J;
  }

  /* Update message box coordinates and dimensions */
  J = VideoW>>3;
  W = W>2? W:I+2;
  W = W>J-2? J-2:W;
  X = X<0? (J-W)/2:X+W>J? J-W:X;
  J = VideoH>>3;
  H = H>3? H:Total+3;
  H = H>J-2? J-2:H;
  Y = Y<0? (J-H)/2:Y+H>J? J-H:Y;

  /* Print text in portions, until it ends */
  for(P=Text;*P&&VideoImg;)
  {
    /* Draw window */
    CONWindow(X,Y,W,H,FGColor,BGColor,Title);

    /* Draw button icon if waiting for a button */
    if(Wait) CONChar(X+W-1,Y+H-1,CON_BUTTON);

    /* Draw frame */
    CONFrame(X<<3,(Y+1)<<3,W<<3,(H-1)<<3,FGColor);
    CONFrame((X<<3)-1,(Y<<3)-1,(W<<3)+2,(H<<3)+2,BGColor);

    /* Draw text */
    for(J=0;*P&&(J<H-3);P++,J++)
    {
      CONPrintN(X+1,Y+J+2,P,W-2);
      for(;*P;P++);
    }

    /* Wait for key */
    if(Wait) { ShowVideo();GetKey();WaitKey(); }
  }
}

/** CONInput() ***********************************************/
/** Show an input box. Input modes (text/hex/dec) are ORed  **/
/** to the Length argument.                                 **/
/*************************************************************/
char *CONInput(int X,int Y,pixel FGColor,pixel BGColor,const char *Title,char *Input,unsigned int Length)
{
  static const char DecDigits[] = "0123456789";
  static const char HexDigits[] = "0123456789ABCDEF";
  int J,I,H,W,Mode;

  /* Remove input mode modifiers off Length */
  Mode=Length&(CON_DEC|CON_HEX|CON_HIDE);
  Length&=~(CON_DEC|CON_HEX|CON_HIDE);

  /* Update input box coordinates and dimensions */
  I=strlen(Title);
  J = VideoW>>3;
  W = (I>Length? I:Length)+2;
  W = W>J-2? J-2:W;
  X = X<0? (J-W)/2:X+W>J? J-W:X;
  J = VideoH>>3;
  H = 4;
  H = H>J-2? J-2:H;
  Y = Y<0? (J-H)/2:Y+H>J? J-H:Y;

  /* Correct length */
  if(Length>W-2) Length=W-2;

  /* Draw window */
  CONWindow(X,Y,W,H,FGColor,BGColor,Title);

  for(J=I=0;(J>=0)&&(I!=CON_OK)&&(I!=CON_EXIT);)
  {
    CONChar(X+J+1,Y+H-2,'_');
    ShowVideo();
    I=WaitKey();
    if(!I||!VideoImg) return(0);
    if((I>=0x20)&&(I<0x80)&&(J<Length-1))
      if(
        ((Mode&CON_DEC)&&strchr(DecDigits,I)) ||
        ((Mode&CON_HEX)&&strchr(HexDigits,I)) ||
        !Mode
      )
      {
        CONChar(X+J+1,Y+H-2,Mode&CON_HIDE? '*':I);
        Input[J++]=I;
      }
    if(I==0x08)
    {
      CONChar(X+J+1,Y+H-2,' ');
      J--;
    }
  }

  if((J<0)||(I==CON_EXIT)) return(0);
  Input[J]='\0';
  return(Input);
}

int CONMenu(int X,int Y,int W,int H,pixel FGColor,pixel BGColor,const char *Items,int Item)
{
  const char *P;
  int I,J,Draw,Total,Top;
  int FileSelect;

  /* Drop out if no video buffer */
  if(!VideoImg) return(0);

  /* Compute menu items count and width */
  for(P=Items,I=0,Total=-1;*P;Total++,P++)
  {
    for(J=0;*P;P++,J++);
    if(J>I) I=J;
  }

  /* Update menu coordinates and dimensions */
  J = VideoW>>3;
  W = W>3? W:I+3;
  W = W>J-2? J-2:W;
  X = X<0? (J-W)/2:X+W>J? J-W:X;

  J = VideoH>>3;
  H = H>3? H:Total+3;
  H = H>J-2? J-2:H;
  Y = Y<0? (J-H)/2:Y+H>J? J-H:Y;

  /* File selector is a special mode */
  FileSelect = Item==TAG_SELEFILE;

  /* Prepare for selection loop */
  Item = FileSelect? 0:Item<1? 0:Item>Total? Total-1:Item-1;
  Top  = Item-Item%(H-3);
  Item = Item%(H-3);
  Draw = 1;

  /* Selection loop */
  do
  {
    if(Draw)
    {
      /* Draw window */
      CONWindow(X,Y,W,H,FGColor,BGColor,Items);

      /* Draw arrows */
      if(Top) CONChar(X+1,Y+1,CON_LESS);
      if(Top+H-3<Total) CONChar(X+1,Y+H-1,CON_MORE);

      /* Draw frame */
      CONFrame(X<<3,(Y+1)<<3,W<<3,(H-1)<<3,FGColor);
      CONFrame((X<<3)-1,(Y<<3)-1,(W<<3)+2,(H<<3)+2,BGColor);

      /* Skip to the top item */
      for(P=Items;*P;P++);
      for(P++,J=0;J<Top;J++,P++)
        for(;*P;P++);

      /* Draw menu */
      for(J=0;*P&&(J<H-3);J++,P++)
      {
        CONPrintN(X+2,Y+2+J,P,W-3);
        for(;*P;P++);
      }

      /* Done redrawing */
      Draw=0;
    }

    /* Draw arrow */
    CONChar(X+1,Y+2+Item,CON_ARROW);
    /* Show screen */
    ShowVideo();
    /* Get key, drop out if console closed */
    J=WaitKey();
    if(!J||!VideoImg) return(0);
    /* SPACE, ENTER, TAB treated as "OK" */
    if((J==' ')||(J==0x0A)||(J==0x0D)||(J==0x09)) J=CON_OK;
    /* ESCAPE, BS treated as "EXIT" */
    if((J==0x1B)||(J==0x08)) J=CON_EXIT;
    /* Erase arrow */
    CONChar(X+1,Y+2+Item,' ');

    /* When selecting a filename and a letter has been typed... */
    if(FileSelect&&(J>0x20)&&(J<0x80))
    {
      /* Draw input box */
      CONWindow(X,Y+H-4,W,4,FGColor,BGColor,"Enter Filename:");

      /* Input text */
      for(I=0;(I>=0)&&(J!=CON_OK)&&(J!=CON_EXIT);)
      {
        if((J>=0x20)&&(J<0x80)&&(I<W-3)&&(I<sizeof(Result)-1))
        {
          CONChar(X+I+1,Y+H-2,J);
          Result[I++]=J;
        }
        /* Show cursor */
        CONChar(X+I+1,Y+H-2,'_');
        /* Show screen */
        ShowVideo();
        /* Get key, drop out if console closed */
        J=WaitKey();
        if(!J||!VideoImg) return(0);
        /* Erase characters on BS */
        if(J==0x08)
        {
          I--;
          CONChar(X+I+1,Y+H-2,'_');
          CONChar(X+I+2,Y+H-2,' ');
        }
      }

      /* Terminate and return text */
      if(J==CON_OK) { Result[I]='\0';return((int)Result); }

      /* Redraw menu */
      Draw=1;
    }

    if(J==CON_UP)
    {
      if(Item>0) Item--;
      else
      {
        Draw=1;
        if(Top>0) Top--;
        else
        {
          Top=Total>(H-3)? Total-H+3:0;
          Item=Total-Top>0? Total-Top-1:0;
        }
      }
    }

    if(J==CON_DOWN)
    {
      if((Item<H-4)&&(Item+Top<Total-1)) Item++;
      else
      {
        Draw=1;
        if(Top+H-3<Total) Top++;
        else { Top=0;Item=0; }
      }
    }
  }
  while((J!=CON_OK)&&(J!=CON_EXIT));

  /* Return selection */
  return(J!=CON_OK? 0:FileSelect? (int)nth(Items,Top+Item+1):Top+Item+1); 
}

const char *CONFile(pixel FGColor,pixel BGColor,const char *Ext)
{
  struct dirent *DP;
  struct stat ST;
  int I,J,BufSize;
  const char *P;
  char *Buf,*T;
  DIR *D;

  if(!VideoImg) return(0);

  /* No buffer yet */
  Buf     = 0;
  BufSize = 0;

  do
  {
    /* Open current directory */
    if(!(D=opendir("."))) break;

    /* Compute required buffer size size */
    for(rewinddir(D),I=256;(DP=readdir(D));I+=strlen(DP->d_name)+2);

    /* Reallocate buffer if needed */
    if((I>BufSize)&&(T=(char*)malloc(I)))
    {
      if(Buf) free(Buf);
      BufSize = I;
      Buf     = T;
    }

    /* If failed to allocate buffer, drop out */
    if(!Buf) { closedir(D);return(0); }

    /* Create title from the current pathname */
    if(!getcwd(Buf,BufSize-2)) strcpy(Buf,"Choose File");
    J=strlen(Buf)+1;

    /* Scan subdirectories */
    for(rewinddir(D);(DP=readdir(D));)
      if(!stat(DP->d_name,&ST)&&S_ISDIR(ST.st_mode))
      {
        I=strlen(DP->d_name)+1;
        if(J+I+1<BufSize)
        {
          Buf[J++]=CON_FOLDER;
          strcpy(Buf+J,DP->d_name);
          J+=I;
        }
      }

    /* Scan files */
    for(rewinddir(D);(DP=readdir(D));)
      if(!stat(DP->d_name,&ST)&&!S_ISDIR(ST.st_mode))
      {
        I=strlen(DP->d_name)+1;
        if(J+I+1<BufSize)
          for(P=Ext;*P;P+=strlen(P)+1)
            if((I>strlen(P))&&!stricmp(DP->d_name+I-1-strlen(P),P))
            {
              Buf[J++]=CON_FILE;
              strcpy(Buf+J,DP->d_name);
              J+=I;
              break;
            }
      }

    /* Terminate directory listing */
    Buf[J]='\0';

    /* Done with directory */
    closedir(D);

    /* Show menu */
    P=(const char *)CONMenu(-1,-1,(VideoW>>3)-2,(VideoH>>3)-2,FGColor,BGColor,Buf,TAG_SELEFILE);

    /* Handle menu selection */
    if(P)
      switch(*P)
      {
        case CON_FOLDER:
          /* Folder selected, enter it */
          chdir(P+1);
          break;
        case CON_FILE:
          /* File selected, return it */
          strncpy(Result,P+1,sizeof(Result));
          Result[sizeof(Result)-1]='\0';
          free(Buf);
          return(Result);
        default:
          /* Filename entered into Result[] */
          free(Buf);
          return(Result);
      }
  }
  while(P);

  free(Buf);
  return(0);
}

#endif /* BPP32||BPP24||BPP16||BPP8 */
