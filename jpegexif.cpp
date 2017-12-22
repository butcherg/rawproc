#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdio.h>
#include "gimage/strutil.h"

#include <string>
#include <map>

//#include <sstream>
//#define tostr( x ) static_cast< std::ostringstream & >( \
        ( std::ostringstream() << std::dec << x ) ).str()

/*
template <typename T>
  std::string tostr ( T Number )
  {
     std::ostringstream ss;
     ss << Number;
     return ss.str();
  }
*/

const int BytesPerFormat[] = {0,1,1,2,4,8,1,1,2,4,8,4,8};
#define NUM_FORMATS 12

#define FMT_BYTE       1 
#define FMT_STRING     2
#define FMT_USHORT     3
#define FMT_ULONG      4
#define FMT_URATIONAL  5
#define FMT_SBYTE      6
#define FMT_UNDEFINED  7
#define FMT_SSHORT     8
#define FMT_SLONG      9
#define FMT_SRATIONAL 10
#define FMT_SINGLE    11
#define FMT_DOUBLE    12

std::string FormatName(int fmt)
{
	if (fmt == FMT_BYTE) return "FMT_BYTE";
	if (fmt == FMT_STRING) return "FMT_STRING";
	if (fmt == FMT_USHORT) return "FMT_USHORT";
	if (fmt == FMT_ULONG) return "FMT_ULONG";
	if (fmt == FMT_URATIONAL) return "FMT_URATIONAL";
	if (fmt == FMT_UNDEFINED) return "FMT_UNDEFINED";
	if (fmt == FMT_SSHORT) return "FMT_SSHORT";
	if (fmt == FMT_SLONG) return "FMT_SLONG";
	if (fmt == FMT_SRATIONAL) return "FMT_SRATIONAL";
	if (fmt == FMT_SINGLE) return "FMT_SINGLE";
	if (fmt == FMT_DOUBLE) return "FMT_DOUBLE";
	if (fmt == FMT_SBYTE) return "FMT_SBYTE";
	return "beats me...";
}

/*
typedef struct {
    unsigned short Tag;
    const char * Desc;
}TagTable_t;
*/

//--------------------------------------------------------------------------
// Describes tag values

#define TAG_INTEROP_INDEX          0x0001
#define TAG_INTEROP_VERSION        0x0002
#define TAG_IMAGE_WIDTH            0x0100
#define TAG_IMAGE_LENGTH           0x0101
#define TAG_BITS_PER_SAMPLE        0x0102
#define TAG_COMPRESSION            0x0103
#define TAG_PHOTOMETRIC_INTERP     0x0106
#define TAG_FILL_ORDER             0x010A
#define TAG_DOCUMENT_NAME          0x010D
#define TAG_IMAGE_DESCRIPTION      0x010E
#define TAG_MAKE                   0x010F
#define TAG_MODEL                  0x0110
#define TAG_SRIP_OFFSET            0x0111
#define TAG_ORIENTATION            0x0112
#define TAG_SAMPLES_PER_PIXEL      0x0115
#define TAG_ROWS_PER_STRIP         0x0116
#define TAG_STRIP_BYTE_COUNTS      0x0117
#define TAG_X_RESOLUTION           0x011A
#define TAG_Y_RESOLUTION           0x011B
#define TAG_PLANAR_CONFIGURATION   0x011C
#define TAG_RESOLUTION_UNIT        0x0128
#define TAG_TRANSFER_FUNCTION      0x012D
#define TAG_SOFTWARE               0x0131
#define TAG_DATETIME               0x0132
#define TAG_ARTIST                 0x013B
#define TAG_WHITE_POINT            0x013E
#define TAG_PRIMARY_CHROMATICITIES 0x013F
#define TAG_TRANSFER_RANGE         0x0156
#define TAG_JPEG_PROC              0x0200
#define TAG_THUMBNAIL_OFFSET       0x0201
#define TAG_THUMBNAIL_LENGTH       0x0202
#define TAG_Y_CB_CR_COEFFICIENTS   0x0211
#define TAG_Y_CB_CR_SUB_SAMPLING   0x0212
#define TAG_Y_CB_CR_POSITIONING    0x0213
#define TAG_REFERENCE_BLACK_WHITE  0x0214
#define TAG_RELATED_IMAGE_WIDTH    0x1001
#define TAG_RELATED_IMAGE_LENGTH   0x1002
#define TAG_CFA_REPEAT_PATTERN_DIM 0x828D
#define TAG_CFA_PATTERN1           0x828E
#define TAG_BATTERY_LEVEL          0x828F
#define TAG_COPYRIGHT              0x8298
#define TAG_EXPOSURETIME           0x829A
#define TAG_FNUMBER                0x829D
#define TAG_IPTC_NAA               0x83BB
#define TAG_EXIF_OFFSET            0x8769
#define TAG_INTER_COLOR_PROFILE    0x8773
#define TAG_EXPOSURE_PROGRAM       0x8822
#define TAG_SPECTRAL_SENSITIVITY   0x8824
#define TAG_GPSINFO                0x8825
#define TAG_ISO_EQUIVALENT         0x8827
#define TAG_OECF                   0x8828
#define TAG_EXIF_VERSION           0x9000
#define TAG_DATETIME_ORIGINAL      0x9003
#define TAG_DATETIME_DIGITIZED     0x9004
#define TAG_COMPONENTS_CONFIG      0x9101
#define TAG_CPRS_BITS_PER_PIXEL    0x9102
#define TAG_SHUTTERSPEED           0x9201
#define TAG_APERTURE               0x9202
#define TAG_BRIGHTNESS_VALUE       0x9203
#define TAG_EXPOSURE_BIAS          0x9204
#define TAG_MAXAPERTURE            0x9205
#define TAG_SUBJECT_DISTANCE       0x9206
#define TAG_METERING_MODE          0x9207
#define TAG_LIGHT_SOURCE           0x9208
#define TAG_FLASH                  0x9209
#define TAG_FOCALLENGTH            0x920A
#define TAG_SUBJECTAREA            0x9214
#define TAG_MAKER_NOTE             0x927C
#define TAG_USERCOMMENT            0x9286
#define TAG_SUBSEC_TIME            0x9290
#define TAG_SUBSEC_TIME_ORIG       0x9291
#define TAG_SUBSEC_TIME_DIG        0x9292

