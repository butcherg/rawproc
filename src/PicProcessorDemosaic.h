#ifndef __PICPROCESSORDEMOSAIC_H__
#define __PICPROCESSORDEMOSAIC_H__

#include "PicProcessor.h"


class PicProcessorDemosaic: public PicProcessor
{
	public:
		PicProcessorDemosaic(wxString name, wxString command, wxTreeCtrl *tree, PicPanel *display);
		void createPanel(wxSimplebook* parent);
		bool processPic(bool processnext=true);
};

#endif
