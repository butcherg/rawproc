#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <map>
#include <string>

#ifdef __cplusplus
extern "C" {
#endif

#include <jpeglib.h>

#ifdef __cplusplus
}
#endif


#include <gimage/strutil.h>
#include "jpegexif.h"

#define ICC_MARKER  (JPEG_APP0 + 2)	/* JPEG marker code for ICC */
#define ICC_OVERHEAD_LEN  14		/* size of non-profile data in APP2 */
#define MAX_BYTES_IN_MARKER  65533	/* maximum data len of a JPEG marker */
#define MAX_DATA_BYTES_IN_MARKER  (MAX_BYTES_IN_MARKER - ICC_OVERHEAD_LEN)

char jpegversion[] = "jpeg6B";

const char * jpegVersion()
{
	return jpegversion;
}


/*
 * This routine writes the given ICC profile data into a JPEG file.
 * It *must* be called AFTER calling jpeg_start_compress() and BEFORE
 * the first call to jpeg_write_scanlines().
 * (This ordering ensures that the APP2 marker(s) will appear after the
 * SOI and JFIF or Adobe markers, but before all else.)
 */

void
write_icc_profile (j_compress_ptr cinfo,
		   const JOCTET *icc_data_ptr,
		   unsigned int icc_data_len)
{
  unsigned int num_markers;	/* total number of markers we'll write */
  int cur_marker = 1;		/* per spec, counting starts at 1 */
  unsigned int length;		/* number of bytes to write in this marker */

  /* Calculate the number of markers we'll need, rounding up of course */
  num_markers = icc_data_len / MAX_DATA_BYTES_IN_MARKER;
  if (num_markers * MAX_DATA_BYTES_IN_MARKER != icc_data_len)
    num_markers++;

  while (icc_data_len > 0) {
    /* length of profile to put in this marker */
    length = icc_data_len;
    if (length > MAX_DATA_BYTES_IN_MARKER)
      length = MAX_DATA_BYTES_IN_MARKER;
    icc_data_len -= length;

    /* Write the JPEG marker header (APP2 code and marker length) */
    jpeg_write_m_header(cinfo, ICC_MARKER,
			(unsigned int) (length + ICC_OVERHEAD_LEN));

    /* Write the marker identifying string "ICC_PROFILE" (null-terminated).
     * We code it in this less-than-transparent way so that the code works
     * even if the local character set is not ASCII.
     */
    jpeg_write_m_byte(cinfo, 0x49);
    jpeg_write_m_byte(cinfo, 0x43);
    jpeg_write_m_byte(cinfo, 0x43);
    jpeg_write_m_byte(cinfo, 0x5F);
    jpeg_write_m_byte(cinfo, 0x50);
    jpeg_write_m_byte(cinfo, 0x52);
    jpeg_write_m_byte(cinfo, 0x4F);
    jpeg_write_m_byte(cinfo, 0x46);
    jpeg_write_m_byte(cinfo, 0x49);
    jpeg_write_m_byte(cinfo, 0x4C);
    jpeg_write_m_byte(cinfo, 0x45);
    jpeg_write_m_byte(cinfo, 0x0);

    /* Add the sequencing info */
    jpeg_write_m_byte(cinfo, cur_marker);
    jpeg_write_m_byte(cinfo, (int) num_markers);

    /* Add the profile data */
    while (length--) {
      jpeg_write_m_byte(cinfo, *icc_data_ptr);
      icc_data_ptr++;
    }
    cur_marker++;
  }
}


/*
 * Prepare for reading an ICC profile
 */

void
setup_read_icc_profile (j_decompress_ptr cinfo)
{
  /* Tell the library to keep any APP2 data it may find */
  jpeg_save_markers(cinfo, ICC_MARKER, 0xFFFF);
}


/*
 * Handy subroutine to test whether a saved marker is an ICC profile marker.
 */

static boolean
marker_is_icc (jpeg_saved_marker_ptr marker)
{
  return
    marker->marker == ICC_MARKER &&
    marker->data_length >= ICC_OVERHEAD_LEN &&
    /* verify the identifying string */
    GETJOCTET(marker->data[0]) == 0x49 &&
    GETJOCTET(marker->data[1]) == 0x43 &&
    GETJOCTET(marker->data[2]) == 0x43 &&
    GETJOCTET(marker->data[3]) == 0x5F &&
    GETJOCTET(marker->data[4]) == 0x50 &&
    GETJOCTET(marker->data[5]) == 0x52 &&
    GETJOCTET(marker->data[6]) == 0x4F &&
    GETJOCTET(marker->data[7]) == 0x46 &&
    GETJOCTET(marker->data[8]) == 0x49 &&
    GETJOCTET(marker->data[9]) == 0x4C &&
    GETJOCTET(marker->data[10]) == 0x45 &&
    GETJOCTET(marker->data[11]) == 0x0;
}


