
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
			SetSize(parent->GetSize());
			int sliderwidth = 70;
			wxSizerFlags flags = wxSizerFlags().Left().Border(wxLEFT|wxRIGHT|wxTOP|wxBOTTOM).Expand();
			wxArrayString p = split(params,",");

			rd = atof(p[0]);
			gr = atof(p[1]);
			bl = atof(p[2]);
			b->SetOrientation(wxHORIZONTAL);

			//b->AddStretchSpacer(1);
			b->Add(20,0,1,wxEXPAND);
			redslide = new myTouchSlider((wxFrame *) this, REDSLIDER, "Red", sliderwidth, atof(p[0]), 0.01, 0.0, 1.0, "%2.2f");
			b->Add(redslide, flags);
			greenslide = new myTouchSlider((wxFrame *) this, GREENSLIDER, "Green", sliderwidth, atof(p[1]), 0.01, 0.0, 1.0, "%2.2f");
			b->Add(greenslide, flags);
			blueslide = new myTouchSlider((wxFrame *) this, BLUESLIDER, "Blue", sliderwidth, atof(p[2]), 0.01, 0.0, 1.0, "%2.2f");
			b->Add(blueslide, flags);
			b->AddStretchSpacer(1);

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
		}

		void paramChanged(wxCommandEvent& event)
		{
			double r = redslide->GetDoubleValue();
			double g = greenslide->GetDoubleValue();
			double b = blueslide->GetDoubleValue();
	 		double dr = rd-r; double dg = gr-g; double db = bl - b;
			if (dr != 0.0) { greenslide->SetValue(gr+(dr/2)); blueslide->SetValue(bl+(dr/2)); }
			else if (dg != 0.0) { redslide->SetValue(rd+(dg/2)); blueslide->SetValue(bl+(dg/2)); }
			else if (db != 0.0) { redslide->SetValue(rd+(db/2)); greenslide->SetValue(gr+(db/2)); }
			rd = redslide->GetDoubleValue();
			gr = greenslide->GetDoubleValue();
			bl = blueslide->GetDoubleValue();
			q->setParams(wxString::Format("%0.2f,%0.2f,%0.2f",rd,gr,bl));
			q->processPic();
			Refresh();
			Update();
			event.Skip();
		}


	private:
		//wxPanel *panel;
		myTouchSlider *redslide, *greenslide, *blueslide;
		double rd, gr, bl;

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



