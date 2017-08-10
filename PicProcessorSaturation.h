#ifndef __PICPROCESSORSATURATION_H__
#define __PICPROCESSORSATURATION_H__

#include "PicProcessor.h"


class PicProcessorSaturation: public PicProcessor
{
	public:
		PicProcessorSaturation(wxString name, wxString command, wxTreeCtrl *tree, PicPanel *display, wxPanel *parameters);
		void createPanel(wxSimplebook* parent);
		void showParams();
		bool processPic();
};

#endif
