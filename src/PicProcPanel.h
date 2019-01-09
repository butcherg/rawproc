#ifndef __PICPROCPANEL_H__
#define __PICPROCPANEL_H__

#ifdef __BORLANDC__
	#pragma hdrstop
#endif

#ifndef WX_PRECOMP
	#include <wx/wx.h>
	#include <wx/frame.h>
#else
	#include <wx/wxprec.h>
#endif

#include <wx/gbsizer.h>
#include <wx/statline.h>

#include "PicProcessor.h"
#define SLIDERWIDTH 70

class PicProcessor;

class PicProcPanel: public wxScrolledWindow //wxPanel
{

	public:
		PicProcPanel(wxWindow *parent, PicProcessor *proc, wxString params);
		~PicProcPanel();
		wxString getParams();
		void setRateAdapt(bool r);
		virtual void SetPic(PicProcessor *proc) {}
		void OnSize(wxSizeEvent& event);
		void OnLeftDown(wxMouseEvent& event);

	protected:
		PicProcessor *q;
		wxBoxSizer *b;
		wxGridBagSizer *g;
		wxString p;
		bool rateAdapt;

};



#endif
