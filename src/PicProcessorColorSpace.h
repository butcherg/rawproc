#ifndef __PICPROCESSORCOLORSPACE_H__
#define __PICPROCESSORCOLORSPACE_H__

#include "PicProcessor.h"


class PicProcessorColorSpace: public PicProcessor
{
	public:
		PicProcessorColorSpace(wxString name, wxString command, wxTreeCtrl *tree, PicPanel *display);
		void createPanel(wxSimplebook* parent);
		bool processPic(bool processnext=true);
};

#endif
