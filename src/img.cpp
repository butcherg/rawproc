
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <algorithm>
#include <vector>
#include <string>
#include <iostream>
#include <dirent.h>
#include <math.h> 

#include "gimage/gimage.h"
#include "elapsedtime.h"
#include "gimage/strutil.h"
#include "fileutil.h"
#include "myConfig.h"
#include "CameraData.h"
#include "gimage_cmd.h"

std::string param_string(std::string filter)
{
	std::string paramstr, name, val;

	std::map<std::string, std::string> c = myConfig::getConfig().getDefault();
	for (std::map<std::string, std::string>::iterator it=c.begin(); it!=c.end(); ++it) {
		name = it->first;
		val =  it->second;
		if (name.find(filter) != std::string::npos) {
			val = myConfig::getConfig().getValue(name);
			name = name.substr(name.find_last_of(".")+1, name.size());
			if (val != "") paramstr.append(string_format("%s=%s;",name.c_str(), val.c_str()));
		}
	}

	return paramstr;
}


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
	std::string path = getpath(dspec).c_str();
	std::string filepattern = getfile(dspec).c_str();
	DIR *dirp = opendir(path.c_str());
	while ((dp = readdir(dirp)) != NULL) 
		if (matchspec(std::string(dp->d_name), filepattern) != "") 
			glob.push_back(std::string(dp->d_name));
	(void)closedir(dirp);
	std::sort( glob.begin(), glob.end() );
	return glob;
}

std::vector<std::string> variantglob(std::string dspec)
{
	std::vector<std::string> glob;
	struct dirent *dp;
	std::string path = getpath(dspec).c_str();
	std::string filepattern = getfile(dspec).c_str();
	std::string variantsuffix = filepattern.substr(1);

	DIR *dirp = opendir(path.c_str());
	while ((dp = readdir(dirp)) != NULL) {
		std::string variant = matchspec(std::string(dp->d_name), filepattern);
		if (variant != "") {
			glob.push_back(variant);
		}
	}
	(void)closedir(dirp);
	std::sort( glob.begin(), glob.end() );
	return glob;
}

/*
int file_exists (const char *filename)
{
	struct stat   buffer;   
	return (stat (filename, &buffer) == 0);
}
*/

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
bool force = false;

bool saveFile (gImage &savedib, std::string outfilename, std::string params, std::string commandstring, bool verbose)
{

	gImage dib = savedib;
	GIMAGE_FILETYPE filetype = gImage::getFileNameType(outfilename.c_str());
	
	std::string profilepath = myConfig::getConfig().getValueOrDefault("cms.profilepath","");
	if (profilepath[profilepath.length()-1] != '/') profilepath.push_back('/');
	
	std::string intentstr;
	cmsUInt32Number intent = INTENT_PERCEPTUAL;

	if (filetype == FILETYPE_JPEG) {
		profilepath.append(myConfig::getConfig().getValueOrDefault("output.jpeg.cms.profile",""));
		intentstr = myConfig::getConfig().getValueOrDefault("output.jpeg.cms.renderingintent","perceptual");
		if (params.empty()) params = myConfig::getConfig().getValueOrDefault("output.jpeg.parameters","quality=95");
	}
	else if (filetype == FILETYPE_TIFF) {
		profilepath.append(myConfig::getConfig().getValueOrDefault("output.tiff.cms.profile",""));
		intentstr = myConfig::getConfig().getValueOrDefault("output.tiff.cms.renderingintent","perceptual");
		if (params.empty()) params = myConfig::getConfig().getValueOrDefault("output.tiff.parameters","");
	}
	else if (filetype == FILETYPE_PNG) {
		profilepath.append(myConfig::getConfig().getValueOrDefault("output.png.cms.profile",""));
		intentstr = myConfig::getConfig().getValueOrDefault("output.png.cms.renderingintent","perceptual");
		if (params.empty()) params= myConfig::getConfig().getValueOrDefault("output.png.parameters","");
	}
				
	if (intentstr == "perceptual") intent = INTENT_PERCEPTUAL;
	if (intentstr == "saturation") intent = INTENT_SATURATION;
	if (intentstr == "relative_colorimetric") intent = INTENT_RELATIVE_COLORIMETRIC;
	if (intentstr == "absolute_colorimetric") intent = INTENT_ABSOLUTE_COLORIMETRIC;

	_mark();
	//printf("Saving file %s %s... ",outfile[0].c_str(), outfile[1].c_str());
	if (verbose) { printf("Saving the file %s %s... ",outfilename.c_str(), params.c_str()); fflush(stdout); }
	dib.setInfo("Software","img 0.1");
	dib.setInfo("ImageDescription", commandstring);
	
	cmsHPROFILE profile = cmsOpenProfileFromFile(profilepath.c_str(), "r");
	if (dib.getProfile() && profile)
		dib.saveImageFile(outfilename.c_str(), params.c_str(), profile, intent);
	else
		dib.saveImageFile(outfilename.c_str(), params.c_str());

	if (dib.getLastError() == GIMAGE_OK) {
		if (verbose) printf("done. (%fsec)\n",_duration());
		return true;
	}
	else {
		printf("Error: %s: %s\n",dib.getLastErrorMessage().c_str(), outfilename.c_str());
		return false;
	}
}