#define TAG_WINXP_TITLE            0x9c9b // Windows XP - not part of exif standard.
#define TAG_WINXP_COMMENT          0x9c9c // Windows XP - not part of exif standard.
#define TAG_WINXP_AUTHOR           0x9c9d // Windows XP - not part of exif standard.
#define TAG_WINXP_KEYWORDS         0x9c9e // Windows XP - not part of exif standard.
#define TAG_WINXP_SUBJECT          0x9c9f // Windows XP - not part of exif standard.

#define TAG_FLASH_PIX_VERSION      0xA000
#define TAG_COLOR_SPACE            0xA001
#define TAG_PIXEL_X_DIMENSION      0xA002
#define TAG_PIXEL_Y_DIMENSION      0xA003
#define TAG_RELATED_AUDIO_FILE     0xA004
#define TAG_INTEROP_OFFSET         0xA005
#define TAG_FLASH_ENERGY           0xA20B
#define TAG_SPATIAL_FREQ_RESP      0xA20C
#define TAG_FOCAL_PLANE_XRES       0xA20E
#define TAG_FOCAL_PLANE_YRES       0xA20F
#define TAG_FOCAL_PLANE_UNITS      0xA210
#define TAG_SUBJECT_LOCATION       0xA214
#define TAG_EXPOSURE_INDEX         0xA215
#define TAG_SENSING_METHOD         0xA217
#define TAG_FILE_SOURCE            0xA300
#define TAG_SCENE_TYPE             0xA301
#define TAG_CFA_PATTERN            0xA302
#define TAG_CUSTOM_RENDERED        0xA401
#define TAG_EXPOSURE_MODE          0xA402
#define TAG_WHITEBALANCE           0xA403
#define TAG_DIGITALZOOMRATIO       0xA404
#define TAG_FOCALLENGTH_35MM       0xA405
#define TAG_SCENE_CAPTURE_TYPE     0xA406
#define TAG_GAIN_CONTROL           0xA407
#define TAG_CONTRAST               0xA408
#define TAG_SATURATION             0xA409
#define TAG_SHARPNESS              0xA40A
#define TAG_DISTANCE_RANGE         0xA40C
#define TAG_IMAGE_UNIQUE_ID        0xA420



typedef struct {
	unsigned short Tag;
	const char * Desc;
	unsigned short Format;
}TagTable_t;

