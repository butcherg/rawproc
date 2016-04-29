#ifndef __PICPROCESSORROTATE_H__
#define __PICPROCESSORROTATE_H__

#include "PicProcessor.h"


class PicProcessorRotate: public PicProcessor
{
	public:
		PicProcessorRotate(wxString name, wxString command, wxTreeCtrl *tree, PicPanel *display, wxPanel *parameters);
		void showParams();
		bool processPic();
};

#endif
