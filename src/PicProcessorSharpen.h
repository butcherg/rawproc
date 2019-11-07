#ifndef __PICPROCESSORSHARPEN_H__
#define __PICPROCESSORSHARPEN_H__

#include "PicProcessor.h"


class PicProcessorSharpen: public PicProcessor
{
	public:
		PicProcessorSharpen(wxString name, wxString command, wxTreeCtrl *tree, PicPanel *display);
		void createPanel(wxSimplebook* parent, PicProcessor* proc);
		bool processPicture(gImage *processdib);
};

#endif
