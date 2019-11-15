#ifndef __PICPROCESSORCACORRECT_H__
#define __PICPROCESSORCACORRECT_H__

#include "PicProcessor.h"


class PicProcessorCACorrect: public PicProcessor
{
	public:
		PicProcessorCACorrect(wxString name, wxString command, wxTreeCtrl *tree, PicPanel *display);
		void createPanel(wxSimplebook* parent);
		bool processPicture(gImage *processdib);
};

#endif
