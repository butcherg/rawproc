
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <vector>
#include <string>
#include <iostream>
#include <dirent.h>

#include "FreeImage.h"
#include "FreeImage_Threaded.h"


std::string matchspec(std::string fname, std::string fspec)
{
	if (fname.size() < fspec.size()) return "";
	std::string specf, specb;
	std::string fnamef, fnameb;
	std::string variant;
	int wp = fspec.find_first_of("*");
	specf = fspec.substr(0,wp);
	specb = fspec.substr(wp+1,fspec.size()-1);
	fnamef = fname.substr(0,wp);
	fnameb = fname.substr(fname.size()-specb.size(), fname.size()-1);
	variant = fname.substr(wp,fname.size()-fspec.size()+1);
	if ((specf.compare(fnamef) == 0) & (specb.compare(fnameb) == 0)) 
		return variant;
	return "";
}

std::string makename(std::string variant, std::string endstr)
{
	char e[1024];
	char *a, *b;
	strncpy(e,endstr.c_str(), 1023);
	if (e[0] == '*') {
		b = NULL;
		a = e+1;
	}
	else {
		b = strtok(e,"*");
		a = strtok(NULL,"*");
	}
	std::string m;
	if (b) m.append(b);
	m.append(variant);
	if (a) m.append(a);
	return m;
}

int countchar(std::string s, char c)
{
	int count = 0;
	for (int i=0; i<s.size(); i++) {
		if (s[i] == c) count++;
	}
	return count;
}

std::vector<std::string> filelist(std::string dspec)
{
	std::vector<std::string> flist;
	DIR *dir;
	struct dirent *ent;
	if ((dir = opendir(dspec.c_str())) != NULL) {
		while ((ent = readdir(dir)) != NULL) {
			flist.push_back(ent->d_name);
		}
		closedir(dir);
	}
	return flist;
}


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
            printf("FIT_DOUBLE: Array of double: 64-bit IEEE floating point\n");
            break;
        case FIT_COMPLEX:
            printf("FIT_COMPLEX: Array of FICOMPLEX: 2 x 64-bit IEEE floating point\n");
            break;
        case FIT_RGB16:
            printf("FIT_RGB16: 48-bit RGB image: 3 x unsigned 16-bit\n");
            break;
        case FIT_RGBA16:
            printf("FIT_RGBA16: 64-bit RGBA image: 4 x unsigned 16-bit\n");
            break;
        case FIT_RGBF:
            printf("FIT_RGBF: 96-bit RGB float image: 3 x 32-bit IEEE floating point\n");
            break;
        case FIT_RGBAF:
            printf("FIT_RGBAF: 128-bit RGBA float image: 4 x 32-bit IEEE floating point\n");
            break;
    }
}

int file_exists (char *filename)
{
	struct stat   buffer;   
	return (stat (filename, &buffer) == 0);
}

int is_regular_file(const char *path)
{
    struct stat path_stat;
    stat(path, &path_stat);
    return S_ISREG(path_stat.st_mode);
}

void strappend(char* s, char c)
{
        int len = strlen(s);
        s[len] = c;
        s[len+1] = '\0';
}

int _CRT_glob = 0;

struct fnames {
	std::string infile, outfile;
};

