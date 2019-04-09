#ifndef __GIMAGE_CMD_H__
#define __GIMAGE_CMD_H__

#include "gimage/gimage.h"
#include <string>

std::string do_cmd(gImage &dib, std::string commandstr, std::string outfile=std::string(), bool print=true);

#endif
