#ifndef __PICPROCESSORBRIGHT_H__
#define __PICPROCESSORBRIGHT_H__

#include "PicProcessor.h"


class PicProcessorBright: public PicProcessor
{
	public:
		PicProcessorBright(wxString name, wxString command, wxTreeCtrl *tree, PicPanel *display, wxPanel *parameters);
		void showParams();
		bool processPic();
};

#endif