const TagTable_t TagTable[] = {
{TAG_APERTURE,"ApertureValue",FMT_URATIONAL},
{TAG_ARTIST,"Artist",FMT_STRING},
{TAG_BATTERY_LEVEL,"BatteryLevel",FMT_URATIONAL},
{TAG_BITS_PER_SAMPLE,"BitsPerSample",FMT_USHORT},
{TAG_BRIGHTNESS_VALUE,"BrightnessValue",FMT_SRATIONAL},
{TAG_CFA_PATTERN1,"CFAPattern",FMT_BYTE},
{TAG_CFA_REPEAT_PATTERN_DIM,"CFARepeatPatternDim",FMT_USHORT},
{TAG_COLOR_SPACE,"ColorSpace",FMT_USHORT},
{TAG_COMPRESSION,"Compression",FMT_USHORT},
{TAG_CONTRAST,"Contrast",FMT_USHORT},
{TAG_COPYRIGHT,"Copyright",FMT_STRING},
{TAG_CPRS_BITS_PER_PIXEL,"CompressedBitsPerPixel",FMT_URATIONAL},
{TAG_CUSTOM_RENDERED,"CustomRendered",FMT_USHORT},
{TAG_DATETIME,"DateTime",FMT_STRING},
{TAG_DATETIME_DIGITIZED,"DateTimeDigitized",FMT_STRING},
{TAG_DATETIME_ORIGINAL,"DateTimeOriginal",FMT_STRING},
{TAG_DIGITALZOOMRATIO,"DigitalZoomRatio",FMT_URATIONAL},
{TAG_DISTANCE_RANGE,"SubjectDistanceRange",FMT_USHORT},
{TAG_DOCUMENT_NAME,"DocumentName",FMT_STRING},
{TAG_EXIF_OFFSET,"ExifTag",FMT_ULONG},
{TAG_EXPOSURE_BIAS,"ExposureBiasValue",FMT_SRATIONAL},
{TAG_EXPOSURE_INDEX,"ExposureIndex",FMT_URATIONAL},
{TAG_EXPOSURE_MODE,"ExposureMode",FMT_USHORT},
{TAG_EXPOSURE_PROGRAM,"ExposureProgram",FMT_USHORT},
{TAG_EXPOSURETIME,"ExposureTime",FMT_URATIONAL},
{TAG_FILL_ORDER,"FillOrder",FMT_USHORT},
{TAG_FLASH,"Flash",FMT_USHORT},
{TAG_FLASH_ENERGY,"FlashEnergy",FMT_URATIONAL},
{TAG_FNUMBER,"FNumber",FMT_URATIONAL},
{TAG_FOCAL_PLANE_UNITS,"FocalPlaneResolutionUnit",FMT_USHORT},
{TAG_FOCAL_PLANE_XRES,"FocalPlaneXResolution",FMT_URATIONAL},
{TAG_FOCAL_PLANE_YRES,"FocalPlaneYResolution",FMT_URATIONAL},
{TAG_FOCALLENGTH,"FocalLength",FMT_URATIONAL},
{TAG_FOCALLENGTH_35MM,"FocalLengthIn35mmFilm",FMT_USHORT},
{TAG_GAIN_CONTROL,"GainControl",FMT_USHORT},
{TAG_GPSINFO,"GPSTag",FMT_ULONG},
{TAG_IMAGE_DESCRIPTION,"ImageDescription",FMT_STRING},
{TAG_IMAGE_LENGTH,"ImageLength",FMT_ULONG},
{TAG_IMAGE_UNIQUE_ID,"ImageUniqueID",FMT_STRING},
{TAG_IMAGE_WIDTH,"ImageWidth",FMT_ULONG},
{TAG_INTEROP_OFFSET,"InteroperabilityTag",FMT_ULONG},
{TAG_IPTC_NAA,"IPTCNAA",FMT_ULONG},
{TAG_ISO_EQUIVALENT,"ISOSpeedRatings",FMT_USHORT},
{TAG_JPEG_PROC,"JPEGProc",FMT_ULONG},
{TAG_LIGHT_SOURCE,"LightSource",FMT_USHORT},
{TAG_MAKE,"Make",FMT_STRING},
{TAG_MAXAPERTURE,"MaxApertureValue",FMT_URATIONAL},
{TAG_METERING_MODE,"MeteringMode",FMT_USHORT},
{TAG_MODEL,"Model",FMT_STRING},
{TAG_ORIENTATION,"Orientation",FMT_USHORT},
{TAG_PHOTOMETRIC_INTERP,"PhotometricInterpretation",FMT_USHORT},
{TAG_PIXEL_X_DIMENSION,"PixelXDimension",FMT_ULONG},
{TAG_PIXEL_Y_DIMENSION,"PixelYDimension",FMT_ULONG},
{TAG_PLANAR_CONFIGURATION,"PlanarConfiguration",FMT_USHORT},
{TAG_PRIMARY_CHROMATICITIES,"PrimaryChromaticities",FMT_URATIONAL},
{TAG_REFERENCE_BLACK_WHITE,"ReferenceBlackWhite",FMT_URATIONAL},
{TAG_RELATED_AUDIO_FILE,"RelatedSoundFile",FMT_STRING},
{TAG_RELATED_IMAGE_LENGTH,"RelatedImageLength",FMT_ULONG},
{TAG_RELATED_IMAGE_WIDTH,"RelatedImageWidth",FMT_ULONG},
{TAG_RESOLUTION_UNIT,"ResolutionUnit",FMT_USHORT},
{TAG_ROWS_PER_STRIP,"RowsPerStrip",FMT_ULONG},
{TAG_SAMPLES_PER_PIXEL,"SamplesPerPixel",FMT_USHORT},
{TAG_SATURATION,"Saturation",FMT_USHORT},
{TAG_SCENE_CAPTURE_TYPE,"SceneCaptureType",FMT_USHORT},
{TAG_SENSING_METHOD,"SensingMethod",FMT_USHORT},
{TAG_SHARPNESS,"Sharpness",FMT_USHORT},
{TAG_SHUTTERSPEED,"ShutterSpeedValue",FMT_SRATIONAL},
{TAG_SOFTWARE,"Software",FMT_STRING},
{TAG_SPECTRAL_SENSITIVITY,"SpectralSensitivity",FMT_STRING},
{TAG_SRIP_OFFSET,"StripOffsets",FMT_ULONG},
{TAG_STRIP_BYTE_COUNTS,"StripByteCounts",FMT_ULONG},
{TAG_SUBJECT_DISTANCE,"SubjectDistance",FMT_SRATIONAL},
{TAG_SUBJECT_LOCATION,"SubjectLocation",FMT_USHORT},
{TAG_SUBJECTAREA,"SubjectLocation",FMT_USHORT},
{TAG_SUBSEC_TIME,"SubSecTime",FMT_STRING},
{TAG_SUBSEC_TIME_DIG,"SubSecTimeDigitized",FMT_STRING},
{TAG_SUBSEC_TIME_ORIG,"SubSecTimeOriginal",FMT_STRING},
{TAG_THUMBNAIL_LENGTH,"JPEGInterchangeFormatLength",FMT_ULONG},
{TAG_THUMBNAIL_OFFSET,"JPEGInterchangeFormat",FMT_ULONG},
{TAG_TRANSFER_FUNCTION,"TransferFunction",FMT_USHORT},
{TAG_TRANSFER_RANGE,"TransferRange",FMT_USHORT},
{TAG_WHITE_POINT,"WhitePoint",FMT_URATIONAL},
{TAG_WHITEBALANCE,"WhiteBalance",FMT_USHORT},
{TAG_WINXP_AUTHOR,"XPAuthor",FMT_BYTE},
{TAG_WINXP_COMMENT,"XPComment",FMT_BYTE},
{TAG_WINXP_KEYWORDS,"XPKeywords",FMT_BYTE},
{TAG_WINXP_SUBJECT,"XPSubject",FMT_BYTE},
{TAG_WINXP_TITLE,"XPTitle",FMT_BYTE},
{TAG_X_RESOLUTION,"XResolution",FMT_URATIONAL},
{TAG_Y_CB_CR_COEFFICIENTS,"YCbCrCoefficients",FMT_URATIONAL},
{TAG_Y_CB_CR_POSITIONING,"YCbCrPositioning",FMT_USHORT},
{TAG_Y_CB_CR_SUB_SAMPLING,"YCbCrSubSampling",FMT_USHORT},
{TAG_Y_RESOLUTION,"YResolution",FMT_URATIONAL},
//{TAG_USERCOMMENT,"UserComment",FMT_STRING}
} ;

