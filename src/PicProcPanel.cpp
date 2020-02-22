
#include "PicProcPanel.h"
#include "myConfig.h"
#include "util.h"


PicProcPanel::PicProcPanel(wxWindow *parent, PicProcessor *proc, wxString params): 
	wxScrolledWindow(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxVSCROLL) 
	//wxPanel(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, !wxTAB_TRAVERSAL) 
{
	int fr=0, fg=0, fb=0;
	q = proc;
	p = params;
	rateAdapt = false;
	
	//parm app.parameters.fontsize: Integer font size for paramter pane dialogs.  Default=10
	int fontsize = atoi(myConfig::getConfig().getValueOrDefault("app.parameters.fontsize","10").c_str());
	//parm app.parameters.fontcolor: integer RGB triplet or single of 0-255 (e.g., 128,128,128 or just 128 for gray) to specify font color to be used in the parameter pane.  Default=0
	wxString fc = wxString(myConfig::getConfig().getValueOrDefault("app.parameters.fontcolor","0"));
	if (fc == "") fc = "0";
	wxArrayString fntc = split(fc,",");
	fr = atoi(fntc[0].c_str());
	if (fntc.GetCount() < 3) {
		fg = atoi(fntc[0].c_str());
		fb = atoi(fntc[0].c_str());
	}
	else if (fntc.GetCount() == 3) {
		fg = atoi(fntc[1].c_str());
		fb = atoi(fntc[2].c_str());
	}

	FitInside();
        SetScrollRate(5, 5);
	
	SetForegroundColour(wxColour(fr,fg,fb));
	wxFont font(wxFontInfo(fontsize).Family(wxFONTFAMILY_SWISS));
	SetFont(font);
	SetSize(parent->GetSize());

	SetBackgroundColour(parent->GetBackgroundColour());
	Bind(wxEVT_LEFT_DOWN, &PicProcPanel::OnLeftDown, this);
}

//This OnKey() is specifically organized to trap the arrow keys, so they don't participate
//in pane switching...
void PicProcPanel::OnKey(wxKeyEvent& event)
{
	wxChar uc = event.GetUnicodeKey();
	if ( uc != WXK_NONE )
	{
		// It's a "normal" character. Notice that this includes
		// control characters in 1..31 range, e.g. WXK_RETURN or
		// WXK_BACK, so check for them explicitly.
		if ( uc >= 32 )
		{
			switch (uc) {
			}
		}
		else
		{
			// It's a control character, < WXK_START
			switch (uc)
			{
				case WXK_TAB:
					event.Skip();
					break;
			}
		}
		event.Skip();
	}
	else // No Unicode equivalent.
	{
		// It's a special key, > WXK_START, deal with all the known ones:
		switch ( event.GetKeyCode() )
		{
		}
	}
	event.Skip();
}



void PicProcPanel::setRateAdapt(bool r)
{
	rateAdapt = r;
}

void PicProcPanel::OnSize(wxSizeEvent& event) 
{
//	if (b) {
//		Refresh();
//	}
	event.Skip();
}

void PicProcPanel::OnLeftDown(wxMouseEvent& event)
{
	SetFocus();
	Refresh();
}

wxString PicProcPanel::getParams()
{
	return p;
}



