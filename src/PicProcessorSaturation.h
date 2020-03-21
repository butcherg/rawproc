#ifndef __PICPROCESSORSATURATION_H__
#define __PICPROCESSORSATURATION_H__

#include "PicProcessor.h"


class PicProcessorSaturation: public PicProcessor
{
	public:
		PicProcessorSaturation(wxString name, wxString command, wxTreeCtrl *tree, PicPanel *display);
		~PicProcessorSaturation();
		void createPanel(wxSimplebook* parent);
		bool processPicture(gImage *processdib);
		void OnLeftDown(wxMouseEvent& event);
		void OnMouseMove(wxMouseEvent& event);
		void OnLeftUp(wxMouseEvent& event);
};

#endif
