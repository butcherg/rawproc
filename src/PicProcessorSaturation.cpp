#include "PicProcessor.h"
#include "PicProcessorSaturation.h"
#include "PicProcPanel.h"
#include "myFloatCtrl.h"
#include "myRowSizer.h"
#include "myConfig.h"
#include "util.h"
#include "undo.xpm"

#define SATURATIONENABLE 7600
#define SATURATIONRESET 7601
#define SATURATIONPATCHRESET 7602

class SaturationPanel: public PicProcPanel
{
	public:
		SaturationPanel(wxWindow *parent, PicProcessor *proc, wxString params): PicProcPanel(parent, proc, params)
		{
			Freeze();
			SetSize(parent->GetSize());

			rmin=FLT_MAX; rmax=0.0; gmin=FLT_MAX; gmax=0.0; bmin=FLT_MAX; bmax=0.0;
			ispatch = false;

			double initialvalue = atof(params.c_str());

			enablebox = new wxCheckBox(this, SATURATIONENABLE, _("saturation:"));
			enablebox->SetValue(true);

			saturate = new myFloatCtrl(this, wxID_ANY, 1.0, 2, wxDefaultPosition,wxDefaultSize);
			btn = new wxBitmapButton(this, SATURATIONRESET, wxBitmap(undo_xpm), wxPoint(0,0), wxSize(-1,-1), wxBU_EXACTFIT);
			btn->SetToolTip(_("Reset to default"));
			
			for (int r=0; r<2; r++)
				for (int c=0; c<3; c++)
					colorpatch[r][c] = new wxStaticText(this, wxID_ANY, "--", wxDefaultPosition, wxSize(30,-1),wxALIGN_RIGHT);
			
			wxSizerFlags flags = wxSizerFlags().Center().Border(wxLEFT|wxRIGHT|wxTOP|wxBOTTOM);
			wxSizerFlags patchflags = wxSizerFlags().Center().Border(wxLEFT|wxRIGHT);
			myRowSizer *m = new myRowSizer(wxSizerFlags().Expand());
			m->AddRowItem(enablebox, wxSizerFlags(1).Left().Border(wxLEFT|wxTOP));
			m->NextRow(wxSizerFlags().Expand());
			m->AddRowItem(new wxStaticLine(this, wxID_ANY), wxSizerFlags(1).Left().Border(wxLEFT|wxRIGHT|wxTOP|wxBOTTOM));

			m->NextRow();
			m->AddRowItem(saturate,flags);
			m->AddRowItem(btn,flags);
			
			m->NextRow();

			m->AddRowItem(new wxStaticText(this, wxID_ANY, _("colorpatch:")), flags);
			m->NextRow();
			//m->AddRowItem(new wxStaticText(this, wxID_ANY, "     r     g     b"), flags);
			m->NextRow();
			m->AddRowItem(new wxStaticText(this, wxID_ANY, _("max:")), patchflags);
			m->AddRowItem(colorpatch[0][0], patchflags);
			m->AddRowItem(colorpatch[0][1], patchflags);
			m->AddRowItem(colorpatch[0][2], patchflags);
	
			m->NextRow();
			m->AddRowItem(new wxStaticText(this, wxID_ANY, _("min: ")), patchflags);
			m->AddRowItem(colorpatch[1][0], patchflags);
			m->AddRowItem(colorpatch[1][1], patchflags);
			m->AddRowItem(colorpatch[1][2], patchflags);
			
			m->NextRow();
			m->AddRowItem(new wxButton(this, SATURATIONPATCHRESET, _("Reset Patch")) ,flags);

			m->End();

			SetSizerAndFit(m);
			SetFocus();
			t.SetOwner(this);
			Bind(wxEVT_BUTTON, &SaturationPanel::OnButton, this, SATURATIONRESET);
			Bind(wxEVT_BUTTON, &SaturationPanel::clearPatch, this, SATURATIONPATCHRESET);
			Bind(myFLOATCTRL_CHANGE,&SaturationPanel::OnChanged, this);
			Bind(myFLOATCTRL_UPDATE,&SaturationPanel::OnEnter, this);
			Bind(wxEVT_CHECKBOX, &SaturationPanel::onEnable, this, SATURATIONENABLE);
			Bind(wxEVT_TIMER, &SaturationPanel::OnTimer,  this);
			Bind(wxEVT_CHAR_HOOK, &SaturationPanel::OnKey,  this);
			Thaw();
		}