/*
 * See if there was an ICC profile in the JPEG file being read;
 * if so, reassemble and return the profile data.
 *
 * TRUE is returned if an ICC profile was found, FALSE if not.
 * If TRUE is returned, *icc_data_ptr is set to point to the
 * returned data, and *icc_data_len is set to its length.
 *
 * IMPORTANT: the data at **icc_data_ptr has been allocated with malloc()
 * and must be freed by the caller with free() when the caller no longer
 * needs it.  (Alternatively, we could write this routine to use the
 * IJG library's memory allocator, so that the data would be freed implicitly
 * at jpeg_finish_decompress() time.  But it seems likely that many apps
 * will prefer to have the data stick around after decompression finishes.)
 *
 * NOTE: if the file contains invalid ICC APP2 markers, we just silently
 * return FALSE.  You might want to issue an error message instead.
 */

boolean
read_icc_profile (j_decompress_ptr cinfo,
		  JOCTET **icc_data_ptr,
		  unsigned int *icc_data_len)
{
  jpeg_saved_marker_ptr marker;
  int num_markers = 0;
  int seq_no;
  JOCTET *icc_data;
  unsigned int total_length;
#define MAX_SEQ_NO  255		/* sufficient since marker numbers are bytes */
  char marker_present[MAX_SEQ_NO+1];	  /* 1 if marker found */
  unsigned int data_length[MAX_SEQ_NO+1]; /* size of profile data in marker */
  unsigned int data_offset[MAX_SEQ_NO+1]; /* offset for data in marker */

  *icc_data_ptr = NULL;		/* avoid confusion if FALSE return */
  *icc_data_len = 0;

  /* This first pass over the saved markers discovers whether there are
   * any ICC markers and verifies the consistency of the marker numbering.
   */

  for (seq_no = 1; seq_no <= MAX_SEQ_NO; seq_no++)
    marker_present[seq_no] = 0;

  for (marker = cinfo->marker_list; marker != NULL; marker = marker->next) {
    if (marker_is_icc(marker)) {
      if (num_markers == 0)
	num_markers = GETJOCTET(marker->data[13]);
      else if (num_markers != GETJOCTET(marker->data[13]))
	return FALSE;		/* inconsistent num_markers fields */
      seq_no = GETJOCTET(marker->data[12]);
      if (seq_no <= 0 || seq_no > num_markers)
	return FALSE;		/* bogus sequence number */
      if (marker_present[seq_no])
	return FALSE;		/* duplicate sequence numbers */
      marker_present[seq_no] = 1;
      data_length[seq_no] = marker->data_length - ICC_OVERHEAD_LEN;
    }
  }

  if (num_markers == 0)
    return FALSE;

  /* Check for missing markers, count total space needed,
   * compute offset of each marker's part of the data.
   */

  total_length = 0;
  for (seq_no = 1; seq_no <= num_markers; seq_no++) {
    if (marker_present[seq_no] == 0)
      return FALSE;		/* missing sequence number */
    data_offset[seq_no] = total_length;
    total_length += data_length[seq_no];
  }

  if (total_length <= 0)
    return FALSE;		/* found only empty markers? */

  /* Allocate space for assembled data */
  icc_data = (JOCTET *) malloc(total_length * sizeof(JOCTET));
  if (icc_data == NULL)
    return FALSE;		/* oops, out of memory */

  /* and fill it in */
  for (marker = cinfo->marker_list; marker != NULL; marker = marker->next) {
    if (marker_is_icc(marker)) {
      JOCTET FAR *src_ptr;
      JOCTET *dst_ptr;
      unsigned int length;
      seq_no = GETJOCTET(marker->data[12]);
      dst_ptr = icc_data + data_offset[seq_no];
      src_ptr = marker->data + ICC_OVERHEAD_LEN;
      length = data_length[seq_no];
      while (length--) {
	*dst_ptr++ = *src_ptr++;
      }
    }
  }

  *icc_data_ptr = icc_data;
  *icc_data_len = total_length;

  return TRUE;
}

bool _checkJPEG(const char *filename)
{
	FILE * infile;
	struct jpeg_decompress_struct cinfo;
	struct jpeg_error_mgr jerr;

	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_decompress(&cinfo);
	if ((infile = fopen(filename, "rb")) == NULL) return false;
	jpeg_stdio_src(&cinfo, infile);
	if (jpeg_read_header(&cinfo, TRUE) == JPEG_HEADER_OK) return true;
	return false;
}

bool _loadJPEGInfo(const char *filename, 
			unsigned *width, 
			unsigned *height, 
			unsigned *numcolors, 
			std::map<std::string,std::string> &info)
