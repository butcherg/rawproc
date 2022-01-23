#ifndef __PICPROCESSORGMIC_H__
#define __PICPROCESSORGMIC_H__

#include "PicProcessor.h"


class PicProcessorGMIC: public PicProcessor
{
	public:
		PicProcessorGMIC(wxString name, wxString command, wxTreeCtrl *tree, PicPanel *display);
		void createPanel(wxSimplebook* parent);
		//void selectFile();
		bool processPicture(gImage *processdib);
		void setSource(wxString src);
		wxString getSource();

	private:
		wxString source;
};

#endif