		void onEnable(wxCommandEvent& event)
		{
			if (enablebox->GetValue()) {
				//q->enableProcessing(true);
				//q->processPic();
				processSA();
			}
			else {
				//q->enableProcessing(false);
				//q->processPic();
				processSA();
			}
		}

		void OnEnter(wxCommandEvent& event)
		{
			//q->setParams(wxString::Format("%2.2f",saturate->GetFloatValue()));
			//q->processPic();
			processSA();
			event.Skip();
		}

		void OnChanged(wxCommandEvent& event)
		{
			t.Start(500,wxTIMER_ONE_SHOT);
		}

		void OnTimer(wxTimerEvent& event)
		{
			//q->setParams(wxString::Format("%2.2f",saturate->GetFloatValue()));
			//q->processPic();
			processSA();
			event.Skip();
		}

		void OnButton(wxCommandEvent& event)
		{
			double resetval = atof(myConfig::getConfig().getValueOrDefault("tool.saturate.initialvalue","1.0").c_str());
			saturate->SetFloatValue(resetval);
			//q->setParams(wxString::Format("%2.2f",resetval));
			//q->processPic();
			processSA();
			event.Skip();
		}

		void processSA()
		{
			if (ispatch) 
				q->setParams(wxString::Format("%2.2f,%2.2f,%2.2f,%2.2f,%2.2f,%2.2f,%2.2f",rmin, rmax, gmin, gmax, bmin, bmax, saturate->GetFloatValue()));
			else
				q->setParams(wxString::Format("%2.2f",saturate->GetFloatValue()));
			q->processPic();
		}

		void addToPatch(pix p)
		{
			ispatch = true;
			if (p.r < rmin) rmin = p.r;
			if (p.r > rmax) rmax = p.r;
			if (p.g < gmin) gmin = p.g;
			if (p.g > gmax) gmax = p.g;
			if (p.b < bmin) bmin = p.b;
			if (p.b > bmax) bmax = p.b;
			colorpatch[0][0]->SetLabel(wxString::Format("%0.2f",rmax));
			colorpatch[0][1]->SetLabel(wxString::Format("%0.2f",gmax));
			colorpatch[0][2]->SetLabel(wxString::Format("%0.2f",bmax));
			colorpatch[1][0]->SetLabel(wxString::Format("%0.2f",rmin));
			colorpatch[1][1]->SetLabel(wxString::Format("%0.2f",gmin));
			colorpatch[1][2]->SetLabel(wxString::Format("%0.2f",bmin));
			Refresh();
		}

		void clearPatch(wxCommandEvent& event)
		{
			ispatch = false;
			rmin=FLT_MAX; rmax=0.0; gmin=FLT_MAX; gmax=0.0; bmin=FLT_MAX; bmax=0.0;
			colorpatch[0][0]->SetLabel("--");
			colorpatch[0][1]->SetLabel("--");
			colorpatch[0][2]->SetLabel("--");
			colorpatch[1][0]->SetLabel("--");
			colorpatch[1][1]->SetLabel("--");
			colorpatch[1][2]->SetLabel("--");
			Refresh();
			processSA();
		}


	private:
		myFloatCtrl *saturate;
		float rmin, rmax, gmin, gmax, bmin, bmax;
		bool ispatch;
		wxBitmapButton *btn;
		wxCheckBox *enablebox;
		wxStaticText * colorpatch[2][3];
		wxTimer t;

};


PicProcessorSaturation::PicProcessorSaturation(wxString name, wxString command, wxTreeCtrl *tree, PicPanel *display): PicProcessor(name, command,  tree, display) 
{
	m_display->Bind(wxEVT_LEFT_DOWN, &PicProcessorSaturation::OnLeftDown, this);
	m_display->Bind(wxEVT_MOTION, &PicProcessorSaturation::OnMouseMove, this);
	m_display->Bind(wxEVT_LEFT_UP, &PicProcessorSaturation::OnLeftUp, this);
}


