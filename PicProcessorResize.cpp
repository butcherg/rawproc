#include "PicProcessor.h"
#include "PicProcessorResize.h"
#include "PicProcPanel.h"
#include "myConfig.h"
#include "util.h"

#include <wx/spinctrl.h>

class ResizePanel: public PicProcPanel
{
	public:
		ResizePanel(wxWindow *parent, PicProcessor *proc, wxString params): PicProcPanel(parent, proc, params)
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
			g->Add(new wxStaticText(this,wxID_ANY, "width: "), wxGBPosition(0,0), wxDefaultSpan, wxALIGN_LEFT | wxALL, 3);
			widthedit = new wxSpinCtrl(this, wxID_ANY, p[0], wxDefaultPosition, wxSize(100,25),wxTE_PROCESS_ENTER | wxSP_ARROW_KEYS,0,10000);
			widthedit->SetToolTip("width in pixels, 0 preserves aspect.\nIf you use the spin arrows, type Enter to update the image.");
			g->Add(widthedit, wxGBPosition(0,1), wxDefaultSpan, wxALIGN_LEFT | wxALL, 3);
			g->Add(new wxStaticText(this,-1, "height: "), wxGBPosition(1,0), wxDefaultSpan, wxALIGN_LEFT | wxALL, 3);
			heightedit = new wxSpinCtrl(this, wxID_ANY, p[1], wxDefaultPosition, wxSize(100,25),wxTE_PROCESS_ENTER | wxSP_ARROW_KEYS,0,10000);
			heightedit->SetToolTip("height in pixels, 0 preserves aspect. \nIf you use the spin arrows, type Enter to update the image.");
			g->Add(heightedit, wxGBPosition(1,1), wxDefaultSpan, wxALIGN_LEFT | wxALL, 3);		
			algoselect = new wxRadioBox (this, wxID_ANY, "Resize Algorithm", wxDefaultPosition, wxDefaultSize,  algos, 1, wxRA_SPECIFY_COLS);
			if (p.size() >=3) {
				for (int i=0; i<algos.size(); i++) {
					if (p[2] == algos[i]) algoselect->SetSelection(i);
				}
			}
			g->Add(algoselect, wxGBPosition(2,0), wxGBSpan(1,2), wxALIGN_LEFT | wxALL, 3);	
			SetSizerAndFit(g);
			g->Layout();
			Refresh();
			Update();
			SetFocus();
			Bind(wxEVT_TEXT_ENTER,&ResizePanel::paramChanged, this);
			//Bind(wxEVT_SPINCTRL,&ResizePanel::paramChanged, this);
			Bind(wxEVT_RADIOBOX,&ResizePanel::paramChanged, this);	
		}

		~ResizePanel()
		{

		}

		void paramChanged(wxCommandEvent& event)
		{
			q->setParams(wxString::Format("%d,%d,%s",widthedit->GetValue(),heightedit->GetValue(),algoselect->GetString(algoselect->GetSelection())));
			q->processPic();
			event.Skip();
		}


	private:
		wxSpinCtrl *widthedit, *heightedit;
		wxRadioBox *algoselect;

};


PicProcessorResize::PicProcessorResize(wxString name, wxString command, wxTreeCtrl *tree, PicPanel *display): PicProcessor(name, command,  tree, display) 
{
	//showParams();
}

void PicProcessorResize::createPanel(wxSimplebook* parent)
{
	toolpanel = new ResizePanel(parent, this, c);
	parent->ShowNewPage(toolpanel);
	toolpanel->Refresh();
	toolpanel->Update();
}

bool PicProcessorResize::processPic(bool processnext) {
	wxString algo = "";
	((wxFrame*) m_display->GetParent())->SetStatusText("resize...");
	wxArrayString cp = split(getParams(),",");
	int width =  atoi(cp[0]);
	int height =  atoi(cp[1]);
	if (cp.size() >2) algo  = cp[2];

	if (dib) delete dib;
	dib = new gImage(getPreviousPicProcessor()->getProcessedPic());
	bool result = true;
	unsigned dw = dib->getWidth();
	unsigned dh = dib->getHeight();
	if (height ==  0) height = dh * ((float)width/(float)dw);
	if (width == 0)  width = dw * ((float)height/(float)dh); 
	RESIZE_FILTER filter = FILTER_LANCZOS3;
	if (algo == "box") filter = FILTER_BOX;
	if (algo == "bilinear") filter = FILTER_BILINEAR;
	if (algo == "bspline") filter = FILTER_BSPLINE;
	if (algo == "bicubic") filter = FILTER_BICUBIC;
	if (algo == "catmullrom") filter = FILTER_CATMULLROM;
	if (algo == "lanczos3") filter = FILTER_LANCZOS3;

	int threadcount =  atoi(myConfig::getConfig().getValue("tool.resize.cores","0").c_str());
	if (threadcount == 0) 
		threadcount = gImage::ThreadCount();
	else if (threadcount < 0) 
		threadcount = std::max(gImage::ThreadCount() + threadcount,0);

	mark();

	dib->ApplyResize(width, height, filter, threadcount);
	wxString d = duration();

	if ((myConfig::getConfig().getValue("tool.all.log","0") == "1") || (myConfig::getConfig().getValue("tool.resize.log","0") == "1"))
		log(wxString::Format("tool=resize,imagesize=%dx%d,threads=%d,time=%s",dib->getWidth(), dib->getHeight(),threadcount,d));

	dirty = false;

	((wxFrame*) m_display->GetParent())->SetStatusText("");
	if (processnext) processNext();
	
	return result;
}



