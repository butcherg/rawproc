#include "PicProcessor.h"
#include "PicProcessorSharpen.h"
#include "PicProcPanel.h"
#include "myConfig.h"
#include "myRowSizer.h"
#include "myFloatCtrl.h"
#include "gimage_parse.h"
#include "gimage_process.h"
#include "undo.xpm"
#include "util.h"

#include <vector>

#define SHARPENENABLE 7800
#define SHARPENCONV  7801
#define SHARPENUSM   7802

class SharpenPanel: public PicProcPanel
{
	public:
		SharpenPanel(wxWindow *parent, PicProcessor *proc, wxString params): PicProcPanel(parent, proc, params)
		{
			Freeze();
			SetSize(parent->GetSize());
			wxSizerFlags flags = wxSizerFlags().Center().Border(wxLEFT|wxRIGHT|wxTOP|wxBOTTOM);

			int initialvalue = atoi(params.c_str());

			enablebox = new wxCheckBox(this, SHARPENENABLE, _("sharpen:"));
			enablebox->SetValue(true);

			convb = new wxRadioButton(this, SHARPENCONV, _("convolution"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
			usmb  = new wxRadioButton(this, SHARPENUSM, _("usm"));

			sharpval = new myFloatCtrl(this, wxID_ANY, 0.0, 1, wxDefaultPosition,wxDefaultSize);
			sigma = new myFloatCtrl(this, wxID_ANY, 0.0, 1, wxDefaultPosition,wxDefaultSize);
			radius = new myFloatCtrl(this, wxID_ANY, 1.5, 1, wxDefaultPosition,wxDefaultSize);
			radius->SetIncrement(1.0);
			btn = new wxBitmapButton(this, wxID_ANY, wxBitmap(undo_xpm), wxPoint(0,0), wxSize(-1,-1), wxBU_EXACTFIT);
			btn->SetToolTip(_("Reset to default"));
			for (int r=0; r<3; r++)
				for (int c=0; c<3; c++)
					kernel[r][c] = new wxStaticText(this, wxID_ANY, "0.00", wxDefaultPosition, wxSize(30,-1),wxALIGN_RIGHT);

			convb->SetValue(true);
			sharpmode = SHARPENCONV;
			sharpval->SetFloatValue(0.0);
			sharpval->Enable(true);
			sigma->SetFloatValue(0.0);
			radius->SetFloatValue(1.5);
			sigma->Enable(false);
			radius->Enable(false);

			wxArrayString p = split(params,",");
			if (p.size() >= 1) {
				if (p[0] == "convolution") {
					convb->SetValue(true);
					sharpmode = SHARPENCONV;
					if (p.size() >= 2) sharpval->SetFloatValue(atof(p[1].c_str()));
				}
				else if (p[0] == "usm") {
					usmb->SetValue(true);
					sharpmode = SHARPENUSM;
					if (p.size() >= 2) sigma->SetFloatValue(atof(p[1].c_str()));
					if (p.size() >= 3) radius->SetFloatValue(atof(p[2].c_str()));
					sigma->Enable(true);
					radius->Enable(true);
				}
				else {
					convb->SetValue(true);
					sharpmode = SHARPENCONV;
					if (p.GetCount() >= 1) sharpval->SetFloatValue(atof(p[0].c_str()));
				}
			}
			
			myRowSizer *m = new myRowSizer(wxSizerFlags().Expand());
			m->AddRowItem(enablebox, wxSizerFlags(1).Left().Border(wxLEFT|wxTOP));
			m->NextRow(wxSizerFlags().Expand());
			m->AddRowItem(new wxStaticLine(this, wxID_ANY), wxSizerFlags(1).Left().Border(wxLEFT|wxRIGHT|wxTOP|wxBOTTOM));

			m->NextRow();
			m->AddRowItem(usmb,flags);
			m->NextRow();
			m->AddRowItem(new wxStaticText(this, wxID_ANY, _("sigma:")), flags);
			m->AddRowItem(sigma, flags);
			m->NextRow();
			m->AddRowItem(new wxStaticText(this, wxID_ANY, _("radius:")), flags);
			m->AddRowItem(radius, flags);
			m->NextRow(wxSizerFlags().Expand());
			m->AddRowItem(new wxStaticLine(this, wxID_ANY), wxSizerFlags(1).Left().Border(wxLEFT|wxRIGHT|wxTOP|wxBOTTOM));

			m->NextRow();
			m->AddRowItem(convb,flags);
			m->NextRow();
			m->AddRowItem(new wxStaticText(this, wxID_ANY, _("strength:")), flags);
			m->AddRowItem(sharpval, flags);
			m->AddRowItem(btn, flags);
			
			m->NextRow();
			m->AddRowItem(new wxStaticText(this, wxID_ANY, _("Kernel:")), flags);
			m->NextRow();
			m->AddRowItem(kernel[0][0], flags);
			m->AddRowItem(kernel[0][1], flags);
			m->AddRowItem(kernel[0][2], flags);
			m->NextRow();
			m->AddRowItem(kernel[1][0], flags);
			m->AddRowItem(kernel[1][1], flags);
			m->AddRowItem(kernel[1][2], flags);
			m->NextRow();
			m->AddRowItem(kernel[2][0], flags);
			m->AddRowItem(kernel[2][1], flags);
			m->AddRowItem(kernel[2][2], flags);
			
			m->End();
			SetSizerAndFit(m);
			SetFocus();
			t.SetOwner(this);
			Bind(wxEVT_BUTTON, &SharpenPanel::OnButton, this);
			Bind(myFLOATCTRL_CHANGE,&SharpenPanel::OnChanged, this);
			Bind(myFLOATCTRL_UPDATE,&SharpenPanel::paramChanged, this);
			Bind(wxEVT_RADIOBUTTON, &SharpenPanel::OnRadioButton, this);
			Bind(wxEVT_CHECKBOX, &SharpenPanel::onEnable, this, SHARPENENABLE);
			Bind(wxEVT_TIMER, &SharpenPanel::OnTimer,  this);
			Bind(wxEVT_CHAR_HOOK, &SharpenPanel::OnKey,  this);
			Thaw();
		}

		void onEnable(wxCommandEvent& event)
		{
			if (enablebox->GetValue()) {
				q->enableProcessing(true);
				q->processPic();
			}
			else {
				q->enableProcessing(false);
				q->processPic();
			}
		}

		void OnChanged(wxCommandEvent& event)
		{
			float val = sharpval->GetFloatValue();
			if (val < 0.0) sharpval->SetFloatValue(0.0);
			if (val > 10.0) sharpval->SetFloatValue(10.0);
			float sval = sigma->GetFloatValue();
			if (sval < 0.0) sigma->SetFloatValue(0.0);
			if (sval > 100.0) sharpval->SetFloatValue(100.0);
			float rval = radius->GetFloatValue();
			if (rval < 1.5) radius->SetFloatValue(1.5);
			t.Start(500,wxTIMER_ONE_SHOT);
		}

		void OnThumbTrack(wxCommandEvent& event)
		{

		}
		
		void paramChanged(wxCommandEvent& event)
		{
			processSH();
			event.Skip();
		}

		void OnTimer(wxTimerEvent& event)
		{
			processSH();
		}

		void OnButton(wxCommandEvent& event)
		{
			int resetval = atoi(myConfig::getConfig().getValueOrDefault("tool.sharpen.initialvalue","0").c_str());
			sharpval->SetFloatValue((float) resetval);
			processSH();
			event.Skip();
		}

		void processSH()
		{
			switch (sharpmode) {
				case SHARPENCONV:
					q->setParams(wxString::Format("convolution,%2.2f",sharpval->GetFloatValue()));
					q->processPic();
					break;
				case SHARPENUSM:
					q->setParams(wxString::Format("usm,%2.2f,%2.2f",sigma->GetFloatValue(),radius->GetFloatValue()));
					q->processPic();
					break;
			}
		}

		void OnRadioButton(wxCommandEvent& event)
		{
			sharpmode = event.GetId();
			if (sharpmode == SHARPENCONV) {
				sigma->Enable(false);
				radius->Enable(false);
				sharpval->Enable(true);
			}
			else {
				sigma->Enable(true);
				radius->Enable(true);
				sharpval->Enable(false);
			}
			processSH();
		}
		
		void setKernel(double strength)
		{
			double k[3][3] =
			{
				0.0, 0.0, 0.0,
				0.0, 0.0, 0.0,
				0.0, 0.0, 0.0
			};
			double x = -((strength)/4.0);
			k[0][1] = x;
			k[1][0] = x;
			k[1][2] = x;
			k[2][1] = x;
			k[1][1] = strength+1.0;
			
			for (int r=0; r<3; r++)
				for (int c=0; c<3; c++)
					kernel[r][c]->SetLabel(wxString::Format("%0.2f",k[r][c]+0.0));
			Refresh();
		}


	private:
		myFloatCtrl *sharpval, *sigma, *radius;
		wxBitmapButton *btn;
		wxCheckBox *enablebox;
		wxRadioButton *convb, *usmb;
		int sharpmode;
		wxTimer t;
		wxStaticText * kernel[3][3];


};


PicProcessorSharpen::PicProcessorSharpen(wxString name, wxString command, wxTreeCtrl *tree, PicPanel *display): PicProcessor(name, command,  tree, display) 
{
	//showParams();
}

void PicProcessorSharpen::createPanel(wxSimplebook* parent)
{
	toolpanel = new SharpenPanel(parent, this, c);
	parent->ShowNewPage(toolpanel);
	toolpanel->Refresh();
	toolpanel->Update();
}

bool PicProcessorSharpen::processPicture(gImage *processdib) 
{
	if (!processingenabled) return true;
	
	((wxFrame*) m_display->GetParent())->SetStatusText(_("sharpen..."));
	bool ret = true;
	std::map<std::string,std::string> result;

	std::map<std::string,std::string> params;
	std::string pstr = getParams().ToStdString();

	if (!pstr.empty())
		params = parse_sharpen(std::string(pstr));
	
	if (params.find("error") != params.end()) {
		wxMessageBox(params["error"]);
		ret = false; 
	}
	else if (params.find("mode") == params.end()) {  //all variants need a mode, now...
		wxMessageBox("Error - no mode");
		ret = false;
	}
	else { 
		result = process_sharpen(*dib, params);
		
		if (result.find("error") != result.end()) {
			wxMessageBox(wxString(result["error"]));
			ret = false;
		}
		else {
			if (paramexists(result,"treelabel")) m_tree->SetItemText(id, wxString(result["treelabel"]));
			if (params["mode"] == "convolution") 
				((SharpenPanel *) toolpanel)->setKernel(atof(params["strength"].c_str()));
			m_display->SetModified(true);
			if ((myConfig::getConfig().getValueOrDefault("tool.all.log","0") == "1") || 
				(myConfig::getConfig().getValueOrDefault("tool.sharpen.log","0") == "1"))
					log(wxString::Format(_("tool=sharpen,%s,imagesize=%dx%d,threads=%s,time=%s"),
						params["mode"].c_str(),
						dib->getWidth(), 
						dib->getHeight(),
						result["threadcount"].c_str(),
						result["duration"].c_str())
					);
		}
	}

	dirty=false;
	((wxFrame*) m_display->GetParent())->SetStatusText("");
	return ret;
}


