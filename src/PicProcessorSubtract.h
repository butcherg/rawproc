#ifndef __PICPROCESSORSUBTRACT_H__
#define __PICPROCESSORSUBTRACT_H__

#include "PicProcessor.h"


class PicProcessorSubtract: public PicProcessor
{
	public:
		PicProcessorSubtract(wxString name, wxString command, wxTreeCtrl *tree, PicPanel *display);
		void createPanel(wxSimplebook* parent);
		bool processPic(bool processnext=true);
};

#endif
