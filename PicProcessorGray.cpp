
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

	public:
		GrayPanel(wxPanel *parent, PicProcessor *proc, wxString params): PicProcPanel(parent, proc, params)
		{
			c = new wxBoxSizer(wxHORIZONTAL); 
			SetSize(parent->GetSize());
			wxSizerFlags flags = wxSizerFlags().Left().Border(wxLEFT|wxRIGHT|wxTOP|wxBOTTOM).Expand();
			wxArrayString p = split(params,",");

			rd = atof(p[0]);
			gr = atof(p[1]);
			bl = atof(p[2]);
			//b->SetOrientation();

			//b->AddStretchSpacer(1);
			c->Add(15,0,1,wxEXPAND);
			redslide = new myTouchSlider((wxFrame *) this, REDSLIDER, "Red", SLIDERWIDTH, atof(p[0]), 0.01, 0.0, 1.0, "%2.2f");
			c->Add(redslide, flags);
			greenslide = new myTouchSlider((wxFrame *) this, GREENSLIDER, "Green", SLIDERWIDTH, atof(p[1]), 0.01, 0.0, 1.0, "%2.2f");
			c->Add(greenslide, flags);
			blueslide = new myTouchSlider((wxFrame *) this, BLUESLIDER, "Blue", SLIDERWIDTH, atof(p[2]), 0.01, 0.0, 1.0, "%2.2f");
			c->Add(blueslide, flags);
			c->AddStretchSpacer(1);
			c->Layout();

			b->Add(c,flags);

			t = new wxStaticText(this,-1, wxString::Format("Total: %2.2f", rd+gr+bl), wxDefaultPosition, wxSize(100,20));
			b->Add(t, flags);

			SetSizerAndFit(b);
			b->Layout();
			Refresh();
			Update();
			SetFocus();
			Connect(wxID_ANY, wxEVT_SCROLL_THUMBRELEASE,wxCommandEventHandler(GrayPanel::paramChanged));
		}

		~GrayPanel()
		{
			redslide->~myTouchSlider();
			greenslide->~myTouchSlider();
			blueslide->~myTouchSlider();
			t->~wxStaticText();
		}

		void paramChanged(wxCommandEvent& event)
		{
			rd = redslide->GetDoubleValue();
			gr = greenslide->GetDoubleValue();
			bl = blueslide->GetDoubleValue();
			t->SetLabel(wxString::Format("Total: %2.2f", rd+gr+bl));
			q->setParams(wxString::Format("%2.2f,%2.2f,%2.2f",rd,gr,bl));
			q->processPic();
			Refresh();
			Update();
			event.Skip();
		}


	private:
		myTouchSlider *redslide, *greenslide, *blueslide;
		double rd, gr, bl;
		wxBoxSizer *c;
		wxStaticText *t;

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



