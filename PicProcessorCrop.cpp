
#include "PicProcessor.h"
#include "PicProcessorCrop.h"
#include "PicProcPanel.h"
#include "FreeImage.h"
#include "FreeImage16.h"
#include "util.h"

class CropPanel: public PicProcPanel
{
	public:
		CropPanel(wxPanel *parent, PicProcessorCrop *proc, wxString params): PicProcPanel(parent, (PicProcessor *) proc, params)
		{
			wxSizerFlags flags = wxSizerFlags().Left().Border(wxLEFT|wxRIGHT).Expand();
			panel = new wxPanel(this);
			b->Add(panel, flags);
			SetSizerAndFit(b);
			b->Layout();
			Refresh();
			Update();
			SetFocus();
		}

		~CropPanel()
		{
			q->processPic();
			((PicProcessorCrop *) q)->CropMode(false);
			panel->~wxPanel();
		}


	private:
		wxPanel *panel;
		//wxTouchSlider *slide;

};


PicProcessorCrop::PicProcessorCrop(wxString name, wxString command, wxTreeCtrl *tree, PicPanel *display, wxPanel *parameters): PicProcessor(name, command,  tree, display, parameters) 
{
	showParams();
	m_display->Bind(wxEVT_SCROLL_THUMBRELEASE,&PicProcessorCrop::paramChanged, this);
}

PicProcessorCrop::PicProcessorCrop(wxString name, wxTreeCtrl *tree, PicPanel *display, wxPanel *parameters): PicProcessor(name, "",  tree, display, parameters)
{
	//FIBITMAP *prev
	previous = FreeImage_Clone(getPreviousPicProcessor()->getProcessedPic());
	wxString command = wxString::Format("0,0,%d,%d",FreeImage_GetWidth(previous), FreeImage_GetHeight(previous));
	setParams(command);
	showParams();
	m_display->Bind(wxEVT_SCROLL_THUMBRELEASE,&PicProcessorCrop::paramChanged, this);
}


PicProcessorCrop::~PicProcessorCrop()
{
	if (previous) FreeImage_Unload(previous); 
	m_display->DeletePendingEvents();
	m_display->Unbind(wxEVT_SCROLL_THUMBRELEASE,&PicProcessorCrop::paramChanged,this);
	m_display->CropMode(false);
}


void PicProcessorCrop::showParams()
{
	if (!m_parameters) return;
	m_parameters->DestroyChildren();
	r = new CropPanel(m_parameters, this, c);
	displayProcessedPic();
}

void PicProcessorCrop::paramChanged(wxCommandEvent& event)
{
	setParams(event.GetString());
	processPic();
}

void PicProcessorCrop::CropMode(bool c)
{
	m_display->CropMode(c);
}

void PicProcessorCrop::displayProcessedPic() 
{
	if (m_display) {
		if (previous) {
			m_display->FitMode(false);
			m_display->SetPic(getPreviousPicProcessor()->getProcessedPic());
			m_display->SetCropParams(getParams());
			m_display->SetScaleToWidth(0.9);
			m_display->CropMode(true);
		}
	}
}

bool PicProcessorCrop::processPic() {
	((wxFrame*) m_parameters->GetParent())->SetStatusText("crop...");
	m_tree->SetItemBold(GetId(), true);
	bool result = true;

	wxArrayString cp = split(getParams(),",");
	int left =  atoi(cp[0]);
	int top =  atoi(cp[1]);
	int right = left + atoi(cp[2]);
	int bottom = top + atoi(cp[3]);
	if (previous) FreeImage_Unload(previous);
	previous = FreeImage_Clone(getPreviousPicProcessor()->getProcessedPic());

	if (previous) {
		int bpp = FreeImage_GetBPP(previous);
		if (bpp == 48 |bpp == 24 | bpp == 32) {
			if (dib) FreeImage_Unload(dib);
			dib = FreeImage_Copy(previous, left, top, right, bottom);
			if (!dib) wxMessageBox(wxString::Format("crop failed, %s",getParams()));
		}
		else result = false; 

		//put in every processPic()...
		//if (m_tree->GetItemState(GetId()) == 1) displayProcessedPic();
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



