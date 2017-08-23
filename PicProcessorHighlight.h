#ifndef __PICPROCESSORHIGHLIGHT_H__
#define __PICPROCESSORHIGHLIGHT_H__

#include <vector>

#include "PicProcessor.h"

class PicProcessorHighlight: public PicProcessor
{
	public:
		PicProcessorHighlight(wxString name, wxString command, wxTreeCtrl *tree, PicPanel *display);
		void createPanel(wxSimplebook* parent);
		bool processPic();

};

#endif
