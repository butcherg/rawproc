#ifndef __PICPROCESSORSCRIPT_H__
#define __PICPROCESSORSCRIPT_H__

#include "PicProcessor.h"


class PicProcessorScript: public PicProcessor
{
	public:
		PicProcessorScript(wxString name, wxString command, wxTreeCtrl *tree, wxString imagefile, PicPanel *display);
		void createPanel(wxSimplebook* parent);
		//void selectFile();
		bool processPicture(gImage *processdib);
		void setSource(wxString src);
		wxString getSource();
		void setImageName(wxString img);
		wxString getImageName();
		void SetMenuString(wxString pgm);

	private:
		wxString source, img;
};

#endif