#define TAG_TABLE_SIZE  (sizeof(TagTable) / sizeof(TagTable_t))


unsigned getTag(std::string tagname)
{
	for (unsigned i = 0; i < TAG_TABLE_SIZE; i++) 
		if (tagname.compare(TagTable[i].Desc) == 0) return TagTable[i].Tag;
	return 0;
}

unsigned getTagFormat(std::string tagname)
{
	for (unsigned i = 0; i < TAG_TABLE_SIZE; i++) 
		if (tagname.compare(TagTable[i].Desc) == 0) return TagTable[i].Format;
	return 0;
}

unsigned countValidEXIFEntries(std::map<std::string,std::string> imageinfo)
{
	unsigned c = 0;
	for (std::map<std::string,std::string>::iterator it=imageinfo.begin(); it!=imageinfo.end(); ++it)
		if (getTag(it->first)) c++;
	return c;
}


typedef unsigned char uchar;
int MotorolaOrder = 0;

//--------------------------------------------------------------------------
// Convert a 16 bit unsigned value to file's native byte order
//--------------------------------------------------------------------------
static void Put16u(void * Short, unsigned short PutValue)
{
    if (MotorolaOrder){
        ((uchar *)Short)[0] = (uchar)(PutValue>>8);
        ((uchar *)Short)[1] = (uchar)PutValue;
    }else{
        ((uchar *)Short)[0] = (uchar)PutValue;
        ((uchar *)Short)[1] = (uchar)(PutValue>>8);
    }
}