//			std::string params="",
//			char ** iccprofile=NULL, 
//			unsigned  *icclength=0)
{
	struct jpeg_decompress_struct cinfo;
	struct jpeg_error_mgr jerr;

	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_decompress(&cinfo);

	char *img;
	JSAMPROW dst;
	unsigned row_stride;
	FILE * infile;

	unsigned len;
	JOCTET * buffer;

	if ((infile = fopen(filename, "rb")) == NULL) return false;

	jpeg_stdio_src(&cinfo, infile);

	jpeg_save_markers(&cinfo, JPEG_APP0+1, 0xFFFF);
	setup_read_icc_profile(&cinfo);

	jpeg_read_header(&cinfo, TRUE);

	jpeg_marker_struct * marker = cinfo.marker_list;
	std::map<std::string,std::string> imageinfo;
	while (marker != NULL) {
		if (marker->marker == JPEG_APP0+1) parse_APP1marker(marker->data-2, marker->data_length, info);
		marker = marker->next;
	}

	/*
	if (read_icc_profile (&cinfo, &buffer, &len)) {
		*iccprofile = new char[len];
		memcpy(*iccprofile, buffer, len);
		free (buffer);
		*icclength = len;
	}
	else {
		*iccprofile = NULL;
		*icclength = 0;
	}
	*/

	*width = cinfo.image_width;
	*height = cinfo.image_height;
	*numcolors = cinfo.output_components;

	jpeg_destroy_decompress(&cinfo);

	return true;
}

char * _loadJPEG(const char *filename, 
			unsigned *width, 
			unsigned *height, 
			unsigned *numcolors, 
			std::map<std::string,std::string> &info,
			std::string params="",
			char ** iccprofile=NULL, 
			unsigned  *icclength=0)
{
	struct jpeg_decompress_struct cinfo;
	struct jpeg_error_mgr jerr;

	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_decompress(&cinfo);

	char *img;
	JSAMPROW dst;
	unsigned row_stride;
	FILE * infile;

	unsigned len;
	JOCTET * buffer;

	if ((infile = fopen(filename, "rb")) == NULL) {
	    fprintf(stderr, "can't open %s\n", filename);
	    exit(1);
	}
	jpeg_stdio_src(&cinfo, infile);

	jpeg_save_markers(&cinfo, JPEG_APP0+1, 0xFFFF);
	setup_read_icc_profile(&cinfo);

	jpeg_read_header(&cinfo, TRUE);

	jpeg_marker_struct * marker = cinfo.marker_list;
	std::map<std::string,std::string> imageinfo;

	while (marker != NULL) {
		parse_APP1marker(marker->data-2, marker->data_length, info);
		marker = marker->next;
	}

	if (read_icc_profile (&cinfo, &buffer, &len)) {
		*iccprofile = new char[len];
		memcpy(*iccprofile, buffer, len);
		free(buffer);
		*icclength = len;
	}
	else {
		*iccprofile = NULL;
		*icclength = 0;
	}

	jpeg_start_decompress(&cinfo);


	row_stride = cinfo.output_width * cinfo.output_components;
	img = new char[cinfo.image_height * row_stride];
	dst = (JSAMPROW) img;

	while (cinfo.output_scanline < cinfo.output_height) {
		jpeg_read_scanlines(&cinfo, &dst, 1);
		dst += row_stride;
	}


	jpeg_finish_decompress(&cinfo);

	*width = cinfo.image_width;
	*height = cinfo.image_height;
	*numcolors = cinfo.output_components;

	jpeg_destroy_decompress(&cinfo);

	fclose(infile);
	
	return img;

}






bool _writeJPEG(const char *filename, 
			char *imagedata, 
			unsigned width, 
			unsigned height, 
			unsigned numcolors,
			unsigned numbits,
			std::map<std::string,std::string> info,
			std::string params,
			char *iccprofile, 
			unsigned iccprofilelength)
{
	if (numbits != 8) return false;

	std::map<std::string,std::string> p = parseparams(params);

	info["UserComment"] = "This, is foo.";

	struct jpeg_compress_struct cinfo;
	struct jpeg_error_mgr jerr;

	unsigned char *exif_data;
	unsigned int exif_data_len;

	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_compress(&cinfo);

	unsigned char * marker;
	unsigned markerlength;

	FILE * outfile;
	unsigned row_stride;
	JSAMPROW dst;

	if ((outfile = fopen(filename, "wb")) == NULL) {
	    fprintf(stderr, "can't open %s\n", filename);
	    //exit(1);
		return false;
	}
	jpeg_stdio_dest(&cinfo, outfile);

	cinfo.image_width = width; 	/* image width and height, in pixels */
	cinfo.image_height = height;
	cinfo.input_components = numcolors;	/* # of color components per pixel */
	cinfo.in_color_space = JCS_RGB; /* colorspace of input image */

	jpeg_set_defaults(&cinfo);

	if (p.find("quality") != p.end()) 
		jpeg_set_quality(&cinfo, atoi(p["quality"].c_str()), TRUE);

	jpeg_start_compress(&cinfo, TRUE);

	marker =  construct_APP1marker(info, &markerlength);
	jpeg_write_marker(&cinfo, JPEG_APP0+1, marker+2, markerlength);
	delete marker;

	if (iccprofile) write_icc_profile (&cinfo, (const JOCTET *) iccprofile, iccprofilelength);

	row_stride = cinfo.image_width * cinfo.input_components;
	dst = (JSAMPROW) imagedata;

	while (cinfo.next_scanline < cinfo.image_height) {
		jpeg_write_scanlines(&cinfo, &dst, 1);
		dst += row_stride;
	}

	jpeg_finish_compress(&cinfo);

	jpeg_destroy_compress(&cinfo);

	fclose(outfile);
	
	return true;

}



