
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

#include <gimage/gimage.h>
#include "elapsedtime.h"
#include <gimage/strutil.h>

//matchspec: takes a file name and returns the variant part, specified by the file specification
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

//makename: returns a file name constructed using a file specification and the variant string to insert
std::string makename(std::string variant, std::string fspec)
{
	char e[1024];
	char *a, *b;
	strncpy(e,fspec.c_str(), 1023);
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

//gets the file out of a path/file string
std::string getfile(std::string path)
{
	std::size_t found = path.find_last_of("/\\");
	if (found == std::string::npos) return path;
	return path.substr(found+1);
}

//gets the path out of a path/file string
std::string getpath(std::string path)
{
	std::size_t found = path.find_last_of("/\\");
	if (found == std::string::npos) return ".";
	return path.substr(0,found);
}

std::vector<std::string> fileglob(std::string dspec)
{
	std::vector<std::string> glob;
	struct dirent *dp;
	DIR *dirp = opendir(".");
	while ((dp = readdir(dirp)) != NULL)
		if (matchspec(std::string(dp->d_name), dspec) != "")
			glob.push_back(std::string(dp->d_name));
	(void)closedir(dirp);
	return glob;
}


int file_exists (const char *filename)
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

//used to contain a list of corresponding input and output file names,
//constructed from the input and output file specifications
struct fnames {
	std::string infile, outfile;
};

int main (int argc, char **argv) 
{
	char * filename;
	
	//the corresponding input and output file names 
	std::vector<fnames> files;
	
	std::string commandstring;
	
	int c;
	int flags;
	//gImage dib;

	if (argc < 2) {
		//printf("Error: No input file specified.\n");
		printf("Usage: img inputfile [command[:parameters] ...] outputfile\n\n");
		printf("inputfile and output file can have one wildcard each, '*', to process \n");
		printf("multiple files, e.g., 'img *.NEF gamma:2.2 blackwhitepoint *.tif' will\n");
		printf("open all the .NEFs in the current directory, apply gamma and auto black/white \n");
		printf("point correction to each,and save each as the corresponding filename.tif.\n\n");
		printf("Available commands:\n");
		printf("\tbright:[-100 - 100, default=0]\n");
		printf("\tblackwhitepoint[:0-127,128-255 default=auto]\n");
		printf("\tcontrast:[-100 - 100, default=0]\n");
		printf("\tcrop:x,y,w,h\n");
		printf("\tdenoise:[0 - 100.0],[1-10],[1-10], default=0.0,1,3\n");
		printf("\tgamma:[0.0 - 5.0, default=1.0]\n");
		printf("\tgray:r,g,b - values are the fractional (0.x) proportions with which to calculate the neutral value.\n");
		printf("\tresize:[width],[height],[box|bilinear|bspline|bicubic|catmullrom|\n");
		printf("\t\tlanczos3 (default)]\n");
		printf("\trotate:[0 - 45, default=0]\n");
		printf("\tsharpen:[0 - 10, default=0]\n");
		printf("\tsaturation:[0 - 5.0, default=1.0, no change]\n");
		printf("\ttint:r,g,b - add/subtract value from each channel\n\n");
		printf("\twhitebalance:rmult,gmult,bmult\n");
		
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

	//separates the parameters from the input and output file strings
	std::vector<std::string> infile = split(std::string(argv[1]),":");
	if (infile.size() < 2) infile.push_back("");
	std::vector<std::string> outfile = split(std::string(argv[argc-1]),":");
	if (outfile.size() < 2) outfile.push_back("");



	if (countchar(infile[0],'*') == 1) {
		if (countchar(outfile[0],'*') == 1) {
			std::vector<std::string> flist = fileglob(infile[0]);
			for (int i=0; i<flist.size(); i++) {
				std::string variant = matchspec(flist[i], infile[0]);
				if (variant == "") continue;
				fnames f;
				f.infile = flist[i];
				f.outfile = makename(variant,outfile[0]);
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
		f.infile = infile[0];
		f.outfile = outfile[0];
		files.push_back(f);
	}
	

//list of commands to apply to each input file
std::vector<std::string> commands;
for (int i = 2; i<argc-1; i++) {
	commands.push_back(std::string(argv[i]));
	//commandstring.append(" "+std::string(argv[i]));
}

int count = 0;

for (int f=0; f<files.size(); f++)
{
	count++;
	char iname[256];
	strncpy(iname, files[f].infile.c_str(), 255);
	
	if (file_exists(files[f].outfile.c_str())) {
		printf("Output file %s exists, skipping %s...\n",files[f].outfile.c_str(), iname);
		continue;
	}

	printf("%d: Loading file %s %s... ",count, iname, infile[1].c_str());
	_mark();
	gImage dib = gImage::loadImageFile(iname, infile[1]);
	printf("done. (%fsec)\nImage size: %dx%d\n",_duration(), dib.getWidth(),dib.getHeight());
	
	for (int i=0; i<commands.size(); i++) {
		char c[256];
		strncpy(c, commands[i].c_str(), 255);
		char* cmd = strtok(c,":");

		if (strcmp(cmd,"bright") == 0) {  //#bright:[-100 - 100, default=0]
			double bright=0.0;
			char *b = strtok(NULL," ");
			if (b) bright = atof(b);

			Curve ctrlpts;
			ctrlpts.insertpoint(0,0);
			if (bright < 0)
				ctrlpts.insertpoint(255,255+bright);
			else
				ctrlpts.insertpoint(255-bright,255);

			int threadcount=gImage::ThreadCount();
			printf("bright: %0.2f (%d threads)... ",bright,threadcount);

			_mark();
			dib.ApplyToneCurve(ctrlpts.getControlPoints(), threadcount);
			printf("done (%fsec).\n",_duration());
		}

		else if (strcmp(cmd,"blackwhitepoint") == 0) {  //#blackwhitepoint[:0-127,128-255 default=auto]
			double blk, wht;
			char *b = strtok(NULL,",");
			char *w = strtok(NULL,", ");
			wht = 255; blk = 0;
			if (!b) { 	//calculate black and white points from image histogram
				int i;
				std::vector<long> hdata = dib.Histogram();
				long hmax=0;
				for (i=0; i<256; i++) if (hdata[i] > hmax) hmax = hdata[i];
				
				//find the lower threshold, the black point
				for (i=1; i<128; i++) if ((double)hdata[i]/(double)hmax > 0.05) break;
				blk = (double) i;
				
				//find the local max in 250-255:
				int m = 0, l=255;
				for (i=255; i>=250; i--) if (hdata[i]>m) {m = hdata[i]; l = i;}

				//start looking for the white point threshold right after the location of the local max:
				for (i=l-1; i>=128; i--) if ((double)hdata[i]/(double)hmax > 0.05) break;
				wht = (double) i;
			}
			else {
				if (w) wht = atof(w);
				if (b) blk = atof(b);
			}

			Curve ctrlpts;
			ctrlpts.insertpoint(blk,0);
			ctrlpts.insertpoint(wht,255);

			int threadcount=gImage::ThreadCount();
			printf("blackwhitepoint: %0.2f,%0.2f (%d threads)... ",blk,wht,threadcount);

			_mark();
			dib.ApplyToneCurve(ctrlpts.getControlPoints(), threadcount);
			//dib.ApplyToneLine(blk, wht, threadcount);
			printf("done (%fsec).\n",_duration());
			char cs[256];
			sprintf(cs, "%s:%0.0f,%0.0f ",cmd, blk, wht);
		}

		else if (strcmp(cmd,"contrast") == 0) {  //#contrast:[-100 - 100, default=0]
			double contrast=0.0;
			char *c = strtok(NULL," ");
			if (c) contrast = atof(c);

			Curve ctrlpts;
			if (contrast < 0) {
				ctrlpts.insertpoint(0,-contrast);
				ctrlpts.insertpoint(255,255+contrast);
			}
			else {
				ctrlpts.insertpoint(contrast,0);
				ctrlpts.insertpoint(255-contrast,255);
			}

			int threadcount=gImage::ThreadCount();
			printf("contrast: %0.2f (%d threads)... ",contrast,threadcount);

			_mark();
			dib.ApplyToneCurve(ctrlpts.getControlPoints(), threadcount);
			printf("done (%fsec).\n",_duration());
		}

		else if (strcmp(cmd,"gamma") == 0) {  //#gamma:[0.0 - 5.0, default=1.0]
			double gamma=1.0;
			char *g = strtok(NULL," ");
			if (g) gamma = atof(g);

			Curve ctrlpts;
			double exponent = 1 / gamma;
			double v = 255.0 * (double)pow((double)255, -exponent);
			for (int i = 0; i< 256; i+=1) {
				double color = (double)pow((double)i, exponent) * v;
				if (color > 255.0) color = 255.0;
				ctrlpts.insertpoint((double) i, color);
			}	

			int threadcount=gImage::ThreadCount();
			printf("gamma: %0.2f (%d threads)... ",gamma,threadcount);

			_mark();
			dib.ApplyToneCurve(ctrlpts.getControlPoints(), threadcount);
			printf("done (%fsec).\n",_duration());
		}



		else if (strcmp(cmd,"resize") == 0) {  //#resize:x,y      
			unsigned w, h;
			char *wstr = strtok(NULL,", ");
			char *hstr = strtok(NULL," ");
			if (wstr) w = atoi(wstr);
			if (hstr) h = atoi(hstr);
			unsigned dw = dib.getWidth();
			unsigned dh = dib.getHeight();

			if (h ==  0) h = dh * ((float)w/(float)dw);
			if (w == 0)  w = dw * ((float)h/(float)dh); 
			int threadcount=gImage::ThreadCount();
			printf("resize: %dx%d (%d threads)... ",w,h,threadcount);

			_mark();
			dib.ApplyResize(w,h, FILTER_LANCZOS3, threadcount);
			printf("done (%fsec).\n", _duration());
		}

		else if (strcmp(cmd,"rotate") == 0) {  //#rotate:[0 - 45, default=0] 
			double angle=0.0;
			char *s = strtok(NULL," ");
			if (s) angle = atof(s);
			int threadcount=gImage::ThreadCount();
			printf("rotate: %0.2f (%d threads)... ",angle,threadcount);

			_mark();
			dib.ApplyRotate(angle, false, threadcount);
			printf("done (%fsec).\n",_duration());
		}
		
		else if (strcmp(cmd,"sharpen") == 0) {  //#sharpen:[0 - 10, default=0]      //add in else when other commands are uncommented
			double sharp=0.0;
			char *s = strtok(NULL," ");
			if (s) sharp = atof(s);
			int threadcount=gImage::ThreadCount();
			printf("sharp: %0.2f (%d threads)... ",sharp, threadcount);

			_mark();
			dib.ApplySharpen(sharp, threadcount);
			printf("done (%fsec).\n",_duration());
		}

		else if (strcmp(cmd,"crop") == 0) {  //#crop:x,y,w,h      
			unsigned x=0, y=0, width=0, height=0;
			char *xstr = strtok(NULL,", ");
			char *ystr = strtok(NULL,", ");
			char *wstr = strtok(NULL,", ");
			char *hstr = strtok(NULL," ");
			if (xstr) x = atoi(xstr);
			if (ystr) y = atoi(ystr);
			if (wstr) width = atoi(wstr);
			if (hstr) height = atoi(hstr);
			width += x;
			height += y;
			int threadcount=gImage::ThreadCount();
			printf("crop: %d,%d %dx%d (%d threads)... ",x,y,width,height,threadcount);

			_mark();
			dib.ApplyCrop(x,y,width,height,threadcount);
			printf("done (%fsec).\n",_duration());
		}

		else if (strcmp(cmd,"saturation") == 0) {  //#saturation:[0 - 5.0, default=1.0, no change]
			double saturation=0.0;
			char *s = strtok(NULL," ");
			if (s) saturation = atof(s);

			int threadcount=gImage::ThreadCount();
			printf("saturate: %0.2f (%d threads)... ",saturation,threadcount);

			_mark();
			dib.ApplySaturate(saturation, threadcount);
			printf("done (%fsec).\n",_duration());
		}

		else if (strcmp(cmd,"denoise") == 0) {  //#denoise:[0 - 100.0],[1-10],[1-10], default=0.0,1,3
			double sigma=0.0;
			char *s = strtok(NULL," ");
			if (s) sigma = atof(s);

			int threadcount=gImage::ThreadCount();
			printf("denoise: %0.2f (%d threads)... ",sigma,threadcount);

			_mark();
			dib.ApplyNLMeans(sigma, 3, 1, threadcount);
			printf("done (%fsec).\n",_duration());
		}

		else if (strcmp(cmd,"tint") == 0) {  //#tint:r,g,b
			double red=0.0; double green=0.0; double blue = 0.0;
			char *r = strtok(NULL,", ");
			char *g = strtok(NULL,", ");
			char *b = strtok(NULL," ");
			if (r) red = atof(r);
			if (g) green = atof(g);
			if (b) blue = atof(b);

			int threadcount=gImage::ThreadCount();
			printf("tint: %0.2f,%0.2f,%0.2f (%d threads)... ",red,green,blue,threadcount);

			_mark();
			dib.ApplyTint(red,green,blue, threadcount);
			printf("done (%fsec).\n",_duration());
		}
		
		else if (strcmp(cmd,"whitebalance") == 0) {  //#whitebalance:rmult,gmult,bmult
			double redmult=1.0; double greenmult = 1.0; double bluemult = 1.0;
			char *rm = strtok(NULL,", ");
			char *gm = strtok(NULL,", ");
			char *bm = strtok(NULL," ");
			if (rm) redmult = atof(rm);
			if (gm) greenmult = atof(gm);
			if (bm) bluemult = atof(bm);

			int threadcount=gImage::ThreadCount();
			printf("whitebalance: %0.2f,%0.2f,%0.2f (%d threads)... ",redmult,greenmult,bluemult,threadcount);

			_mark();
			dib.ApplyWhiteBalance(redmult,greenmult,bluemult, threadcount);
			printf("done (%fsec).\n",_duration());
		}

		else if (strcmp(cmd,"gray") == 0) {  //#gray:[r],[g],[b] 
			double red=0.21; double green=0.72; double blue = 0.07;
			char *r = strtok(NULL,", ");
			char *g = strtok(NULL,", ");
			char *b = strtok(NULL," ");
			if (r) red = atof(r);
			if (g) green = atof(g);
			if (b) blue = atof(b);

			int threadcount=gImage::ThreadCount();
			printf("gray: %0.2f,%0.2f,%0.2f (%d threads)... ",red,green,blue,threadcount);

			_mark();
			dib.ApplyGray(red,green,blue, threadcount);
			printf("done (%fsec).\n",_duration());
		}
		
		else if (strcmp(cmd,"redeye") == 0) {  //not documented, for testing only
			int limit = 25; double threshold = 1.5;
			char *sx = strtok(NULL,", ");
			char *sy = strtok(NULL,", ");
			char *st = strtok(NULL,", ");
			char *sl = strtok(NULL,", ");
			if (sx) {
				int x = atoi(sx);
				if (sy) {
					if (st) threshold = atof(st);
					if (sl) limit = atoi(sl);
					int y = atoi(sy);
					std::vector<coord> pts;
					struct coord pt; pt.x = x; pt.y = y;
					pts.push_back(pt);
					int threadcount = gImage::ThreadCount();
					printf("redeye (%d threads)... ", threadcount);
					_mark();
					dib.ApplyRedeye(pts, threshold, limit, false, 1.0,  threadcount);
					printf("done (%fsec).\n",_duration());
				}
				else printf("redeye: bad y coord\n");
			}
			else printf("redeye: bad x coord\n");
		
		}
		
		else if (strcmp(cmd,"rotate90") == 0) {
			int threadcount = gImage::ThreadCount();
			printf("rotate90 (%d threads)... ", threadcount);
			_mark();
			dib.ApplyRotate90(threadcount);
			printf("done (%fsec).\n",_duration());
		}
		
		else if (strcmp(cmd,"rotate180") == 0) {
			int threadcount = gImage::ThreadCount();
			printf("rotate180 (%d threads)... ", threadcount);
			_mark();
			dib.ApplyRotate180(threadcount);
			printf("done (%fsec).\n",_duration());
		}
		
		else if (strcmp(cmd,"rotate270") == 0) {
			int threadcount = gImage::ThreadCount();
			printf("rotate270 (%d threads)... ", threadcount);
			_mark();
			dib.ApplyRotate270(threadcount);
			printf("done (%fsec).\n",_duration());
		}

		else printf("Unrecognized command: %s.  Continuing...\n",cmd);

	} //end of processing commands.


	char outfilename[256];
	strncpy(outfilename, files[f].outfile.c_str(), 255);


	if (strcmp(outfilename, "info") == 0) {
		std::map<std::string,std::string> imginfo = dib.getInfo();
		for (std::map<std::string,std::string>::iterator it=imginfo.begin(); it!=imginfo.end(); ++it)
			printf("%s: %s\n",it->first.c_str(), it->second.c_str());
		printf("\n");
		exit(0);
	}


	_mark();
	//printf("Saving file %s %s... ",outfile[0].c_str(), outfile[1].c_str());
	printf("Saving file %s %s... ",outfilename, outfile[1].c_str());
	dib.setInfo("Software","gimg 0.1");
	if (dib.saveImageFile(outfilename, outfile[1].c_str())) 
		printf("done. (%fsec)\n\n",_duration());
	else
		printf("Error: bad output file specification: %s\n\n",outfile[0].c_str());

}


	return 0;
}
