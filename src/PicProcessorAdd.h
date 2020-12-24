#ifndef __PICPROCESSORADD_H__
#define __PICPROCESSORADD_H__

#include "PicProcessor.h"


class PicProcessorAdd: public PicProcessor
{
	public:
		PicProcessorAdd(wxString name, wxString command, wxTreeCtrl *tree, PicPanel *display);
		void createPanel(wxSimplebook* parent);
		bool processPicture(gImage *processdib);
};

#endif
