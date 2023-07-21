#ifndef __PICPROCESSORSPOT_H__
#define __PICPROCESSORSPOT_H__

#include "PicProcessor.h"


class PicProcessorSpot: public PicProcessor
{
public:
	PicProcessorSpot(wxString name, wxString command, wxTreeCtrl *tree, PicPanel *display);
	~PicProcessorSpot();
	void createPanel(wxSimplebook* parent);
	bool processPicture(gImage *processdib);
	void OnLeftDown(wxMouseEvent& event);
};

#endif
