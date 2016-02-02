
#include "PicProcessor.h"
#include "PicProcessorGray.h"
#include "PicProcPanel.h"
#include "FreeImage.h"
#include "FreeImage16.h"
#include "myTouchSlider.h"
#include "util.h"

#define REDSLIDER   7000
#define GREENSLIDER 7001
#define BLUESLIDER  7002

class GrayPanel: public PicProcPanel
{

//blank panel for now, until I implement a channel mixer...
	public:
		GrayPanel(wxPanel *parent, PicProcessor *proc, wxString params): PicProcPanel(parent, proc, params)
		{
			wxSizerFlags flags = wxSizerFlags().Left().Border(wxLEFT|wxRIGHT).Expand();
			wxArrayString p = split(params,",");

			//wxScrolledWindow *sw = new wxScrolledWindow(this);
			//sw->SetScrollRate( 5, 5 );
			//b->Add(sw, flags);

			//wxBoxSizer *bc;

			b->Add(new wxStaticText(this,-1, "Red", wxDefaultPosition, wxSize(100,20)), flags);
			redslide = new myTouchSlider((wxFrame *) this, REDSLIDER, "Red", atof(p[0]), 0.01, 0.0, 1.0, "%2.2f");
			b->Add(redslide, flags);
			b->Add(new wxStaticText(this,-1, "Green", wxDefaultPosition, wxSize(100,20)), flags);
			greenslide = new myTouchSlider((wxFrame *) this, GREENSLIDER, "Green", atof(p[1]), 0.01, 0.0, 1.0, "%2.2f");
			b->Add(greenslide, flags);
			b->Add(new wxStaticText(this,-1, "Blue", wxDefaultPosition, wxSize(100,20)), flags);
			blueslide = new myTouchSlider((wxFrame *) this, BLUESLIDER, "Blue", atof(p[2]), 0.01, 0.0, 1.0, "%2.2f");
			b->Add(blueslide, flags);
			
			//sw->SetSizerAndFit(bc);
			//bc->Layout();

			SetSizerAndFit(b);
			b->Layout();
			Refresh();
			Update();
			SetFocus();
			Connect(wxID_ANY, wxEVT_SCROLL_THUMBRELEASE,wxCommandEventHandler(GrayPanel::paramChanged));
		}

		~GrayPanel()
		{
			//panel->~wxPanel();
			redslide->~myTouchSlider();
		}

		void paramChanged(wxCommandEvent& event)
		{
			double r = redslide->GetDoubleValue();
			double g = greenslide->GetDoubleValue();
			double b = blueslide->GetDoubleValue();
			q->setParams(wxString::Format("%0.2f,%0.2f,%0.2f",r,g,b));
			event.Skip();
		}


	private:
		//wxPanel *panel;
		myTouchSlider *redslide, *greenslide, *blueslide;

};


PicProcessorGray::PicProcessorGray(wxString name, wxString command, wxTreeCtrl *tree, PicPanel *display, wxPanel *parameters): PicProcessor(name, command,  tree, display, parameters) 
{
	showParams();
}

void PicProcessorGray::showParams()
{
	if (!m_parameters) return;
	m_parameters->DestroyChildren();
	r = new GrayPanel(m_parameters, this, c);
}

bool PicProcessorGray::processPic() {
	((wxFrame*) m_parameters->GetParent())->SetStatusText("gray...");
	m_tree->SetItemBold(GetId(), true);
	wxArrayString cp = split(getParams(),",");
	double r = atof(cp[0]);
	double g = atof(cp[1]);
	double b = atof(cp[2]);
	bool result = true;
	FIBITMAP *prev = dib;
	dib = FreeImage_Clone(getPreviousPicProcessor()->getProcessedPic());
	if (dib) {
		int bpp = FreeImage_GetBPP(dib);
		if (bpp == 48 |bpp == 24 | bpp == 32) {
			//if (!FreeImage_Gray16(dib, 0.21, 0.72, 0.07)) {
			if (!FreeImage_Gray16(dib, r, g, b)) {
				result = false;
			}
		}
		else result = false; 
		if (prev) FreeImage_Unload(prev);

		//put in every processPic()...
		if (m_tree->GetItemState(GetId()) == 1) m_display->SetPic(dib);
		m_tree->SetItemBold(GetId(), false);
		wxTreeItemId next = m_tree->GetNextSibling(GetId());
		if (next.IsOk()) {
			PicProcessor * nextitem = (PicProcessor *) m_tree->GetItemData(next);
			nextitem->processPic();
		}
	}
	else result = false;
	m_tree->SetItemBold(GetId(), false);
	((wxFrame*) m_parameters->GetParent())->SetStatusText("");
	return result;
}



