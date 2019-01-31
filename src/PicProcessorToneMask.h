#ifndef __PICPROCESSORTONEMASK_H__
#define __PICPROCESSORTONEMASK_H__

#include "PicProcessor.h"


class PicProcessorToneMask: public PicProcessor
{
	public:
		PicProcessorToneMask(wxString name, wxString command, wxTreeCtrl *tree, PicPanel *display);
		void createPanel(wxSimplebook* parent);
		bool processPic(bool processnext=true);
};

#endif
