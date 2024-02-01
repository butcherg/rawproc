#ifndef __PICPROCESSORLENSVIGNETTING_H__
#define __PICPROCESSORLENSVIGNETTING_H__

#include "PicProcessor.h"


class PicProcessorLensVignetting: public PicProcessor
{
	public:
		PicProcessorLensVignetting(wxString name, wxString command, wxTreeCtrl *tree, PicPanel *display);
		void createPanel(wxSimplebook* parent);
		bool processPicture(gImage *processdib);

};

#endif
