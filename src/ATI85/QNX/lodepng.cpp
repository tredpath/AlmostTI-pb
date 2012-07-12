#include "../../EMULib/EMULib.h"
#include <png.h>
#include <stdio.h>
#include <stdlib.h>

int LoadPNG(Image *Img,const char *FileName)
{
	png_structp PNGRead;
	png_infop PNGInfo;
	png_infop PNGEnd;
	png_bytepp Rows;
	png_bytep Src;
	FILE *F;
	png_uint_32 W,H;
	int J,D,X,Y;
	char Buf[16];
	pixel *Dst;

	/* Open file */
	F=fopen(FileName,"rb");
	if(!F) return(0);

	PNGRead = png_create_read_struct(PNG_LIBPNG_VER_STRING,0,0,0);
	PNGInfo = PNGRead? png_create_info_struct(PNGRead):0;
	PNGEnd  = PNGInfo? png_create_info_struct(PNGRead):0;

	if(!PNGRead||!PNGInfo||!PNGEnd)
	{
		png_destroy_read_struct(PNGRead? &PNGRead:0,PNGInfo? &PNGInfo:0,PNGEnd? &PNGEnd:0);
		fclose(F);
		return(0);
	}

	if(setjmp(png_jmpbuf(PNGRead)))
	{
		png_destroy_read_struct(&PNGRead,&PNGInfo,&PNGEnd);
		fclose(F);
		return(0);
	}

	/* Read PNG file into memory */
	png_init_io(PNGRead,F);
	png_read_png(PNGRead,PNGInfo,PNG_TRANSFORM_EXPAND,0);

	/* Get metadata */
	Rows = png_get_rows(PNGRead,PNGInfo);
	W    = png_get_image_width(PNGRead,PNGInfo);
	H    = png_get_image_height(PNGRead,PNGInfo);
	D    = png_get_bit_depth(PNGRead,PNGInfo);
	W    = W>Img->W? Img->W:W;
	H    = H>Img->H? Img->H:H;

	/* Copy image */
	for(Y=0,Dst=Img->Data;Y<H;++Y,Dst+=Img->L-W)
		for(X=0,Src=Rows[Y];X<W;++X,++Dst,Src+=3)
			*Dst = PIXEL(Src[0],Src[1],Src[2]);

	/* Done */
	png_destroy_read_struct(&PNGRead,&PNGInfo,&PNGEnd);
	fclose(F);
	return(1);
}