int main (int argc, char **argv) 
{
	char * filename;
	std::vector<fnames> files;
	int c;
	int flags;
	FIBITMAP *dib;
	FREE_IMAGE_FORMAT fif;

	if (argc < 2) {
		//printf("Error: No input file specified.\n");
		printf("Usage: img inputfile [command[:parameters] ...] outputfile\n\n");
		printf("inputfile and output file can have one wildcard each, '*', to process multiple files,\n");
		printf("e.g., 'img *.NEF gamma:2.2 blackwhitepoint *.tif' will open all the .NEFs in the \n");
		printf("current directory, apply gamma and auto black/white point correction to each,\n");
		printf("and save each as the corresponding filename.tif.\n\n");
		printf("Available commands:\n");
		printf("\tbright:[-100 - 100, default=0]\n");
                printf("\tblackwhitepoint[:0-127,128-255 default=auto]\n");
                printf("\tcontrast:[-100 - 100, default=0]\n");
                printf("\tgamma:[0.0 - 5.0, default=1.0]\n");
                printf("\tresize:[width],[height],[box|bilinear|bspline|bicubic|catmullrom|lanczos3 (default)]\n");
                printf("\tsharpen:[0 - 10, default=0]\n");
                printf("\tsaturation:[0 - 5.0, default=1.0, no change]\n");
                printf("\tgray (no parameters ,uses the saturate algorithm to desaturate, 0.0)\n");
		exit(1);
	}

	if (argc < 3) {
		printf("Error: No output file specified.\n");
		exit(1);
	}

	if (countchar(std::string(argv[1]),'*') > 1) {
		printf("Error: Too many wildcards in input filespec.\n");
		exit(1);
	}
	if (countchar(std::string(argv[argc-1]),'*') > 1) {
		printf("Error: Too many wildcards in output filespec.\n");
		exit(1);
	}

	if (countchar(std::string(argv[1]),'*') == 1) {
		if (countchar(std::string(argv[argc-1]),'*') == 1) {
			std::vector<std::string> flist = filelist(".");
			for (int i=0; i<flist.size(); i++) {
				std::string variant = matchspec(flist[i], std::string(argv[1]));
				matchspec(flist[i], std::string(argv[1]));
				if (variant == "") continue;
				fnames f;
				f.infile = flist[i];
				f.outfile = makename(variant,argv[argc-1]);
				files.push_back(f);
			}
				
		}
		else {
			printf("Error: If input file has a wildcard spec, the output file should have one also.\n");
			exit(1);
		}

	}
	else {
		fnames f;
		f.infile = argv[1];
		f.outfile = argv[argc-1];
		files.push_back(f);
	}
	

std::vector<std::string> commands;
for (int i = 2; i<argc-1; i++) {
	commands.push_back(std::string(argv[i]));
}



for (int f=0; f<files.size(); f++)
{
	char fname[256];
	strncpy(fname, files[f].infile.c_str(), 255);

	fif = FreeImage_GetFileType(fname, 0);
	if(fif != FIF_UNKNOWN) {
		// load from the file handle
		printf("Loading file %s... \n",fname);
		flags = 0;
		dib = FreeImage_Load(fif, fname, flags);
		int dw = FreeImage_GetWidth(dib);
		int dh = FreeImage_GetHeight(dib);
		printf("Image size: %dx%d\n",dw,dh);
	}
	else {
		printf("Error: Unknown file type.\n");
		exit(1);
	}


	FreeImage_PrintImageType(FreeImage_GetImageType(dib));
	//int bpp = FreeImage_GetBPP(dib);

	for (int i=0; i<commands.size(); i++) {
		char c[256];
		strncpy(c, commands[i].c_str(), 255);
		char* cmd = strtok(c,":");
        
		if (strcmp(cmd,"bright") == 0) {  //#bright:[-100 - 100, default=0]
			double bright=0.0;
			char *b = strtok(NULL," ");
			if (b) bright = atof(b);
			int bpp = FreeImage_GetBPP(dib);
			printf("brightness: %0.2f (%dbpp)... ",bright,bpp);

			Curve ctrlpts;
			ctrlpts.insertpoint(0,0);
			if (bright < 0)
				ctrlpts.insertpoint(255,255+bright);
			else
				ctrlpts.insertpoint(255-bright,255);

			int threadcount=0;
			FIBITMAP *dst = FreeImage_Clone(dib);
			double d = ApplyCurve(dib, dst, ctrlpts.getControlPoints(), threadcount);
			FreeImage_Unload(dib);
			dib = dst;
			printf("done (%f).\n",d);
		}

		else if (strcmp(cmd,"blackwhitepoint") == 0) {  //#blackwhitepoint[:0-127,128-255 default=auto]
			double blk, wht;
			char *b = strtok(NULL,",");
			char *w = strtok(NULL,", ");
			wht = 255; blk = 0;
			if (!b) {
				int i;
				DWORD hdata[256]; DWORD hmax=0;
				FIBITMAP *hdib = FreeImage_ConvertTo24Bits(dib);
				FreeImage_GetHistogram(hdib, hdata);
				FreeImage_Unload(hdib);
				for (i=0; i<256; i++) if (hdata[i] > hmax) hmax = hdata[i];
				for (i=1; i<128; i++) if ((double)hdata[i]/(double)hmax > 0.05) break;
				blk = (double) i;
				for (i=255; i>=128; i--) if ((double)hdata[i]/(double)hmax > 0.05) break;
				wht = (double) i;
			}
			else {


				if (w) wht = atof(w);
				if (b) blk = atof(b);
			}

			int bpp = FreeImage_GetBPP(dib);
			printf("blackwhitepoint: %0.2f,%0.2f (%dbpp)... ",blk,wht,bpp);

			Curve ctrlpts;
			ctrlpts.insertpoint(blk,0);
			ctrlpts.insertpoint(wht,255);

			int threadcount=0;
			FIBITMAP *dst = FreeImage_Clone(dib);
			double d = ApplyCurve(dib, dst, ctrlpts.getControlPoints(), threadcount);
			FreeImage_Unload(dib);
			dib = dst;
			printf("done (%f).\n",d);
		}

		else if (strcmp(cmd,"contrast") == 0) {  //#contrast:[-100 - 100, default=0]
			double contrast=0.0;
			char *c = strtok(NULL," ");
			if (c) contrast = atof(c);
			int bpp = FreeImage_GetBPP(dib);
			printf("contrast: %0.2f (%dbpp)... ",contrast,bpp);

			Curve ctrlpts;
			if (contrast < 0) {
				ctrlpts.insertpoint(0,-contrast);
				ctrlpts.insertpoint(255,255+contrast);
			}
			else {
				ctrlpts.insertpoint(contrast,0);
				ctrlpts.insertpoint(255-contrast,255);
			}

			int threadcount=0;
			FIBITMAP *dst = FreeImage_Clone(dib);
			double d = ApplyCurve(dib, dst, ctrlpts.getControlPoints(), threadcount);
			FreeImage_Unload(dib);
			dib = dst;
			printf("done (%f).\n",d);
		}

		else if (strcmp(cmd,"gamma") == 0) {  //#gamma:[0.0 - 5.0, default=1.0]
			double d;
			double gamma=1.0;
			char *g = strtok(NULL," ");
			if (g) gamma = atof(g);
			int bpp = FreeImage_GetBPP(dib);
			printf("gamma: %0.1f (%dbpp)... ",gamma,bpp);

			BYTE LUT8[256];
			WORD LUT16[65536];
			FIBITMAP *dst = FreeImage_Clone(dib);
			int threadcount=0;

			if (bpp == 24) {
				double exponent = 1 / gamma;
				double v = 255.0 * (double)pow((double)255, -exponent);
				for(int i = 0; i < 256; i++) {
					double color = (double)pow((double)i, exponent) * v;
					if(color > 255) color = 255;
					LUT8[i] = (BYTE)floor(color + 0.5);
				}
				d = ApplyLUT(dib, dst, (char *)LUT8, threadcount);
			}
			else if(bpp == 48) {
				double exponent = 1 / gamma;
				double v = 65535.0 * (double)pow((double)65535, -exponent);
				for(int i = 0; i < 65536; i++) {
					double color = (double)pow((double)i, exponent) * v;
					if(color > 65535)color = 65535;
					LUT16[i] = (WORD)floor(color + 0.5);
				}
				d = ApplyLUT(dib, dst, (char *)LUT16, threadcount);
			}

			FreeImage_Unload(dib);
			dib = dst;
			printf("done (%f).\n",d);
		}

		else if (strcmp(cmd,"resize") == 0) {  //#resize:[width],[height],[box|bilinear|bspline|bicubic|catmullrom|lanczos3 (default)]
			char *w = strtok(NULL,",");
			char *h = strtok(NULL,", ");
			char *f = strtok(NULL," ");
			char algo[30];
			if (f)
				strncpy(algo,f,29);
			else
				strncpy(algo,"lanczos3",29);
			int width = atoi(w);
			int height = atoi(h);
			unsigned dw = FreeImage_GetWidth(dib);
			unsigned dh = FreeImage_GetHeight(dib);
			float pct;
			if (height ==  0) height = dh * ((float)width/(float)dw);
			if (width == 0)  width = dw * ((float)height/(float)dh); 
			printf("resize: from %dx%d to %dx%d, filter: %s... ",dw,dh,width,height,algo);
			FREE_IMAGE_FILTER filter;
			if (strcmp(algo,"box") == 0) filter = FILTER_BOX;
			if (strcmp(algo,"bilinear") == 0) filter = FILTER_BILINEAR;
			if (strcmp(algo,"bspline") == 0) filter = FILTER_BSPLINE;
			if (strcmp(algo,"bicubic") == 0) filter = FILTER_BICUBIC;
			if (strcmp(algo,"catmullrom") == 0) filter = FILTER_CATMULLROM;
			if (strcmp(algo,"lanczos3") == 0) filter = FILTER_LANCZOS3;
			FIBITMAP *dest = FreeImage_Rescale(dib,width,height,filter);
			if (dest) {
				FreeImage_Unload(dib);
				dib = dest;
				printf("done.\n");
			}
			else {
				printf("failed.\n");
			}
		}

		else if (strcmp(cmd,"sharpen") == 0) {  //#sharpen:[0 - 10, default=0]
			double sharp=0.0;
			char *s = strtok(NULL," ");
			if (s) sharp = atof(s);
			int bpp = FreeImage_GetBPP(dib);
			printf("sharp: %0.2f (%dbpp)... ",sharp,bpp);

			double kernel[3][3] =
			{
				0.0, 0.0, 0.0,
				0.0, 0.0, 0.0,
				0.0, 0.0, 0.0
			};
			double x = -((sharp)/4.0);
			kernel[0][1] = x;
			kernel[1][0] = x;
			kernel[1][2] = x;
			kernel[2][1] = x;
			kernel[1][1] = sharp+1;

			int threadcount=0;

			FIBITMAP *dst = FreeImage_Clone(dib);
			double d = ApplyKernel(dib, dst, kernel, threadcount);
			FreeImage_Unload(dib);
			dib = dst;
			printf("done (%f).\n",d);
		}

		else if (strcmp(cmd,"saturation") == 0) {  //#saturation:[0 - 5.0, default=1.0, no change]
			double d;
			double saturation=1.0;
			char *s = strtok(NULL," ");
			if (s) saturation = atof(s);
			int bpp = FreeImage_GetBPP(dib);
			printf("saturation: %0.1f (%dbpp)... ",saturation,bpp);

			int threadcount=0;

			FIBITMAP *dst = FreeImage_Clone(dib);
			d = ApplySaturation(dib, dst, saturation, threadcount);
			FreeImage_Unload(dib);
			dib = dst;
			printf("done (%f).\n",d);
		}

		else if (strcmp(cmd,"gray") == 0) {  //#gray (no parameters ,uses the saturate algorithm to desaturate)
			double d;
			double saturation=0.0;
			//char *s = strtok(NULL," ");
			//if (s) saturation = atof(s);
			int bpp = FreeImage_GetBPP(dib);
			printf("gray: (%dbpp)... ",bpp);

			int threadcount=0;

			FIBITMAP *dst = FreeImage_Clone(dib);
			d = ApplySaturation(dib, dst, saturation, threadcount);
			FreeImage_Unload(dib);
			dib = dst;
			printf("done (%f).\n",d);
		}

		else printf("Unrecognized command: %s.  Continuing...\n",cmd);



	} //end of processing commands.


	char outfilename[256];
	strncpy(outfilename, files[f].outfile.c_str(), 255);
			
	flags = 100;
	const char *output_filename = strtok(outfilename,":");
	const char *fl = strtok(NULL," ");
	if (fl) flags = atoi(fl);

	FREE_IMAGE_FORMAT out_fif = FreeImage_GetFIFFromFilename(output_filename);

	if(out_fif != FIF_UNKNOWN) {
		if(out_fif == FIF_JPEG) {
			if(FreeImage_GetBPP(dib) != 24) {
				FIBITMAP *dst = FreeImage_ConvertTo24Bits(dib);
				FreeImage_Unload(dib);
				dib = dst;
			}
		}
		printf("Saving file %s...\n\n",output_filename);
		FreeImage_Save(out_fif, dib, output_filename, flags);
	}
	else {
		printf("Error: bad output file specification:\n",output_filename);
	}

	FreeImage_Unload(dib);

}


	return 0;
}