//--------------------------------------------------------------------------
// Convert a 16 bit unsigned value from file's native byte order
//--------------------------------------------------------------------------
int Get16u(void * Short)
{
    if (MotorolaOrder){
        return (((uchar *)Short)[0] << 8) | ((uchar *)Short)[1];
    }else{
        return (((uchar *)Short)[1] << 8) | ((uchar *)Short)[0];
    }
}

//--------------------------------------------------------------------------
// Convert a 32 bit signed value from file's native byte order
//--------------------------------------------------------------------------
int Get32s(void * Long)
{
    if (MotorolaOrder){
        return  ((( char *)Long)[0] << 24) | (((uchar *)Long)[1] << 16)
              | (((uchar *)Long)[2] << 8 ) | (((uchar *)Long)[3] << 0 );
    }else{
        return  ((( char *)Long)[3] << 24) | (((uchar *)Long)[2] << 16)
              | (((uchar *)Long)[1] << 8 ) | (((uchar *)Long)[0] << 0 );
    }
}

//--------------------------------------------------------------------------
// Convert a 32 bit unsigned value to file's native byte order
//--------------------------------------------------------------------------
void Put32u(void * Value, unsigned PutValue)
{
    if (MotorolaOrder){
        ((uchar *)Value)[0] = (uchar)(PutValue>>24);
        ((uchar *)Value)[1] = (uchar)(PutValue>>16);
        ((uchar *)Value)[2] = (uchar)(PutValue>>8);
        ((uchar *)Value)[3] = (uchar)PutValue;
    }else{
        ((uchar *)Value)[0] = (uchar)PutValue;
        ((uchar *)Value)[1] = (uchar)(PutValue>>8);
        ((uchar *)Value)[2] = (uchar)(PutValue>>16);
        ((uchar *)Value)[3] = (uchar)(PutValue>>24);
    }
}

//--------------------------------------------------------------------------
// Convert a 32 bit unsigned value from file's native byte order
//--------------------------------------------------------------------------
unsigned Get32u(void * Long)
{
    return (unsigned)Get32s(Long) & 0xffffffff;
}

//--------------------------------------------------------------------------
// Display a number as one of its many formats
//--------------------------------------------------------------------------
void PrintFormatNumber(void * ValuePtr, int Format, int ByteCount)
{
    int s,n;

    for(n=0;n<16;n++){
        switch(Format){
            case FMT_SBYTE:
            case FMT_BYTE:      printf("%02x",*(uchar *)ValuePtr); s=1;  break;
            case FMT_USHORT:    printf("%d",Get16u(ValuePtr)); s=2;      break;
            case FMT_ULONG:     
            case FMT_SLONG:     printf("%d",Get32s(ValuePtr)); s=4;      break;
            case FMT_SSHORT:    printf("%hd",(signed short)Get16u(ValuePtr)); s=2; break;
            case FMT_URATIONAL:
                printf("%u/%u",Get32s(ValuePtr), Get32s(4+(char *)ValuePtr)); 
                s = 8;
                break;

            case FMT_SRATIONAL: 
                printf("%d/%d",Get32s(ValuePtr), Get32s(4+(char *)ValuePtr)); 
                s = 8;
                break;

            case FMT_SINGLE:    printf("%f",(double)*(float *)ValuePtr); s=8; break;
            case FMT_DOUBLE:    printf("%f",*(double *)ValuePtr);        s=8; break;
            default: 
                printf("Unknown format %d:", Format);
                return;
        }
        ByteCount -= s;
        if (ByteCount <= 0) break;
        printf(", ");
        ValuePtr = (void *)((char *)ValuePtr + s);

    }
    if (n >= 16) printf("...");
}