//used to contain a list of corresponding input and output file names,
//constructed from the input and output file specifications
struct fnames {
	std::string infile, outfile, variant;
};

std::string commandstring;



//https://github.com/ccxvii/asstools/blob/master/getopt.c
/*
 * This is a version of the public domain getopt implementation by
 * Henry Spencer originally posted to net.sources.
 *
 * This file is in the public domain.
 */


#define getopt xgetopt
#define optarg xoptarg
#define optind xoptind

char *optarg; /* Global argument pointer. */
int optind = 0; /* Global argv index. */

static char *scan = NULL; /* Private scan pointer. */

int
getopt(int argc, char *argv[], char *optstring)
{
	char c;
	char *place;

	optarg = NULL;

	if (!scan || *scan == '\0') {
		if (optind == 0)
			optind++;

		if (optind >= argc || argv[optind][0] != '-' || argv[optind][1] == '\0')
			return EOF;
		if (argv[optind][1] == '-' && argv[optind][2] == '\0') {
			optind++;
			return EOF;
		}

		scan = argv[optind]+1;
		optind++;
	}

	c = *scan++;
	place = strchr(optstring, c);

	if (!place || c == ':') {
		fprintf(stderr, "%s: unknown option -%c\n", argv[0], c);
		return '?';
	}

	place++;
	if (*place == ':') {
		if (*scan != '\0') {
			optarg = scan;
			scan = NULL;
		} else if( optind < argc ) {
			optarg = argv[optind];
			optind++;
		} else {
			fprintf(stderr, "%s: option requires argument -%c\n", argv[0], c);
			return ':';
		}
	}

	return c;
}

//end getopt.c

std::string getFileType(std::string filename)
{
	size_t pos = filename.find_last_of(".");
	if (pos = std::string::npos) return "";
	std::string ext = filename.substr(pos);
	if (ext == "jpg" | ext == "jpeg" | ext == "JPG" | ext == "JPEG") return "jpeg";
	if (ext == "tif" | ext == "tiff" | ext == "TIF" | ext == "TIFF") return "tiff";
	if (ext == "png" | ext == "PNG") return "png";
	return "raw";
}



