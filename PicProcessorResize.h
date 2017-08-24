#ifndef __PICPROCESSORRESIZE_H__
#define __PICPROCESSORRESIZE_H__

#include "PicProcessor.h"


class PicProcessorResize: public PicProcessor
{
	public:
		PicProcessorResize(wxString name, wxString command, wxTreeCtrl *tree, PicPanel *display);
		void createPanel(wxSimplebook* parent);
		bool processPic(bool processnext=true);
};

#endif