//--------------------------------------------------------------------------
// Evaluate number, be it int, rational, or float from directory.
//--------------------------------------------------------------------------
double ConvertAnyFormat(void * ValuePtr, int Format)
{
    double Value;
    Value = 0;

    switch(Format){
        case FMT_SBYTE:     Value = *(signed char *)ValuePtr;  break;
        case FMT_BYTE:      Value = *(uchar *)ValuePtr;        break;

        case FMT_USHORT:    Value = Get16u(ValuePtr);          break;
        case FMT_ULONG:     Value = Get32u(ValuePtr);          break;

        case FMT_URATIONAL:
        case FMT_SRATIONAL: 
            {
                int Num,Den;
                Num = Get32s(ValuePtr);
                Den = Get32s(4+(char *)ValuePtr);
                if (Den == 0){
                    Value = 0;
                }else{
                    if (Format == FMT_SRATIONAL){
                        Value = (double)Num/Den;
                    }else{
                        Value = (double)(unsigned)Num/(double)(unsigned)Den;
                    }
                }
                break;
            }

        case FMT_SSHORT:    Value = (signed short)Get16u(ValuePtr);  break;
        case FMT_SLONG:     Value = Get32s(ValuePtr);                break;

        // Not sure if this is correct (never seen float used in Exif format)
        case FMT_SINGLE:    Value = (double)*(float *)ValuePtr;      break;
        case FMT_DOUBLE:    Value = *(double *)ValuePtr;             break;

        //default:
            //ErrNonfatal("Illegal format code %d in Exif header",Format,0);
    }
    return Value;
}



