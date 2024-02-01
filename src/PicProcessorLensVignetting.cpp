#include "PicProcessorLensVignetting.h"
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

#define LVENABLE 9000 

#define LVPA 9001
#define LVADOBE 9002

#define LVA 9003
#define LVB 9004
#define LVC 9005
#define LVD 9006
#define LVAUTOD 9007

#define LVK0 9008
#define LVK1 9009
#define LVK2 9010
#define LVK3 9011
#define LVAUTOK0 9012

#define LVPARESET 9015
#define LVADOBERESET 9016
#define LVCOPY 9017
#define LVPASTE 9018


class LensVignettingPanel: public PicProcPanel
{
	public:
		LensVignettingPanel(wxWindow *parent, PicProcessor *proc, wxString params): PicProcPanel(parent, proc, params)
		{
			Freeze();
			//double rm, gm, bm;
			wxSize spinsize(60, TEXTCTRLHEIGHT);

			enablebox = new wxCheckBox(this, LVENABLE, _("lens vignetting:"));
			enablebox->SetValue(true);

			pab =  new wxRadioButton(this, LVPA, _("pa"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
			adobeb =   new wxRadioButton(this, LVADOBE, _("adobe"));

			pk1 = new myFloatCtrl(this, wxID_ANY, 1.0, 5, wxDefaultPosition, spinsize); pk1->SetFloatValue(0.0);
			pk2 = new myFloatCtrl(this, wxID_ANY, 1.0, 5, wxDefaultPosition, spinsize); pk2->SetFloatValue(0.0);
			pk3 = new myFloatCtrl(this, wxID_ANY, 1.0, 5, wxDefaultPosition, spinsize); pk3->SetFloatValue(0.0);

			ak0 = new myFloatCtrl(this, wxID_ANY, 1.0, 5, wxDefaultPosition, spinsize); ak0->SetFloatValue(1.0);
			ak1 = new myFloatCtrl(this, wxID_ANY, 1.0, 5, wxDefaultPosition, spinsize); ak1->SetFloatValue(0.0);
			ak2 = new myFloatCtrl(this, wxID_ANY, 1.0, 5, wxDefaultPosition, spinsize); ak2->SetFloatValue(0.0);
			ak3 = new myFloatCtrl(this, wxID_ANY, 1.0, 5, wxDefaultPosition, spinsize); ak3->SetFloatValue(0.0);
			ak4 = new myFloatCtrl(this, wxID_ANY, 1.0, 5, wxDefaultPosition, spinsize); ak4->SetFloatValue(0.0);

			pareset = new wxBitmapButton(this, LVPARESET, wxBitmap(undo_xpm), wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT);
			pareset->SetToolTip(_("Reset pa coefficients to original values"));
			
			adobereset = new wxBitmapButton(this, LVADOBERESET, wxBitmap(undo_xpm), wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT);
			adobereset->SetToolTip(_("Reset adobe coefficients to original values"));

			//Lay out with RowSizer:
			//wxSizerFlags flags = wxSizerFlags().Left().CenterVertical().Border(wxLEFT|wxRIGHT|wxTOP); wx3.1
			wxSizerFlags flags = wxSizerFlags().Left().Center().Border(wxLEFT|wxRIGHT|wxTOP);
			myRowSizer *m = new myRowSizer(wxSizerFlags().Expand());
			m->AddRowItem(enablebox, wxSizerFlags(1).Left().Border(wxLEFT|wxTOP));
			m->AddRowItem(new wxBitmapButton(this, LVCOPY, wxBitmap(copy_xpm), wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT), flags);
			m->AddRowItem(new wxBitmapButton(this, LVPASTE, wxBitmap(paste_xpm), wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT), flags);
			m->NextRow(wxSizerFlags().Expand());
			m->AddRowItem(new wxStaticLine(this, wxID_ANY), wxSizerFlags(1).Left().Border(wxLEFT|wxRIGHT|wxTOP|wxBOTTOM));

			unsigned labelwidth = 20;

			//pa:
			m->NextRow();
			m->AddRowItem(pab, flags);

			m->NextRow();
			m->AddRowItem(new wxStaticText(this,wxID_ANY, _("a:"), wxDefaultPosition, wxSize(labelwidth,TEXTHEIGHT)), flags);
			m->AddRowItem(pk1, flags);

			m->NextRow();
			m->AddRowItem(new wxStaticText(this,wxID_ANY, _("b:"), wxDefaultPosition, wxSize(labelwidth,TEXTHEIGHT)), flags);
			m->AddRowItem(pk2, flags);

			m->NextRow();
			m->AddRowItem(new wxStaticText(this,wxID_ANY, _("c:"), wxDefaultPosition, wxSize(labelwidth,TEXTHEIGHT)), flags);
			m->AddRowItem(pk3, flags);

			m->NextRow();
			m->AddRowItem(pareset, flags);

			//adobe:
			m->NextRow(wxSizerFlags().Expand());
			m->AddRowItem(new wxStaticLine(this, wxID_ANY), wxSizerFlags(1).Left().Border(wxLEFT|wxRIGHT|wxTOP|wxBOTTOM));
			m->NextRow();
			m->AddRowItem(adobeb, flags);

			m->NextRow();
			m->AddRowItem(new wxStaticText(this,wxID_ANY, _("k0:"), wxDefaultPosition, wxSize(labelwidth,TEXTHEIGHT)), flags);
			m->AddRowItem(ak0, flags);

			m->NextRow();
			m->AddRowItem(new wxStaticText(this,wxID_ANY, _("k1:"), wxDefaultPosition, wxSize(labelwidth,TEXTHEIGHT)), flags);
			m->AddRowItem(ak1, flags);

			m->NextRow();
			m->AddRowItem(new wxStaticText(this,wxID_ANY, _("k2:"), wxDefaultPosition, wxSize(labelwidth,TEXTHEIGHT)), flags);
			m->AddRowItem(ak2, flags);
			
			m->NextRow();
			m->AddRowItem(new wxStaticText(this,wxID_ANY, _("k3:"), wxDefaultPosition, wxSize(labelwidth,TEXTHEIGHT)), flags);
			m->AddRowItem(ak3, flags);

			m->NextRow();
			m->AddRowItem(new wxStaticText(this,wxID_ANY, _("k4:"), wxDefaultPosition, wxSize(labelwidth,TEXTHEIGHT)), flags);
			m->AddRowItem(ak4, flags);

			m->NextRow();
			m->AddRowItem(adobereset, flags);

			m->End();
			SetSizerAndFit(m);

			//parse parameters:
			params.Trim();
			wxArrayString p = split(params,",");

			if (p[0] == "pa") {
				if (p.size() >= 4) {
					pk1->SetFloatValue(atof(p[1].c_str()));
					pk2->SetFloatValue(atof(p[2].c_str()));
					pk3->SetFloatValue(atof(p[3].c_str()));
				}

				capturePAParams();

				lvmode = LVPA;
				processLV();
			}
			else if (p[0] == "adobe") {
				if (p.size() >= 2) ak0->SetFloatValue(atof(p[1].c_str()));
				if (p.size() >= 3) ak1->SetFloatValue(atof(p[2].c_str()));
				if (p.size() >= 4) ak2->SetFloatValue(atof(p[3].c_str()));
				if (p.size() >= 5) ak3->SetFloatValue(atof(p[4].c_str()));
				if (p.size() >= 6) ak3->SetFloatValue(atof(p[5].c_str()));

				captureAdobeParams();

				lvmode = LVADOBE;
				processLV();
			}
			else 
				wxMessageBox(_("Error: ill-formed param string"));

			SetFocus();
			t.SetOwner(this);

			Bind(wxEVT_TIMER, &LensVignettingPanel::OnTimer, this);
			Bind(wxEVT_RADIOBUTTON, &LensVignettingPanel::OnButton, this);
			Bind(myFLOATCTRL_CHANGE,&LensVignettingPanel::onWheel, this);
			Bind(myFLOATCTRL_UPDATE,&LensVignettingPanel::paramChanged, this);
			Bind(wxEVT_CHECKBOX, &LensVignettingPanel::OnEnable, this, LVENABLE);
			Bind(wxEVT_BUTTON, &LensVignettingPanel::OnPAReset, this, LVPARESET);
			Bind(wxEVT_BUTTON, &LensVignettingPanel::OnAdobeReset, this, LVADOBERESET);
			Bind(wxEVT_BUTTON, &LensVignettingPanel::OnCopy, this, LVCOPY);
			Bind(wxEVT_BUTTON, &LensVignettingPanel::OnPaste, this, LVPASTE);
			Bind(wxEVT_CHAR_HOOK, &LensVignettingPanel::OnKey,  this);
			Thaw();
		}

		void OnPAReset(wxCommandEvent&)
		{
			restorePAParams();
			processLV();
		}

		void OnAdobeReset(wxCommandEvent& event)
		{
			restoreAdobeParams();
			processLV();
		}

		void OnEnable(wxCommandEvent& event)
		{
			if (enablebox->GetValue()) {
				q->enableProcessing(true);
				processLV();
			}
			else {
				q->enableProcessing(false);
				processLV();
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
				if (p[0] == "pa") {
					if (p.size() >= 4) {
						pk1->SetFloatValue(atof(p[1].c_str()));
						pk2->SetFloatValue(atof(p[2].c_str()));
						pk3->SetFloatValue(atof(p[3].c_str()));
					}

					capturePAParams();

					lvmode = LVPA;
					processLV();
				}
				else if (p[0] == "adobe") {
					if (p.size() >= 2) ak0->SetFloatValue(atof(p[1].c_str()));
					if (p.size() >= 3) ak1->SetFloatValue(atof(p[2].c_str()));
					if (p.size() >= 4) ak2->SetFloatValue(atof(p[3].c_str()));
					if (p.size() >= 5) ak3->SetFloatValue(atof(p[4].c_str()));
					if (p.size() >= 6) ak4->SetFloatValue(atof(p[5].c_str()));
					
					captureAdobeParams();
					
					lvmode = LVADOBE;
					processLV();
				}

				else 
					wxMessageBox(wxString::Format(_("Error: ill-formed param string: %s"),q->getParams()));
				((wxFrame *) GetGrandParent())->SetStatusText(wxString::Format(_("Pasted command from clipboard: %s"),q->getCommand()));
			}
			else wxMessageBox(wxString::Format(_("Invalid Paste")));
		}

		void processLV()
		{
			if (lvmode == LVPA) {
				pab->SetValue(true);
				q->setParams(wxString::Format("pa,%0.3f,%0.3f,%0.3f,%0.3f",pk1->GetFloatValue(), pk2->GetFloatValue(), pk3->GetFloatValue()));
				q->processPic();
				Refresh();
			}
			else if (lvmode == LVADOBE) {
				adobeb->SetValue(true);
				wxString cmd = "adobe";
				float k = ak0->GetFloatValue();
				if (k != 0.0) cmd.Append(wxString::Format(",%f", k));
				k = ak1->GetFloatValue();
				if (k != 0.0) cmd.Append(wxString::Format(",%f", k));
				k = ak2->GetFloatValue();
				if (k != 0.0) cmd.Append(wxString::Format(",%f", k));
				k = ak3->GetFloatValue();
				if (k != 0.0) cmd.Append(wxString::Format(",%f", k));
				k = ak4->GetFloatValue();
				if (k != 0.0) cmd.Append(wxString::Format(",%f", k));
				q->setParams(cmd);
				q->processPic();
				Refresh();
			}
		}

		void paramChanged(wxCommandEvent& event)
		{
			processLV();
			Refresh();
		}

		void onWheel(wxCommandEvent& event)
		{
			t.Start(500,wxTIMER_ONE_SHOT);
		}

		void OnTimer(wxTimerEvent& event)
		{
			processLV();
			Refresh();
		}

		void OnButton(wxCommandEvent& event)
		{
			lvmode = event.GetId();
			processLV();
			event.Skip();
		}

		//void OnAutoD(wxCommandEvent& event)
		//{
		//	if (autod->GetValue()) 
		//		ad = true;
		//	else
		//		ad = false;
		//	processLD();
		//}

		//void OnAutoK0(wxCommandEvent& event)
		//{
		//	if (autok0->GetValue()) 
		//		ak0 = true;
		//	else
		//		ak0 = false;
		//	processLD();
		//}
		
		void capturePAParams()
		{
			ppk1 = pk1->GetFloatValue();
			ppk2 = pk2->GetFloatValue();
			ppk3 = pk3->GetFloatValue();
		}
		
		void restorePAParams()
		{
			pk1->SetFloatValue(ppk1);
			pk2->SetFloatValue(ppk2);
			pk3->SetFloatValue(ppk3);
		}
		
		void captureAdobeParams() 
		{
			pak0 = ak0->GetFloatValue();
			pak1 = ak1->GetFloatValue();
			pak2 = ak2->GetFloatValue();
			pak3 = ak3->GetFloatValue();
		}
		
		void restoreAdobeParams()
		{
			ak0->SetFloatValue(pak0);
			ak1->SetFloatValue(pak1);
			ak2->SetFloatValue(pak2);
			ak3->SetFloatValue(pak3);
		}

	private:

		myFloatCtrl *pk1, *pk2 ,*pk3, *ak0, *ak1, *ak2, *ak3, *ak4;
		float ppk1, ppk2, ppk3, pak0, pak1, pak2, pak3, pak4;
		wxRadioButton *pab, *adobeb;
		wxBitmapButton *pareset, *adobereset;
		wxCheckBox *enablebox;
		int lvmode;
		wxTimer t;

};


PicProcessorLensVignetting::PicProcessorLensVignetting(wxString name, wxString command, wxTreeCtrl *tree, PicPanel *display): PicProcessor(name, command, tree, display) 
{

}

void PicProcessorLensVignetting::createPanel(wxSimplebook* parent)
{
	toolpanel = new LensVignettingPanel(parent, this, c);
	parent->ShowNewPage(toolpanel);
	toolpanel->Refresh();
	toolpanel->Update();
}

bool PicProcessorLensVignetting::processPicture(gImage *processdib) 
{
if (!processingenabled) return true;
	
	((wxFrame*) m_display->GetParent())->SetStatusText(_("lensvignetting..."));
	bool ret = true;
	std::map<std::string,std::string> result;

	std::map<std::string,std::string> params;
	std::string pstr = getParams().ToStdString();

	if (!pstr.empty())
		params = parse_lensvignetting(std::string(pstr));

	if (params.find("error") != params.end()) {
		wxMessageBox(params["error"]);
		ret = false; 
	}
	else if (params.find("mode") == params.end()) {  //all variants need a mode, now...
		wxMessageBox("Error - no mode");
		ret = false;
	}
	else { 
		result = process_lensvignetting(*dib, params);
		
		if (result.find("error") != result.end()) {
			wxMessageBox(wxString(result["error"]));
			ret = false;
		}
		else {
			if (paramexists(result,"treelabel")) m_tree->SetItemText(id, wxString(result["treelabel"]));
			m_display->SetModified(true);
			if ((myConfig::getConfig().getValueOrDefault("tool.all.log","0") == "1") || 
				(myConfig::getConfig().getValueOrDefault("tool.lensvignetting.log","0") == "1"))
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





