#ifndef __PICPROCESSORBANDING_H__
#define __PICPROCESSORBANDING_H__

#include "PicProcessor.h"


class PicProcessorBanding: public PicProcessor
{
	public:
		PicProcessorBanding(wxString name, wxString command, wxTreeCtrl *tree, PicPanel *display);
		void createPanel(wxSimplebook* parent);
		bool processPicture(gImage *processdib);
};

#endif