bool ProcessExifDir(unsigned char * DirStart, unsigned char * OffsetBase, unsigned ExifLength, std::map<std::string,std::string> &imageinfo)
{
	int de;
	int a;
	int NumDirEntries;
	unsigned ThumbnailOffset = 0;
	unsigned ThumbnailSize = 0;

	NumDirEntries = Get16u(DirStart);
	#define DIR_ENTRY_ADDR(Start, Entry) (Start+2+12*(Entry))

		unsigned char * DirEnd;

	{
		//unsigned char * DirEnd;
		DirEnd = DIR_ENTRY_ADDR(DirStart, NumDirEntries);
		if (DirEnd+4 > (OffsetBase+ExifLength)){
			if (DirEnd+2 == OffsetBase+ExifLength || DirEnd == OffsetBase+ExifLength){
			// Version 1.3 of jhead would truncate a bit too much.
			// This also caught later on as well.
			}else{
				return false;
			}
		}
	}

	for (de=0;de<NumDirEntries;de++){
		int Tag, Format, Components;
		unsigned char * ValuePtr;
		int ByteCount;
		unsigned char * DirEntry;
		DirEntry = DIR_ENTRY_ADDR(DirStart, de);

		Tag = Get16u(DirEntry);
		Format = Get16u(DirEntry+2);
		Components = Get32u(DirEntry+4);

		if ((Format-1) >= NUM_FORMATS) continue;
		if ((unsigned)Components > 0x10000) continue;

		ByteCount = Components * BytesPerFormat[Format];

		if (ByteCount > 4){
			unsigned OffsetVal;
			OffsetVal = Get32u(DirEntry+8);
			// If its bigger than 4 bytes, the dir entry contains an offset.
			if (OffsetVal+ByteCount > ExifLength){
				// Bogus pointer offset and / or bytecount value
				continue;
			}
			ValuePtr = OffsetBase+OffsetVal;



		}else{
			// 4 bytes or less and value is in the dir entry itself
			ValuePtr = DirEntry+8;
		}

		if (Tag == TAG_MAKER_NOTE) continue;

		// Show tag name
		std::string tagname = "";
		std::string tagvalue = "";
		for (a=0;;a++){
			if (a >= TAG_TABLE_SIZE){
				//printf("%04x=", Tag);
				tagname = "Unknown"; //nope, no string
				break;
			}
			if (TagTable[a].Tag == Tag){
				//printf("%s=", TagTable[a].Desc);
				tagname = TagTable[a].Desc;
				break;
			}
		}

		//printf("%s  ", FormatName(Format).c_str());
		// Show tag value
		switch(Format){
			case FMT_BYTE:
				tagvalue.push_back(ValuePtr[0]);
				//if(ByteCount>1){
				//	printf("%.*ls\n", ByteCount/2, (wchar_t *)ValuePtr);
				//}else{
				//	PrintFormatNumber(ValuePtr, Format, ByteCount);
				//	printf("\n");
				//}
				break;

			case FMT_UNDEFINED:
				// Undefined is typically an ascii string.

			case FMT_STRING:
				// String arrays printed without function call (different from int arrays)
				{
					for (a=0;a<ByteCount;a++){
						if (ValuePtr[a] == 0) {
							break;
						}
						if (ValuePtr[a] >= 32 & ValuePtr[a] <= 126 ){
							tagvalue.push_back(ValuePtr[a]);
						}else{

							tagvalue.push_back(' ');

						}
					}
					//printf("\"\n");
				}
				break;
				
			default:
				// Handle arrays of numbers later (will there ever be?)
				//PrintFormatNumber(ValuePtr, Format, ByteCount);
				//printf("\n");
				//tagvalue = FormatNumber(ValuePtr, Format, ByteCount);
				tagvalue = tostr(ConvertAnyFormat(ValuePtr, Format));
		}

		imageinfo[tagname] = tagvalue;

		switch(Tag){
			case TAG_EXIF_OFFSET:
			case TAG_INTEROP_OFFSET:
				//if (Tag == TAG_INTEROP_OFFSET && ShowTags) printf("%s    Interop Dir:",IndentString);
				{
					unsigned char * SubdirStart;
					SubdirStart = OffsetBase + Get32u(ValuePtr);
					if (SubdirStart < OffsetBase || SubdirStart > OffsetBase+ExifLength){
						//ErrNonfatal("Illegal Exif or interop ofset directory link",0,0);
					}else{
						//ggb: ProcessExifDir(SubdirStart, OffsetBase, ExifLength, NestingLevel+1);
						ProcessExifDir(SubdirStart, OffsetBase, ExifLength, imageinfo);
					}
					continue;
				}
                	break;
		}


	}

	



    {
        // In addition to linking to subdirectories via exif tags, 
        // there's also a potential link to another directory at the end of each
        // directory.  this has got to be the result of a committee!
        unsigned char * SubdirStart;
        unsigned Offset;

        if (DIR_ENTRY_ADDR(DirStart, NumDirEntries) + 4 <= OffsetBase+ExifLength){
            Offset = Get32u(DirStart+2+12*NumDirEntries);
            if (Offset){
                SubdirStart = OffsetBase + Offset;
                if (SubdirStart > OffsetBase+ExifLength || SubdirStart < OffsetBase){
                    if (SubdirStart > OffsetBase && SubdirStart < OffsetBase+ExifLength+20){
                        // Jhead 1.3 or earlier would crop the whole directory!
                        // As Jhead produces this form of format incorrectness, 
                        // I'll just let it pass silently
                        //if (ShowTags) printf("Thumbnail removed with Jhead 1.3 or earlier\n");
                    }else{
                        //ErrNonfatal("Illegal subdirectory link in Exif header",0,0);
                    }
                }else{
                    if (SubdirStart <= OffsetBase+ExifLength){
                        //if (ShowTags) printf("%s    Continued directory ",IndentString);
			//ggb: ProcessExifDir(SubdirStart, OffsetBase, ExifLength, NestingLevel+1);
                        ProcessExifDir(SubdirStart, OffsetBase, ExifLength, imageinfo);
                    }
                }
                //if (Offset > ImageInfo.LargestExifOffset){
                //    ImageInfo.LargestExifOffset = Offset;
                //}
            }
        }else{
            // The exif header ends before the last next directory pointer.
        }
    }

    if (ThumbnailOffset){

        if (ThumbnailOffset <= ExifLength){
            if (ThumbnailSize > ExifLength-ThumbnailOffset){
                // If thumbnail extends past exif header, only save the part that
                // actually exists.  Canon's EOS viewer utility will do this - the
                // thumbnail extracts ok with this hack.
                ThumbnailSize = ExifLength-ThumbnailOffset;
                //if (ShowTags) printf("Thumbnail incorrectly placed in header\n");

            }
            // The thumbnail pointer appears to be valid.  Store it.
            //ImageInfo.ThumbnailOffset = ThumbnailOffset;
            //ImageInfo.ThumbnailSize = ThumbnailSize;

        }
    }

}






