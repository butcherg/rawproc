#include "PicProcessorLensDistortion.h"
#include "PicProcPanel.h"
#include "myConfig.h"
#include "util.h"
#include "myRowSizer.h"
#include "myFloatCtrl.h"
#include <wx/spinctrl.h>
#include <wx/clipbrd.h>
#include "gimage_parse.h"
#include "gimage_process.h"
#include "unchecked.xpm"
#include "checked.xpm"
#include "undo.xpm"
#include "copy.xpm"
#include "paste.xpm"

#define LDENABLE 8900

#define LDPTLENS 8901
#define LDADOBE 8902

#define LDA 8903
#define LDB 8904
#define LDC 8905
#define LDD 8906
#define LDAUTOD 8907

#define LDK0 8908
#define LDK1 8909
#define LDK2 8910
#define LDK3 8911

#define LDRESET 8915
#define LDCOPY 8916
#define LDPASTE 8917


class LensDistortionPanel: public PicProcPanel
{

	public:
		LensDistortionPanel(wxWindow *parent, PicProcessor *proc, wxString params): PicProcPanel(parent, proc, params)
		{
			Freeze();
			//double rm, gm, bm;
			wxSize spinsize(60, TEXTCTRLHEIGHT);
			
			ad = false;

			enablebox = new wxCheckBox(this, LDENABLE, _("lens distortion:"));
			enablebox->SetValue(true);
			
			ptlensb =  new wxRadioButton(this, LDPTLENS, _("ptlens"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
			adobeb =   new wxRadioButton(this, LDADOBE, _("adobe"));

			a = new myFloatCtrl(this, wxID_ANY, 1.0, 5, wxDefaultPosition, spinsize); a->SetFloatValue(0.0);
			b = new myFloatCtrl(this, wxID_ANY, 1.0, 5, wxDefaultPosition, spinsize); b->SetFloatValue(0.0);
			c = new myFloatCtrl(this, wxID_ANY, 1.0, 5, wxDefaultPosition, spinsize); c->SetFloatValue(0.0);
			d = new myFloatCtrl(this, wxID_ANY, 1.0, 5, wxDefaultPosition, spinsize); d->SetFloatValue(1.0);
			autod = new wxCheckBox(this, LDAUTOD, _("d=1-(a+b+c):"));
			
			k0 = new myFloatCtrl(this, wxID_ANY, 1.0, 5, wxDefaultPosition, spinsize); k0->SetFloatValue(1.0);
			k1 = new myFloatCtrl(this, wxID_ANY, 1.0, 5, wxDefaultPosition, spinsize); k1->SetFloatValue(0.0);
			k2 = new myFloatCtrl(this, wxID_ANY, 1.0, 5, wxDefaultPosition, spinsize); k2->SetFloatValue(0.0);
			k3 = new myFloatCtrl(this, wxID_ANY, 1.0, 5, wxDefaultPosition, spinsize); k3->SetFloatValue(0.0);
			
			
			
			btn = new wxBitmapButton(this, LDRESET, wxBitmap(undo_xpm), wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT);
			btn->SetToolTip(_("Reset coefficients to original values"));


			//Lay out with RowSizer:
			//wxSizerFlags flags = wxSizerFlags().Left().CenterVertical().Border(wxLEFT|wxRIGHT|wxTOP); wx3.1
			wxSizerFlags flags = wxSizerFlags().Left().Center().Border(wxLEFT|wxRIGHT|wxTOP);
			myRowSizer *m = new myRowSizer(wxSizerFlags().Expand());
			m->AddRowItem(enablebox, wxSizerFlags(1).Left().Border(wxLEFT|wxTOP));
			m->AddRowItem(new wxBitmapButton(this, LDCOPY, wxBitmap(copy_xpm), wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT), flags);
			m->AddRowItem(new wxBitmapButton(this, LDPASTE, wxBitmap(paste_xpm), wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT), flags);
			m->NextRow(wxSizerFlags().Expand());
			m->AddRowItem(new wxStaticLine(this, wxID_ANY), wxSizerFlags(1).Left().Border(wxLEFT|wxRIGHT|wxTOP|wxBOTTOM));

			unsigned labelwidth = 20;

			//ptlens:
			m->NextRow();
			m->AddRowItem(ptlensb, flags);
			
			m->NextRow();
			m->AddRowItem(new wxStaticText(this,wxID_ANY, _("a:"), wxDefaultPosition, wxSize(labelwidth,TEXTHEIGHT)), flags);
			m->AddRowItem(a, flags);

			m->NextRow();
			m->AddRowItem(new wxStaticText(this,wxID_ANY, _("b:"), wxDefaultPosition, wxSize(labelwidth,TEXTHEIGHT)), flags);
			m->AddRowItem(b, flags);

			m->NextRow();
			m->AddRowItem(new wxStaticText(this,wxID_ANY, _("c:"), wxDefaultPosition, wxSize(labelwidth,TEXTHEIGHT)), flags);
			m->AddRowItem(c, flags);
			
			m->NextRow();
			m->AddRowItem(new wxStaticText(this,wxID_ANY, _("d:"), wxDefaultPosition, wxSize(labelwidth,TEXTHEIGHT)), flags);
			m->AddRowItem(d, flags);
			m->AddRowItem(autod, flags);

			m->NextRow();
			m->AddRowItem(btn, flags);
			
			//adobe:
			m->NextRow(wxSizerFlags().Expand());
			m->AddRowItem(new wxStaticLine(this, wxID_ANY), wxSizerFlags(1).Left().Border(wxLEFT|wxRIGHT|wxTOP|wxBOTTOM));
			m->NextRow();
			m->AddRowItem(adobeb, flags);
			
			m->NextRow();
			m->AddRowItem(new wxStaticText(this,wxID_ANY, _("k0:"), wxDefaultPosition, wxSize(labelwidth,TEXTHEIGHT)), flags);
			m->AddRowItem(k0, flags);

			m->NextRow();
			m->AddRowItem(new wxStaticText(this,wxID_ANY, _("k1:"), wxDefaultPosition, wxSize(labelwidth,TEXTHEIGHT)), flags);
			m->AddRowItem(k1, flags);

			m->NextRow();
			m->AddRowItem(new wxStaticText(this,wxID_ANY, _("k2:"), wxDefaultPosition, wxSize(labelwidth,TEXTHEIGHT)), flags);
			m->AddRowItem(k2, flags);
			
			m->NextRow();
			m->AddRowItem(new wxStaticText(this,wxID_ANY, _("k3:"), wxDefaultPosition, wxSize(labelwidth,TEXTHEIGHT)), flags);
			m->AddRowItem(k3, flags);

			//m->NextRow();
			//m->AddRowItem(btn, flags);
			

			m->End();
			SetSizerAndFit(m);

			//parse parameters:
			params.Trim();
			wxArrayString p = split(params,",");
			
			if (p[0] == "ptlens") {
				if (p.size() >= 4) {
					a->SetFloatValue(atof(p[1].c_str()));
					b->SetFloatValue(atof(p[2].c_str()));
					c->SetFloatValue(atof(p[3].c_str()));
				}

				if (p.size() >= 5) {
					d->SetFloatValue(atof(p[4].c_str()));
				}
				else 
					d->SetFloatValue(1.0);
				
				ldmode = LDPTLENS;
				processLD();
			}
			else if (p[0] == "adobe") {
				k0->SetFloatValue(atof(p[1].c_str()));
				k1->SetFloatValue(atof(p[2].c_str()));
				k2->SetFloatValue(atof(p[3].c_str()));
				k3->SetFloatValue(atof(p[4].c_str()));
				ldmode = LDADOBE;
				processLD();
			}

			else 
				wxMessageBox(_("Error: ill-formed param string"));

			SetFocus();
			t.SetOwner(this);

			Bind(wxEVT_TIMER, &LensDistortionPanel::OnTimer, this);
			Bind(wxEVT_RADIOBUTTON, &LensDistortionPanel::OnButton, this);
			Bind(myFLOATCTRL_CHANGE,&LensDistortionPanel::onWheel, this);
			Bind(myFLOATCTRL_UPDATE,&LensDistortionPanel::paramChanged, this);
			Bind(wxEVT_CHECKBOX, &LensDistortionPanel::OnEnable, this, LDENABLE);
			Bind(wxEVT_CHECKBOX, &LensDistortionPanel::OnAutoD, this, LDAUTOD);
			Bind(wxEVT_BUTTON, &LensDistortionPanel::OnReset, this, LDRESET);
			Bind(wxEVT_BUTTON, &LensDistortionPanel::OnCopy, this, LDCOPY);
			Bind(wxEVT_BUTTON, &LensDistortionPanel::OnPaste, this, LDPASTE);
			Bind(wxEVT_CHAR_HOOK, &LensDistortionPanel::OnKey,  this);
			Thaw();
		}
		
		void OnReset(wxCommandEvent& event)
		{
			processLD();
			//q->setParams(wxString::Format("%0.3f,%0.3f,%0.3f",a->GetFloatValue(), a->GetFloatValue(), a->GetFloatValue()));
			//q->processPic();
			
		}

		void OnEnable(wxCommandEvent& event)
		{
			if (enablebox->GetValue()) {
				q->enableProcessing(true);
				processLD();
				//q->processPic();
			}
			else {
				q->enableProcessing(false);
				processLD();
				//q->processPic();
			}
		}

		void OnCopy(wxCommandEvent& event)
		{
			q->copyParamsToClipboard();
			((wxFrame *) GetGrandParent())->SetStatusText(wxString::Format(_("Copied command to clipboard: %s"),q->getCommand()));
			
		}

		void OnPaste(wxCommandEvent& event)
		{
			if (q->pasteParamsFromClipboard()) {
				wxArrayString p = split(q->getParams(),",");
				if (p[0] == "ptlens") {
					if (p.size() >= 4) {
						a->SetFloatValue(atof(p[1].c_str()));
						b->SetFloatValue(atof(p[2].c_str()));
						c->SetFloatValue(atof(p[3].c_str()));
					}

					if (p.size() >= 5) 
						d->SetFloatValue(atof(p[4].c_str()));
					else
						d->SetFloatValue(1.0);
					
					ldmode = LDPTLENS;
					processLD();
				}
				else if (p[0] == "adobe") {
					k0->SetFloatValue(atof(p[1].c_str()));
					k1->SetFloatValue(atof(p[2].c_str()));
					k2->SetFloatValue(atof(p[3].c_str()));
					k3->SetFloatValue(atof(p[4].c_str()));
					ldmode = LDADOBE;
					processLD();
				}

				else 
					wxMessageBox(wxString::Format(_("Error: ill-formed param string: %s"),q->getParams()));
				((wxFrame *) GetGrandParent())->SetStatusText(wxString::Format(_("Pasted command from clipboard: %s"),q->getCommand()));
			}
			else wxMessageBox(wxString::Format(_("Invalid Paste")));
		}



		void processLD()
		{
			if (ldmode == LDPTLENS) {
				ptlensb->SetValue(true);
				if (ad) d->SetFloatValue(1-(a->GetFloatValue() + b->GetFloatValue() + c->GetFloatValue()));
				q->setParams(wxString::Format("ptlens,%0.3f,%0.3f,%0.3f,%0.3f",a->GetFloatValue(), b->GetFloatValue(), c->GetFloatValue(), d->GetFloatValue()));
				q->processPic();
				Refresh();
			}
			else if (ldmode == LDADOBE) {
				adobeb->SetValue(true);
				q->setParams(wxString::Format("adobe,%0.3f,%0.3f,%0.3f,%0.3f",k0->GetFloatValue(), k1->GetFloatValue(), k2->GetFloatValue(), k3->GetFloatValue()));
				q->processPic();
				Refresh();
			}
		}


		void paramChanged(wxCommandEvent& event)
		{
			//if (ad) d->SetFloatValue(1-(a->GetFloatValue() + b->GetFloatValue() + c->GetFloatValue()));
			//q->setParams(wxString::Format("%0.3f,%0.3f,%0.3f,%0.3f",a->GetFloatValue(), b->GetFloatValue(), c->GetFloatValue(), //d->GetFloatValue()));
			//q->processPic();
			processLD();
			Refresh();
		}
		
		void onWheel(wxCommandEvent& event)
		{
			//if (ad) d->SetFloatValue(1-(a->GetFloatValue() + b->GetFloatValue() + c->GetFloatValue()));
			//q->setParams(wxString::Format("%0.3f,%0.3f,%0.3f,%0.3f",a->GetFloatValue(), b->GetFloatValue(), c->GetFloatValue(), d->GetFloatValue()));
			//q->processPic();
			t.Start(500,wxTIMER_ONE_SHOT);
		}

		void OnTimer(wxTimerEvent& event)
		{
			processLD();
			Refresh();
		}

		void OnButton(wxCommandEvent& event)
		{
			ldmode = event.GetId();
			processLD();
			event.Skip();
		}
		
		void OnAutoD(wxCommandEvent& event)
		{
			if (autod->GetValue()) 
				ad = true;
			else
				ad = false;
			processLD();
		}

	private:
		
		myFloatCtrl *a, *b ,*c, *d, *k0, *k1, *k2, *k3;
		bool ad;
		wxRadioButton *ptlensb, *adobeb;
		wxBitmapButton *btn;
		wxCheckBox *enablebox, *autod;
		int ldmode;
		wxTimer t;

};


PicProcessorLensDistortion::PicProcessorLensDistortion(wxString name, wxString command, wxTreeCtrl *tree, PicPanel *display): PicProcessor(name, command, tree, display) 
{

}

void PicProcessorLensDistortion::createPanel(wxSimplebook* parent)
{
	toolpanel = new LensDistortionPanel(parent, this, c);
	parent->ShowNewPage(toolpanel);
	toolpanel->Refresh();
	toolpanel->Update();
}


bool PicProcessorLensDistortion::processPicture(gImage *processdib) 
{
if (!processingenabled) return true;
	
	((wxFrame*) m_display->GetParent())->SetStatusText(_("lensdistortion..."));
	bool ret = true;
	std::map<std::string,std::string> result;

	std::map<std::string,std::string> params;
	std::string pstr = getParams().ToStdString();

	if (!pstr.empty())
		params = parse_lensdistortion(std::string(pstr));
	
	if (params.find("error") != params.end()) {
		wxMessageBox(params["error"]);
		ret = false; 
	}
	else if (params.find("mode") == params.end()) {  //all variants need a mode, now...
		wxMessageBox("Error - no mode");
		ret = false;
	}
	else { 
		result = process_lensdistortion(*dib, params);
		
		if (result.find("error") != result.end()) {
			wxMessageBox(wxString(result["error"]));
			ret = false;
		}
		else {
			if (paramexists(result,"treelabel")) m_tree->SetItemText(id, wxString(result["treelabel"]));
			m_display->SetModified(true);
			if ((myConfig::getConfig().getValueOrDefault("tool.all.log","0") == "1") || 
				(myConfig::getConfig().getValueOrDefault("tool.lensdistortion.log","0") == "1"))
					log(wxString::Format(_("tool=resize,%s,imagesize=%dx%d,threads=%s,time=%s"),
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





