#ifndef __PICPROCESSORGRAY_H__
#define __PICPROCESSORGRAY_H__

#include <vector>

#include "PicProcessor.h"

class PicProcessorGray: public PicProcessor
{
	public:
		PicProcessorGray(wxString name, wxString command, wxTreeCtrl *tree, PicPanel *display);
		void createPanel(wxSimplebook* parent);
		bool processPicture(gImage *processdib);

	private:
		double rp, bp, gp;

};

#endif
