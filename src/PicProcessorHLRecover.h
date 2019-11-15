#ifndef __PICPROCESSORHLRECOVER_H__
#define __PICPROCESSORHLRECOVER_H__

#include "PicProcessor.h"


class PicProcessorHLRecover: public PicProcessor
{
	public:
		PicProcessorHLRecover(wxString name, wxString command, wxTreeCtrl *tree, PicPanel *display);
		void createPanel(wxSimplebook* parent);
		bool processPicture(gImage *processdib);
};

#endif
