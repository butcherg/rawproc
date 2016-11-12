#ifndef __PICPROCESSOR_H__
#define __PICPROCESSOR_H__

#ifndef WX_PRECOMP
	#include <wx/wx.h>
	#include <wx/frame.h>
#else
	#include <wx/wxprec.h>
#endif

#include <wx/image.h>
#include <wx/treectrl.h>
#include <wx/sizer.h>
#include <wx/event.h>
//#include <wx/thread.h>

#include "PicPanel.h"
#include "PicProcPanel.h"
#include <gimage.h>

class PicProcPanel;

class PicPanel;

class PicProcessor: public wxTreeItemData//, public wxEvtHandler
{

	public:
		PicProcessor(wxString name, wxString command, wxTreeCtrl *tree, PicPanel *display, wxPanel *parameters, gImage& startipc);
		//PicProcessor(PicProcessor * copyof);
		~PicProcessor();
		virtual bool processPic();
		wxString getCommand();
		wxString getParams();
		wxString getName();
		virtual void showParams();
		PicProcessor *getPreviousPicProcessor();
		gImage& getProcessedPic();
		PicPanel *getDisplay();
		wxTreeCtrl *getCommandTree();
		virtual void displayProcessedPic();
		virtual void setParams(wxString params);

//	private:
		gImage dib;
		PicPanel *m_display;
		
		wxTreeCtrl *m_tree;
		wxString n, c;

		wxPanel *m_parameters;
		PicProcPanel *r;

		bool dirty;

};


#endif
