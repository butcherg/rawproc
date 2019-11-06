#ifndef __PICPROCESSORGROUP_H__
#define __PICPROCESSORGROUP_H__

#include "PicProcessor.h"


class PicProcessorGroup: public PicProcessor
{
	public:
		PicProcessorGroup(wxString name, wxString command, wxTreeCtrl *tree, PicPanel *display);
		void createPanel(wxSimplebook* parent);
		void selectFile();
		bool processPicture(gImage *processdib);
		void loadCommands(wxString commandstring);
		void setSource(wxString src);
		wxString getSource();

	private:
		wxString source;
};

#endif
