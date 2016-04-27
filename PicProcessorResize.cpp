#include "PicProcessor.h"
#include "PicProcessorResize.h"
#include "PicProcPanel.h"
#include "FreeImage.h"
#include "FreeImage16.h"
#include "util.h"

class ResizePanel: public PicProcPanel
{
	public:
		ResizePanel(wxPanel *parent, PicProcessor *proc, wxString params): PicProcPanel(parent, proc, params)
		{
			wxSizerFlags flags = wxSizerFlags().Left().Border(wxLEFT|wxRIGHT|wxTOP|wxBOTTOM).Expand();
			wxArrayString algos;
			algos.Add("box");
			algos.Add("bilinear");
			algos.Add("bspline");
			algos.Add("bicubic");
			algos.Add("catmullrom");
			algos.Add("lanczos3");
			wxArrayString p = split(params,",");
			b->Add(new wxStaticText(this,-1, "width (0 for original aspect)", wxDefaultPosition, wxSize(200,20)),  flags);
			widthedit = new wxTextCtrl(this, wxID_ANY, p[0], wxDefaultPosition, wxSize(100,20),wxTE_PROCESS_ENTER);
			b->Add(widthedit, flags);
			b->Add(new wxStaticText(this,-1, "height (0 for original aspect)", wxDefaultPosition, wxSize(200,20)), flags);
			heightedit = new wxTextCtrl(this, wxID_ANY, p[1], wxDefaultPosition, wxSize(100,20),wxTE_PROCESS_ENTER);
			b->Add(heightedit, flags);		
			algoselect = new wxRadioBox (this, wxID_ANY, "Resize Algorithm", wxDefaultPosition, wxSize(100,220),  algos, 1, wxRA_SPECIFY_COLS);
			if (p.size() >=3) {
				for (int i=0; i<algos.size(); i++) {
					if (p[2] == algos[i]) algoselect->SetSelection(i);
				}
			}
			b->Add(algoselect, flags);	
			SetSizerAndFit(b);
			b->Layout();
			Refresh();
			Update();
			SetFocus();
			Bind(wxEVT_TEXT_ENTER,&ResizePanel::paramChanged, this);
			Bind(wxEVT_RADIOBOX,&ResizePanel::paramChanged, this);	
		}

		~ResizePanel()
		{
			widthedit->~wxTextCtrl();
			heightedit->~wxTextCtrl();
			algoselect->~wxRadioBox();
		}

		void paramChanged(wxCommandEvent& event)
		{
			q->setParams(wxString::Format("%s,%s,%s",widthedit->GetValue(),heightedit->GetValue(),algoselect->GetString(algoselect->GetSelection())));
			q->processPic();
			event.Skip();
		}


	private:
		wxTextCtrl *widthedit, *heightedit;
		wxRadioBox *algoselect;

};


PicProcessorResize::PicProcessorResize(wxString name, wxString command, wxTreeCtrl *tree, PicPanel *display, wxPanel *parameters): PicProcessor(name, command,  tree, display, parameters) 
{
	//p->DestroyChildren();
	//r = new BrightPanel(p,this,c);
	showParams();
}

void PicProcessorResize::showParams()
{
	if (!m_parameters) return;
	m_parameters->DestroyChildren();
	r = new ResizePanel(m_parameters, this, c);
}


bool PicProcessorResize::processPic() {
	wxString algo = "";
	m_tree->SetItemBold(GetId(), true);
	((wxFrame*) m_parameters->GetParent())->SetStatusText("resize...");
	wxArrayString cp = split(getParams(),",");
	int width =  atoi(cp[0]);
	int height =  atoi(cp[1]);
	if (cp.size() >2) algo  = cp[2];

	bool result = true;
	FIBITMAP *prev = dib;
	dib = FreeImage_Clone(getPreviousPicProcessor()->getProcessedPic());
	if (dib) {

		unsigned dw = FreeImage_GetWidth(dib);
		unsigned dh = FreeImage_GetHeight(dib);
		if (height ==  0) height = dh * ((float)width/(float)dw);
		if (width == 0)  width = dw * ((float)height/(float)dh); 
		FREE_IMAGE_FILTER filter = FILTER_CATMULLROM;
		if (algo == "box") filter = FILTER_BOX;
		if (algo == "bilinear") filter = FILTER_BILINEAR;
		if (algo == "bspline") filter = FILTER_BSPLINE;
		if (algo == "bicubic") filter = FILTER_BICUBIC;
		if (algo == "catmullrom") filter = FILTER_CATMULLROM;
		if (algo == "lanczos3") filter = FILTER_LANCZOS3;

		int bpp = FreeImage_GetBPP(dib);
		if (bpp == 8 |bpp == 24 | bpp == 32 | bpp == 48) {
			dib = FreeImage_Rescale(dib, width, height, filter);
			dirty = false;
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
	//else {	
	//	result = false;
	//}
	m_tree->SetItemBold(GetId(), false);
	((wxFrame*) m_parameters->GetParent())->SetStatusText("");
	return result;
}



