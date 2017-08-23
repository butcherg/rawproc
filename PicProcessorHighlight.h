#ifndef __PICPROCESSORHIGHLIGHT_H__
#define __PICPROCESSORHIGHLIGHT_H__

#include <vector>

#include "PicProcessor.h"

class PicProcessorHighlight: public PicProcessor
{
	public:
		PicProcessorHighlight(wxString name, wxString command, wxTreeCtrl *tree, PicPanel *display, wxPanel *parameters);
		void createPanel(wxSimplebook* parent);
		void showParams();
		bool processPic();

};

#endif