int main (int argc, char **argv) 
{
	char * filename;

	std::string conffile;
	bool noconf = false;
	bool verbose = false;
	int f;
	//opterr = 0;

	while ((f = getopt(argc, argv, (char *) "fnvc:")) != -1)
		switch(f) {
			case 'f':  //force processing even if output file exists
				force = true;
				break;
			case 'n': //no config file is used
				noconf = true;
				break;
			case 'v': //verbose output
				verbose = true;
				break;
			case 'c': //use the specified config file
				conffile = std::string(optarg);
				break;
			case '?':
				exit(-1);
			default:
				exit(-1);

		}
	
	//the corresponding input and output file names 
	std::vector<fnames> files;
	
	int c;
	int flags;
	//gImage dib;
	
	if (!noconf) {
		std::string configfile = getRawprocConfPath(conffile);
		if (configfile != "(none)") myConfig::loadConfig(configfile);
		printf("configuration file: %s\n", configfile.c_str());
	}

	gImage::setProfilePath(filepath_normalize(myConfig::getConfig().getValueOrDefault("cms.profilepath",getCwd())));

	#ifdef USE_DCRAW
	gImage::setdcrawPath(filepath_normalize(myConfig::getConfig().getValueOrDefault("input.raw.dcraw.path","dcraw")));
	#endif

	if (argc < 2) {
		//printf("Error: No input file specified.\n");
		#ifdef VERSION
		printf("img version: %s build date: %s\n",VERSION, BUILDDATE);
		#else
		printf("img build date: %s\n", BUILDDATE);
		#endif
		printf("Usage: img inputfile [command[:parameters] ...] outputfile\n\n");
		printf("inputfile and output file can have one wildcard each, '*', to process \n");
		printf("multiple files, e.g., 'img *.NEF gamma:2.2 blackwhitepoint *.tif' will\n");
		printf("open all the .NEFs in the current directory, apply gamma and auto black/white \n");
		printf("point correction to each,and save each as the corresponding filename.tif.\n\n");
		printf("Available commands:\n");

		printf("\tcolorspace:profilefile[,convert|assign][,renderingintent][,bpc]\n");
		printf("\tbright:[-100 - 100] default: 0 (no-bright)\n");
		printf("\tdemosaic:[half|half_resize|color|vng|amaze|dcb|rcd|igv|lmmse|ahd|\n\t\txtrans_fast|xtrans_markesteijn] default: ahd\n");
		printf("\taddexif:tagname,value - tagname must be valid EXIF tag for it\n\t\t to survive the file save...\n");
		printf("\tblackwhitepoint[:rgb|red|green|blue][,0-127,128-255] \n\t\tdefault: auto blackwhitepoint determination. The \n\t\tcalculated points will be used in the metafile entry.\n");
		printf("\tcontrast:[-100 - 100] default: 0 (no-contrast)\n");
		printf("\tgamma:[0.0-5.0] default: 1.0 (linear, or no-gamma)\n");
		printf("\tresize:w[,h] If either w or h is 0, resize preserves aspect of\n\t\t that dimension.  If only one number is present, the image is\n\t\t resized to that number along the longest dimension,\n\t\tpreserving aspect.\n");
		printf("\trotate:[-45.0 - 45.0] default: 0 (no-rotate)\n");
		printf("\tsharpen:[0 - 10, default: 0 (no-sharpen)\n");
		printf("\tcrop:x,y,w,y  no defaults\n");
		printf("\tsaturation:[0 - 5.0] default=1.0, no change\n");
		printf("\tdenoise:[0 - 100.0],[1-10],[1-10], default=0.0,1,3\n");
		printf("\ttint:[r,g,b] default: 0,0,0 (doesn't have a corresponding\n\t\t tool in rawproc)\n");
		printf("\twhitebalance:[auto]|[patch]|[camera]|[rmult,gmult,bmult] \n\t\tdefault: auto, based on 'gray world'\n");
		printf("\tgray:[r,g,b] default: 0.21,0.72,0.07\n"); 
		printf("\tcurve:[rgb,|red,|green,|blue,]x1,y1,x2,y2,...xn,yn  Default channel: rgb\n");
		printf("\texposure:ev default: 1.0\n");
		printf("\thighlight:1-10\n");
		printf("\tshadow:1-10\n");
		printf("\trotate90 - rotate 90 degrees clockwise\n");
		printf("\trotate180 - rotate 180 degrees\n");
		printf("\trotate270 - rotate 270 degrees clockwise\n");
		printf("\thmirror - flip horizontal\n");
		printf("\tvmirror - flip upside down\n");

		exit(1);
	}

	if (argc < 3) {
		printf("Error: No output file specified.\n");
		exit(1);
	}

	if (countchar(std::string(argv[optind]),'*') > 1) {
		printf("Error: Too many wildcards in input filespec.\n");
		exit(1);
	}
	if (countchar(std::string(argv[argc-1]),'*') > 1) {
		printf("Error: Too many wildcards in output filespec.\n");
		exit(1);
	}



	//separates the parameters from the input and output file strings
	std::vector<std::string> infile;
	std::string infilename = std::string(argv[optind]);

#ifdef WIN32
	if (infilename.find_first_of(':') == 1) { 	
		if (std::count(infilename.begin(), infilename.end(), ':') > 1)
			infile = bifurcate(infilename, ':', true);
		else 
			infile.push_back(infilename); 
	}
	else { 
		infile = bifurcate(infilename, ':');
	}
#else
	infile = bifurcate(infilename, ':');
#endif


	if (infile.size() < 2) infile.push_back("");
	optind++;


	std::vector<std::string> outfile;
	std::string outfilename = std::string(std::string(argv[argc-1]));

#ifdef WIN32
	if (outfilename.find_first_of(':') == 1) { 	
		if (std::count(outfilename.begin(), outfilename.end(), ':') > 1)
			outfile = bifurcate(outfilename, ':', true);
		else 
			outfile.push_back(outfilename); 
	}
	else { 
		outfile = bifurcate(outfilename, ':');
	}
#else
	outfile = bifurcate(outfilename, ':');
#endif

	if (outfile.size() < 2) outfile.push_back("");



	std::string filetype = getFileType(infile[0]);

	//construct input parameters from input.filetype.parameters, input.raw.libraw.* if raw, 
	//and the command line parameters in that order.  If raw and rawdata=1, paramlist is
	//truncted to rawdata and cameraprofile
	std::map<std::string,std::string> inputparams;
	if (myConfig::getConfig().exists("input."+filetype+".parameters"))
		parseparams(inputparams, myConfig::getConfig().getValue("input."+filetype+".parameters"));
	if (filetype == "raw")
		parseparams(inputparams, param_string("input.raw.libraw."));
	if (infile[1] != "")
		parseparams(inputparams, infile[1]);
	if (filetype == "raw") {
		if (inputparams.find("rawdata") != inputparams.end()) {
			if (inputparams["rawdata"] == "1") {
				infile[1] = "rawdata=1";
				if (inputparams.find("cameraprofile") != inputparams.end())
					infile[1].append(";cameraprofile="+inputparams["cameraprofile"]);
			}
			else infile[1] = paramstring(inputparams);
		}
	}
	else 
		infile[1] = paramstring(inputparams);


	if (countchar(infile[0],'*') == 1) {
		if (countchar(outfile[0],'*') == 1) {
			std::vector<std::string> vlist = variantglob(infile[0]);
			for (int i=0; i<vlist.size(); i++) {
				std::string variant = vlist[i];
				fnames f;
				f.infile = makename(variant,infile[0]);
				f.outfile = makename(variant,outfile[0]);
				f.variant = variant;
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
for (int i = optind; i<argc-1; i++) {
	commands.push_back(std::string(argv[i]));
}

int alreadyexist = 0;
if (!force) {
	for (int f=0; f<files.size(); f++) {
		if (file_exists(files[f].outfile.c_str())) alreadyexist++;
	}
}
int toprocess = files.size() - alreadyexist;


printf("number of input files to be processed: %d (%d)\n", toprocess, verbose);
if (!force)  printf("number of output files that already exist, skipping: %d\n",alreadyexist);
printf("\n"); fflush(stdout);

int count = 0;
for (int f=0; f<files.size(); f++)
{

	char iname[256];
	strncpy(iname, files[f].infile.c_str(), 255);
	
	if (!force && file_exists(files[f].outfile.c_str())) {
		//printf("%d/%d: Output file %s exists, skipping %s...\n",f+1, toprocess, files[f].outfile.c_str(), iname);
		continue;
	}
	
	if (!file_exists(iname)) {
		printf("%d/%d: Input file %s doesn't exist...\n",f+1, toprocess, iname);
		continue;
	}

	count++;

	commandstring = "rawproc-img ";

	if (verbose) {
		printf("%d/%d: Loading file %s %s... ",count, toprocess, iname, infile[1].c_str()); fflush(stdout);
	}
	else printf("%d/%d: %s\n",count, toprocess, iname); fflush(stdout);
	
	_mark();
	gImage dib = gImage::loadImageFile(iname, infile[1]);
	if (dib.getWidth() == 0 | dib.getHeight() == 0) {
		printf("error: (%fsec) - Image not loaded correctly\n",_duration()); fflush(stdout);
		continue;
	}
	if (verbose) printf("done. (%fsec)\nImage size: %dx%d\n",_duration(), dib.getWidth(),dib.getHeight()); fflush(stdout);

	commandstring += getfile(std::string(iname));
	if (infile[1] != "") commandstring += ":" + infile[1];
	commandstring += " ";


	
	//process commands:
	for (int i=0; i<commands.size(); i++) {
		if (commands[i] == "input.raw.default") {
			std::vector<std::string> cmdlist = split(myConfig::getConfig().getValue("input.raw.default"), " ");
			for (unsigned i=0; i<cmdlist.size(); i++) do_cmd(dib, cmdlist[i], files[f].variant, verbose);
		}
		else {
			std::string cmdstr = do_cmd(dib, commands[i], files[f].variant, verbose);
			fflush(stdout);
			if (cmdstr.find("Error")  != std::string::npos) {
				//printf("%s\n",cmdstr.c_str());
				exit(1);
			}
			else commandstring += cmdstr;
		}
	}

	int orientation = atoi(dib.getInfoValue("Orientation").c_str());
	//printf("Orientation: %d\n", orientation);
	if (orientation != 1) {
		if (verbose) printf("Normalizing image orientation from %d...",orientation); fflush(stdout);
		_mark();
		dib.NormalizeRotation();
		if (verbose) printf("done. (%fsec)\n",_duration()); fflush(stdout);
	}

	char outfilename[256];
	strncpy(outfilename, files[f].outfile.c_str(), 255);


	if (strcmp(outfilename, "info") == 0) {
		std::map<std::string,std::string> imginfo = dib.getInfo();
		for (std::map<std::string,std::string>::iterator it=imginfo.begin(); it!=imginfo.end(); ++it) {
			if (it->first == "ExposureTime") {
				if (atof(it->second.c_str()) < 1.0) {
					printf("%s: 1/%d\n",it->first.c_str(), int(round(1.0/atof(it->second.c_str()))));
					fflush(stdout);
				}
			}
			else {
				printf("%s: %s\n",it->first.c_str(), it->second.c_str()); 
				fflush(stdout);
			}
		}
		printf("\n"); fflush(stdout);
		exit(0);
	}

	else if (strcmp(outfilename,"histogram") == 0) {
		std::vector<long> h = dib.Histogram();
		printf("%ld",h[0]);
		for (int i = 1; i < h.size(); i++) {
			printf(",%ld",h[i]); 
			fflush(stdout);
		}
		printf("\n"); fflush(stdout);
		exit(0);
	}

	else if (strcmp(outfilename,"cdf") == 0) {
		long prev = 0;
		std::vector<long> h = dib.Histogram();
		printf("%ld",h[0]);
		for (int i = 1; i < h.size(); i++) {
			prev += h[i];
			printf(",%ld",prev); 
			fflush(stdout);
		}
		printf("\n"); fflush(stdout);
		exit(0);
	}

	else if (strcmp(outfilename,"stats") == 0) {
		std::string stats = dib.Stats();
		printf("%s\n",stats.c_str()); fflush(stdout);
		exit(0);
	}
	else if (strcmp(outfilename,"rgbat") == 0) {
		int w = dib.getWidth();
		int h = dib.getHeight();
		std::vector<std::string> coords = split (outfile[1], ",");
		int x = atoi(coords[0].c_str());
		int y = atoi(coords[1].c_str());
		unsigned pos = x + y*w;
		std::vector<pix>& img = dib.getImageData();
		if (pos < w*h) {
			pix rgb = img[pos];
			printf("%f,%f,%f\n", rgb.r, rgb.g, rgb.b);
		} else printf("coordinates out of image bounds\n");
		exit(0);
		
	}


	saveFile (dib, std::string(outfilename), outfile[1], std::string(commandstring), verbose);
	if (verbose) printf("\n");

}

if (count == 1) {
	printf("%d file processed.\n",count); 
	fflush(stdout);
}
else {
	printf("%d files processed.\n",count); 
	fflush(stdout);
}

	return 0;
}
