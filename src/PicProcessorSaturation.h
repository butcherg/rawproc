#ifndef __PICPROCESSORSATURATION_H__
#define __PICPROCESSORSATURATION_H__

#include "PicProcessor.h"


class PicProcessorSaturation: public PicProcessor
{
	public:
		PicProcessorSaturation(wxString name, wxString command, wxTreeCtrl *tree, PicPanel *display);
		void createPanel(wxSimplebook* parent, PicProcessor* proc);
		bool processPicture(gImage *processdib);
};

#endif
