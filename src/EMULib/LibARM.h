/** EMULib Emulation Library *********************************/
/**                                                         **/
/**                        LibARM.h                         **/
/**                                                         **/
/** This file declares optimized ARM assembler functions    **/
/** used to copy and process images on ARM-based platforms  **/
/** such as Symbian/S60, Symbian/UIQ, and Maemo. See files  **/
/** LibARM.asm (ARMSDT) and LibARM.s (GNU AS) for the       **/
/** actual functions.                                       **/
/**                                                         **/
/** Copyright (C) Marat Fayzullin 2005-2009                 **/
/**     You are not allowed to distribute this software     **/
/**     commercially. Please, notify me, if you make any    **/
/**     changes to this file.                               **/
/*************************************************************/
#ifndef LIBARM_H
#define LIBARM_H
#if defined(S60) || defined(UIQ) || defined(MAEMO) || defined(STMP3700)

#ifdef __cplusplus
extern "C" {
#endif

/** BPP16 Functions ******************************************/
/** These assembler functions operate on 16bpp pixels. They **/
/** all generally require Count to be divisible by 16 or 8. **/
/*************************************************************/
void C256T120_16(unsigned short *Dst,const unsigned short *Src,int Count);
void C256T160_16(unsigned short *Dst,const unsigned short *Src,int Count);
void C256T176_16(unsigned short *Dst,const unsigned short *Src,int Count);
void C256T208_16(unsigned short *Dst,const unsigned short *Src,int Count);
void C256T240_16(unsigned short *Dst,const unsigned short *Src,int Count);
void C256T256_16(unsigned short *Dst,const unsigned short *Src,int Count);
void C240T320_16(unsigned short *Dst,const unsigned short *Src,int Count);
void C256T320_16(unsigned short *Dst,const unsigned short *Src,int Count);
void C256T352_16(unsigned short *Dst,const unsigned short *Src,int Count);
void C256T416_16(unsigned short *Dst,const unsigned short *Src,int Count);
void C256T512_16(unsigned short *Dst,const unsigned short *Src,int Count);
void C256T768_16(unsigned short *Dst,const unsigned short *Src,int Count);
void TELEVIZE0_16(unsigned short *Dst,int Count);
void TELEVIZE1_16(unsigned short *Dst,int Count);

/** BPP32 Functions ******************************************/
/** These assembler functions operate on 32bpp pixels. They **/
/** all generally require Count to be divisible by 16 or 8. **/
/*************************************************************/
void C256T120_32(unsigned int *Dst,const unsigned int *Src,int Count);
void C256T160_32(unsigned int *Dst,const unsigned int *Src,int Count);
void C256T176_32(unsigned int *Dst,const unsigned int *Src,int Count);
void C256T208_32(unsigned int *Dst,const unsigned int *Src,int Count);
void C256T240_32(unsigned int *Dst,const unsigned int *Src,int Count);
void C256T256_32(unsigned int *Dst,const unsigned int *Src,int Count);
void C240T320_32(unsigned int *Dst,const unsigned int *Src,int Count);
void C256T320_32(unsigned int *Dst,const unsigned int *Src,int Count);
void C256T352_32(unsigned int *Dst,const unsigned int *Src,int Count);
void C256T416_32(unsigned int *Dst,const unsigned int *Src,int Count);
void C256T512_32(unsigned int *Dst,const unsigned int *Src,int Count);
void C256T768_32(unsigned int *Dst,const unsigned int *Src,int Count);
void TELEVIZE0_32(unsigned int *Dst,int Count);
void TELEVIZE1_32(unsigned int *Dst,int Count);

#undef C256T120
#undef C256T160
#undef C256T176
#undef C256T208
#undef C256T240
#undef C256T256
#undef C256T320
#undef C240T320
#undef C256T352
#undef C256T416
#undef C256T512
#undef C256T768
#undef TELEVIZE0
#undef TELEVIZE1

#if defined(BPP32) || defined(BPP24)
#define C256T120  C256T120_32
#define C256T160  C256T160_32
#define C256T176  C256T176_32
#define C256T208  C256T208_32
#define C256T240  C256T240_32
#define C256T256  C256T256_32
#define C256T320  C256T320_32
#define C240T320  C240T320_32
#define C256T352  C256T352_32
#define C256T416  C256T416_32
#define C256T512  C256T512_32
#define C256T768  C256T768_32
#define TELEVIZE0 TELEVIZE0_32
#define TELEVIZE1 TELEVIZE1_32

#elif defined(BPP16)
#define C256T120  C256T120_16
#define C256T160  C256T160_16
#define C256T176  C256T176_16
#define C256T208  C256T208_16
#define C256T240  C256T240_16
#define C256T256  C256T256_16
#define C256T320  C256T320_16
#define C240T320  C240T320_16
#define C256T352  C256T352_16
#define C256T416  C256T416_16
#define C256T512  C256T512_16
#define C256T768  C256T768_16
#define TELEVIZE0 TELEVIZE0_16
#define TELEVIZE1 TELEVIZE1_16

#endif

#ifdef __cplusplus
}
#endif
#endif /* S60 || UIQ || MAEMO || STMP3700 */
#endif /* LIBARM_H */
