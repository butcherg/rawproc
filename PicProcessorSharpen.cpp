#include "PicProcessor.h"
#include "PicProcessorSharpen.h"
#include "PicProcPanel.h"
#include "FreeImage.h"
#include "FreeImage16.h"
#include "myTouchSlider.h"

#include <vector>

class SharpenPanel: public PicProcPanel
{
	public:
		SharpenPanel(wxPanel *parent, PicProcessor *proc, wxString params): PicProcPanel(parent, proc, params)
		{
			SetSize(parent->GetSize());
			b->SetOrientation(wxHORIZONTAL);
			wxSizerFlags flags = wxSizerFlags().Center().Border(wxLEFT|wxRIGHT|wxTOP|wxBOTTOM);
			slide = new myTouchSlider((wxFrame *) this, wxID_ANY, "sharpen", SLIDERWIDTH, atof(p.c_str()), 1.0, 1.0, 10.0, "%2.0f");
			b->Add(100,100,1);
			b->Add(slide, flags);
			b->Add(100,100,1);
			SetSizerAndFit(b);
			b->Layout();
			Refresh();
			Update();
			SetFocus();
			Connect(wxID_ANY, wxEVT_SCROLL_THUMBRELEASE,wxCommandEventHandler(SharpenPanel::paramChanged));
		}

		~SharpenPanel()
		{
			slide->~myTouchSlider();
		}

		void paramChanged(wxCommandEvent& event)
		{
			q->setParams(wxString::Format("%d",slide->GetIntValue()));
			q->processPic();
			event.Skip();
		}


	private:
		myTouchSlider *slide;

};


PicProcessorSharpen::PicProcessorSharpen(wxString name, wxString command, wxTreeCtrl *tree, PicPanel *display, wxPanel *parameters): PicProcessor(name, command,  tree, display, parameters) 
{
	//p->DestroyChildren();
	//r = new SharpenPanel(p,this,c);
	showParams();
	//Bind(wxEVT_THREAD, &PicProcessorSharpen::endProcessPic, this);
}

void PicProcessorSharpen::showParams()
{
	if (!m_parameters) return;
	m_parameters->DestroyChildren();
	r = new SharpenPanel(m_parameters, this, c);
}


bool PicProcessorSharpen::processPic() {
	double kernel[3][3] =
	{
		0.0, 0.0, 0.0,
		0.0, 0.0, 0.0,
		0.0, 0.0, 0.0
	};

	std::vector<ConvolveThread *> t;

	m_tree->SetItemBold(GetId(), true);
	((wxFrame*) m_parameters->GetParent())->SetStatusText("sharpen...");
	double sharp = atof(c.c_str());
	double x = -((sharp-1)/4.0);
	kernel[0][1] = x;
	kernel[1][0] = x;
	kernel[1][2] = x;
	kernel[2][1] = x;
	kernel[1][1] = sharp;
	bool result = true;

	threadcount = wxThread::GetCPUCount();
	if (threadcount > 16) threadcount = 16;
	if (dib) FreeImage_Unload(dib);
	dib = FreeImage_Clone(getPreviousPicProcessor()->getProcessedPic());

	for (int i=0; i<threadcount; i++) {
		//ConvolveThread c(getPreviousPicProcessor()->getProcessedPic(), dib, i,threadcount, kernel);
		//t.push_back(c);
		t.push_back(new ConvolveThread(getPreviousPicProcessor()->getProcessedPic(), dib, i,threadcount, kernel));
		t.back()->Run();
	}
	while (!t.empty()) {
		t.back()->Wait(wxTHREAD_WAIT_BLOCK);
		t.pop_back();
	}
	dirty = false;

	//put in every processPic()...
	if (m_tree->GetItemState(GetId()) == 1) m_display->SetPic(dib);
	m_tree->SetItemBold(GetId(), false);
	wxTreeItemId next = m_tree->GetNextSibling(GetId());
	if (next.IsOk()) {
		PicProcessor * nextitem = (PicProcessor *) m_tree->GetItemData(next);
		nextitem->processPic();
	}
	m_tree->SetItemBold(GetId(), false);
	((wxFrame*) m_parameters->GetParent())->SetStatusText("");

	return result;
}




