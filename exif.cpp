
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <gimage/gimage.h>

void printtag(std::string name, std::string val, bool both=false)
{
	if (both) printf("%s: ",name.c_str());
	if (name == "ExposureTime") {
		if (atof(val.c_str()) < 1.0) {
			printf("1/%dsec ", int(round(1.0/atof(val.c_str()))));
		}
	}
	else if (name == "FNumber") {
		printf("f%s ", val.c_str());
	}
	else if (name.find("FocalLength") != std::string::npos ) {
		printf("%smm ", val.c_str());
	}
	else
		printf("%s ", val.c_str());
	
	
}

int main(int argc, char * argv[])
{
	int c, i;
	bool showfilename = false, alltags = true;
	
	char *tags, *tag;

	const char *shottags = "ExposureTime,FNumber,FocalLength,ISOSpeedRatings";
	
	if (argc == 1) {
		printf("Usage: exif [-f] [-t tag1,tag2,...] [-s]\n\n\t-f: Show filename\n\t-t taglist: Show specific tags\n\t-s: Show predefined shot tags - %s\n",shottags);
		exit(0);
	}

	while ((c = getopt (argc, argv, "fst:")) != -1) { 
		switch (c)
		{
			case 'f':
				showfilename = true;
				break;
			case 't':
				tags = optarg;
				alltags = false;
				break;
			case 's':
				tags = (char *) shottags;
				alltags = false;
				break;
		}
	}

	for (i = optind; i < argc; i++) {
		std::map<std::string,std::string> imgdata =  gImage::loadImageFileInfo(argv[i]);
		if (showfilename) {
			printf ("%s: ", argv[i]);
			if (alltags) printf("\n");
		}
		for (std::map<std::string,std::string>::iterator it=imgdata.begin(); it!=imgdata.end(); ++it) {
			if (alltags) {
				printtag(it->first, it->second, true);
				printf("\n");
			}
			else {
				char taglist[2048];
				strncpy(taglist,tags,2047);
				tag = strtok(taglist,",");
				while (tag) {
					//if ((it->first.find(std::string(tag)) != std::string::npos))
					if (it->first == std::string(tag)) printtag(it->first, it->second);
					tag = strtok(NULL, ",");
				}
			}
		}
		printf("\n");
	}
	
}
