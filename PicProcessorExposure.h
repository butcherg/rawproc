#ifndef __PICPROCESSOREXPOSURE_H__
#define __PICPROCESSOREXPOSURE_H__

#include "PicProcessor.h"


class PicProcessorExposure: public PicProcessor
{
	public:
		PicProcessorExposure(wxString name, wxString command, wxTreeCtrl *tree, PicPanel *display);
		void createPanel(wxSimplebook* parent);
		bool processPic(bool processnext=true);
};

#endif
