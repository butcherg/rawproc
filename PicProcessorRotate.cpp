#include "PicProcessor.h"
#include "PicProcessorRotate.h"
#include "PicProcPanel.h"
#include "FreeImage.h"
#include "FreeImage16.h"
#include "myTouchSlider.h"

class RotatePanel: public PicProcPanel
{
	public:
		RotatePanel(wxPanel *parent, PicProcessor *proc, wxString params): PicProcPanel(parent, proc, params)
		{
			SetSize(parent->GetSize());
			b->SetOrientation(wxHORIZONTAL);
			wxSizerFlags flags = wxSizerFlags().Center().Border(wxLEFT|wxRIGHT|wxTOP|wxBOTTOM);
			slide = new myTouchSlider((wxFrame *) this, wxID_ANY, "rotate", SLIDERWIDTH, atof(p.c_str()), 1.0, -90.0, 90.0, "%2.0f");
			b->Add(100,100,1);
			b->Add(slide, flags);
			b->Add(100,100,1);
			SetSizerAndFit(b);
			b->Layout();
			Refresh();
			Update();
			SetFocus();
			Connect(wxID_ANY, wxEVT_SCROLL_THUMBRELEASE,wxCommandEventHandler(RotatePanel::paramChanged));
		}

		~RotatePanel()
		{
			slide->~myTouchSlider();
		}

		void paramChanged(wxCommandEvent& event)
		{
			q->setParams(wxString::Format("%2.0f",slide->GetDoubleValue()));
			q->processPic();
			event.Skip();
		}


	private:
		myTouchSlider *slide;

};


PicProcessorRotate::PicProcessorRotate(wxString name, wxString command, wxTreeCtrl *tree, PicPanel *display, wxPanel *parameters): PicProcessor(name, command,  tree, display, parameters) 
{
	//p->DestroyChildren();
	//r = new RotatePanel(p,this,c);
	showParams();
}

void PicProcessorRotate::showParams()
{
	if (!m_parameters) return;
	m_parameters->DestroyChildren();
	r = new RotatePanel(m_parameters, this, c);
}


bool PicProcessorRotate::processPic() {
	((wxFrame*) m_parameters->GetParent())->SetStatusText("rotate...");
	double angle = atof(c.c_str());
	bool result = true;

	if (dib) FreeImage_Unload(dib);
	dib = FreeImage_Rotate(getPreviousPicProcessor()->getProcessedPic(), angle);
	dirty = false;

	//put in every processPic()...
	if (m_tree->GetItemState(GetId()) == 1) m_display->SetPic(dib);
	wxTreeItemId next = m_tree->GetNextSibling(GetId());
	if (next.IsOk()) {
		PicProcessor * nextitem = (PicProcessor *) m_tree->GetItemData(next);
		nextitem->processPic();
	}

	((wxFrame*) m_parameters->GetParent())->SetStatusText("");
	return result;
}