void parse_APP1marker(unsigned char * marker, unsigned length, std::map<std::string,std::string> &imageinfo)
{
	char exif_str[65535];

	unsigned int FirstOffset;

	double FocalplaneXRes = 0;
	double FocalplaneUnits = 0;
	int ExifImageWidth = 0;
	int NumOrientations = 0;

	// Check the EXIF header component:
	{   
		static char ExifHeader[] = "Exif\0\0";
		if (memcmp(marker+2, ExifHeader,6)) return;
	}

	// Set the internal endian:
	if (memcmp(marker+8,"II",2) == 0){
		MotorolaOrder = 0;
	}else{
		if (memcmp(marker+8,"MM",2) == 0){
			MotorolaOrder = 1;
		}else{

			return;
		}
	}

	// Check the next value for correctness.
	if (Get16u(marker+10) != 0x2a) return;

	FirstOffset = Get32u(marker+12);
	if (FirstOffset < 8 || FirstOffset > 16)
		if (FirstOffset < 16 || FirstOffset > length-16) return;

	ProcessExifDir(marker+8+FirstOffset, marker+8, length-8, imageinfo);
}





//Globals for constructing the APP1 marker:

unsigned char Buffer[512];

unsigned short NumEntries;
int DataWriteIndex;
int DateIndex;
int DirIndex;
int DirContinuation;
    
void addAPP1Entry(unsigned tag, unsigned format, std::string value)
{
	// Tag
	Put16u(Buffer+DirIndex, tag);

	// Format
	Put16u(Buffer+DirIndex + 2, format);

	//Components
	if (format == FMT_STRING) 
		Put32u(Buffer+DirIndex + 4, value.length()+1);	
	else if (format == FMT_URATIONAL | format == FMT_USHORT)
		Put32u(Buffer+DirIndex + 4, 1);	

	//Pointer or value
	if (format == FMT_USHORT)
		Put16u(Buffer+DirIndex + 8, atoi(value.c_str()));
	else if (format == FMT_URATIONAL | format == FMT_STRING)
		Put32u(Buffer+DirIndex + 8, DataWriteIndex-8);

	DirIndex += 12;

	if (format == FMT_URATIONAL) {
		Put32u(Buffer+DataWriteIndex, (int)(atof(value.c_str())*10000000.0));
		Put32u(Buffer+DataWriteIndex+4, 10000000);
		DataWriteIndex += 8;
	}
	else if (format == FMT_STRING) {
		strcpy((char *) Buffer+DataWriteIndex, value.c_str());
		DataWriteIndex += value.length()+1;
	}
}


unsigned char * construct_APP1marker(std::map<std::string,std::string> imageinfo, unsigned *markerlength)
{

	MotorolaOrder = 0;

	memcpy(Buffer+2, "Exif\0\0II",8);
	Put16u(Buffer+10, 0x2a);

	DataWriteIndex = 16;
	Put32u(Buffer+12, DataWriteIndex-8); // first IFD offset.  Means start 16 bytes in.


	DirIndex = DataWriteIndex;
	NumEntries = countValidEXIFEntries(imageinfo);
	DataWriteIndex += 2 + NumEntries*12 + 4;

	Put16u(Buffer+DirIndex, NumEntries); // Number of entries
	DirIndex += 2;


	for (std::map<std::string,std::string>::iterator it=imageinfo.begin(); it!=imageinfo.end(); ++it) {
		unsigned tag = getTag(it->first);
		if (tag) {
			addAPP1Entry(tag, getTagFormat(it->first), it->second);
		}
	}

	Put32u(Buffer+DirIndex, 0);


	Buffer[0] = (unsigned char)(DataWriteIndex >> 8);
	Buffer[1] = (unsigned char)DataWriteIndex;

	// Remove old exif section, if there was one.
	//RemoveSectionType(M_EXIF);

	unsigned char * NewBuf = new unsigned char[DataWriteIndex]; 
        if (NewBuf == NULL){
            //////ErrFatal("Could not allocate memory");
        }
        memcpy(NewBuf, Buffer, DataWriteIndex);

	*markerlength = DataWriteIndex;
	return NewBuf;
}






