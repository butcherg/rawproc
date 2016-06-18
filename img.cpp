
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "FreeImage.h"

#include "ThreadedCurve.h"

void FreeImage_PrintImageType(FREE_IMAGE_TYPE t)
{
    switch (t) {
        case FIT_UNKNOWN:
            printf("FIT_UNKNOWN: Unknown format (returned value only, never use it as input value)\n");
            break;
        case FIT_BITMAP:
            printf("FIT_BITMAP: Standard image: 1-, 4-, 8-, 16-, 24-, 32-bit\n");
            break;
        case FIT_UINT16:
            printf("FIT_UINT16: Array of unsigned short: unsigned 16-bit\n");
            break;
        case FIT_INT16:
            printf("FIT_INT16: Array of short: signed 16-bit\n");
            break;
        case FIT_UINT32:
            printf("FIT_UINT32: Array of unsigned long: unsigned 32-bit\n");
            break;
        case FIT_INT32:
            printf("FIT_INT32: Array of long: signed 32-bit\n");
            break;
        case FIT_FLOAT:
            printf("FIT_FLOAT: Array of float: 32-bit IEEE floating point\n");
            break;
        case FIT_DOUBLE:
            printf("FIT_DOUBLE Array of double: 64-bit IEEE floating point\n");
            break;
        case FIT_COMPLEX:
            printf("FIT_COMPLEX Array of FICOMPLEX: 2 x 64-bit IEEE floating point\n");
            break;
        case FIT_RGB16:
            printf("FIT_RGB16 48-bit RGB image: 3 x unsigned 16-bit\n");
            break;
        case FIT_RGBA16:
            printf("FIT_RGBA16 64-bit RGBA image: 4 x unsigned 16-bit\n");
            break;
        case FIT_RGBF:
            printf("FIT_RGBF 96-bit RGB float image: 3 x 32-bit IEEE floating point\n");
            break;
        case FIT_RGBAF:
            printf("FIT_RGBAF 128-bit RGBA float image: 4 x 32-bit IEEE floating point\n");
            break;
    }
}

int file_exists (char *filename)
{
	struct stat   buffer;   
	return (stat (filename, &buffer) == 0);
}

int main (int argc, char *argv[]) 
{
	char * filename;
	int c;
	int flags = 0;
	FIBITMAP *dib;
	FREE_IMAGE_FORMAT fif;

	if (!file_exists(argv[1])) {
		printf("Error: No file specified.\n");
		exit(1);
	}

	fif = FreeImage_GetFileType(argv[1], 0);
	if(fif != FIF_UNKNOWN) {
		// load from the file handle
		printf("Loading file %s... ",argv[1]);
		dib = FreeImage_Load(fif, argv[1], flags);
	}
	else {
		printf("Error: Unknown file type.\n");
		exit(1);
	}


	FreeImage_PrintImageType(FreeImage_GetImageType(dib));
    
	int bpp = FreeImage_GetBPP(dib);

	for (int i = 2; i<argc-1; i++) {
		char* cmd=strtok(argv[i],":");
        
		if (strcmp(cmd,"bright") == 0) {  //#bright:[-100 - 100, default=0]
			double bright=atof(strtok(NULL," "));
			int bpp = FreeImage_GetBPP(dib);
			printf("brightness: %0.2f (%dbpp)... ",bright,bpp);

			Curve ctrlpts;
			ctrlpts.insertpoint(0,0);
			if (bright < 0)
				ctrlpts.insertpoint(255,255+bright);
			else
				ctrlpts.insertpoint(255-bright,255);

	int threadcount;
		}

	}

	return 0;
}