PicProcessorSaturation::~PicProcessorSaturation()
{
	m_display->Unbind(wxEVT_LEFT_DOWN, &PicProcessorSaturation::OnLeftDown, this);
	m_display->Unbind(wxEVT_MOTION, &PicProcessorSaturation::OnMouseMove, this);
	m_display->Unbind(wxEVT_LEFT_UP, &PicProcessorSaturation::OnLeftUp, this);
}

void PicProcessorSaturation::createPanel(wxSimplebook* parent)
{
	toolpanel = new SaturationPanel(parent, this, c);
	parent->ShowNewPage(toolpanel);
	toolpanel->Refresh();
	toolpanel->Update();
}

void PicProcessorSaturation::OnLeftDown(wxMouseEvent& event)
{
	struct coord c;
	struct pix p;
	if (m_tree->GetSelection() == GetId()) {
		if (event.ShiftDown()) {
			c = m_display->GetImgCoords();
			p = getPreviousPicProcessor()->getProcessedPic().getPixel(c.x, c.y);
			((SaturationPanel *) toolpanel)->addToPatch(p);
			//printf("rgb: %f,%f,%f\n",p.r, p.g, p.b); fflush(stdout);
		}
		else event.Skip();
	}
	else event.Skip();
}
			
void PicProcessorSaturation::OnMouseMove(wxMouseEvent& event)
{
	struct coord c;
	struct pix p;
	if (m_tree->GetSelection() == GetId()) {
		if (event.ShiftDown()) {
			c = m_display->GetImgCoords();
			p = getPreviousPicProcessor()->getProcessedPic().getPixel(c.x, c.y);
			((SaturationPanel *) toolpanel)->addToPatch(p);
			//printf("rgb: %f,%f,%f\n",p.r, p.g, p.b); fflush(stdout);
		}
	}
	event.Skip();
}

void PicProcessorSaturation::OnLeftUp(wxMouseEvent& event)
{
	struct coord c;
	struct pix p;
	if (m_tree->GetSelection() == GetId()) {
		if (event.ShiftDown()) {
			((SaturationPanel *) toolpanel)->processSA();
		}
	}
	event.Skip();
}

bool PicProcessorSaturation::processPicture(gImage *processdib) 
{
	((wxFrame*) m_display->GetParent())->SetStatusText(_("saturation..."));

	float rmin=0.0, rmax=FLT_MAX, gmin=0.0, gmax=FLT_MAX, bmin=0.0, bmax=FLT_MAX;

	wxArrayString p = split(wxString(c),",");
	double saturation = atof(p[0].c_str());
	if (p.size() >= 2) rmin = atof(p[1].c_str());
	if (p.size() >= 3) rmin = atof(p[2].c_str());
	if (p.size() >= 4) rmin = atof(p[3].c_str());
	if (p.size() >= 5) rmin = atof(p[4].c_str());
	if (p.size() >= 6) rmin = atof(p[5].c_str());
	if (p.size() >= 7) rmin = atof(p[6].c_str());
	bool result = true;
	
	int threadcount =  atoi(myConfig::getConfig().getValueOrDefault("tool.saturate.cores","0").c_str());
	if (threadcount == 0) 
		threadcount = gImage::ThreadCount();
	else if (threadcount < 0) 
		threadcount = std::max(gImage::ThreadCount() + threadcount,0);

	dib = processdib;
	if (!global_processing_enabled) return true;

	if (processingenabled & saturation != 1.0) {
		mark();
		dib->ApplySaturate(saturation, rmin, rmax, gmin, gmax, bmin, bmax, 0.0, 0.0, 1.0, 1.0, threadcount);
		m_display->SetModified(true);
		wxString d = duration();

		if ((myConfig::getConfig().getValueOrDefault("tool.all.log","0") == "1") || (myConfig::getConfig().getValueOrDefault("tool.saturate.log","0") == "1"))
			log(wxString::Format(_("tool=saturate,imagesize=%dx%d,threads=%d,time=%s"),dib->getWidth(), dib->getHeight(),threadcount,d));

	}
	
	dirty = false;

	((wxFrame*) m_display->GetParent())->SetStatusText("");
	
	return result;
}



