#ifndef __PICPROCESSORGAMMA_H__
#define __PICPROCESSORGAMMA_H__

#include "PicProcessor.h"


class PicProcessorGamma: public PicProcessor
{
	public:
		PicProcessorGamma(wxString name, wxString command, wxTreeCtrl *tree, PicPanel *display);
		void createPanel(wxSimplebook* parent);
		bool processPic();
};

#endif
