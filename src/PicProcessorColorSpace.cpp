#include "PicProcessorColorSpace.h"
#include "PicProcPanel.h"
#include "myRowSizer.h"
#include "util.h"
#include "gimage/strutil.h"
#include "fileutil.h"
#include "myConfig.h"
#include "myEXIFDialog.h"
#include "CameraData.h"
#include "gimage_parse.h"
#include "gimage_process.h"
#include <wx/stdpaths.h>
#include "listview.xpm"
#include "copy.xpm"
#include "paste.xpm"

#define COLORENABLE 6500
#define COLOROP 6501
#define COLORINTENT 6502
#define COLORBPC 6503
#define COLORFILE 6504
#define COLORPROFILE 6505
#define COLORCAMERA 6506
#define COLORCAMERASTATUS 6507
#define COLORPASTE	 6508
#define COLORCOPY	 6509


class ColorspacePanel: public PicProcPanel
{

	public:
		ColorspacePanel(wxWindow *parent, PicProcessor *proc, wxString params): PicProcPanel(parent, proc, params)
		{
			Freeze();
			camdat_status = "";
			wxSizerFlags flags = wxSizerFlags().Left().Border(wxLEFT|wxRIGHT|wxTOP);
			wxArrayString parms = split(params, ",");

			enablebox = new wxCheckBox(this, COLORENABLE, _("colorspace:"));
			enablebox->SetValue(true);

			profileb = new wxRadioButton(this, COLORPROFILE, _("profile file:"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
			camb = new wxRadioButton(this, COLORCAMERA, _("assign camera profile:"));
			
			edit = new wxTextCtrl(this, wxID_ANY, "", wxDefaultPosition, wxSize(-1,TEXTCTRLHEIGHT),wxTE_PROCESS_ENTER);

			wxArrayString opers;
			opers.Add("convert");
			opers.Add("assign");

			operselect = new wxRadioBox (this, COLOROP, _("operation"), wxDefaultPosition, wxDefaultSize,  opers, 1, wxRA_SPECIFY_COLS);
			if (parms[1] != "-") operselect->SetSelection(operselect->FindString(parms[1]));
			
			wxArrayString intents;
			intents.Add("perceptual");
			intents.Add("saturation");
			intents.Add("relative_colorimetric");
			intents.Add("absolute_colorimetric");
			
			intentselect = new wxRadioBox (this, COLORINTENT, _("rendering Intent"), wxDefaultPosition, wxDefaultSize,  intents, 1, wxRA_SPECIFY_COLS);
			intentselect->SetSelection(intentselect->FindString("relative_colorimetric"));  //default
			if (parms[2] != "-") intentselect->SetSelection(intentselect->FindString(parms[2]));
			if (parms[1] == "assign") intentselect->Enable(false);
			
			bpc = new wxCheckBox(this, COLORBPC, _("black point compensation"));
			//bpc->SetValue(bpc);

			gImage &img = proc->getPreviousPicProcessor()->getProcessedPic();
			wxString makemodelstr = wxString(img.getInfoValue("Make"));
			makemodelstr.Append(" ");
			makemodelstr.Append(wxString(img.getInfoValue("Model")));

			makemodel = new wxStaticText(this,wxID_ANY, makemodelstr); //, wxDefaultPosition, wxSize(30, -1));
			primaries = new wxStaticText(this,wxID_ANY, ""); //, wxDefaultPosition, wxSize(30, -1));
			primarysource = new wxStaticText(this,wxID_ANY, "");
			//camdatstatus = new wxBitmapButton(this, COLORCAMERASTATUS, wxBitmap(listview_xpm), wxPoint(0,0), wxSize(-1,-1), wxBU_EXACTFIT);
			//camdatstatus->SetToolTip(_("Open a dialog to review the sources found and loaded for camera data"));

			std::map<std::string,std::string> p = proc->paramMap(params.ToStdString(), "profile,op,intent,bpc");

			if (p.find("profile") != p.end()) { 
				if (p["profile"] == "camera") {
					camb->SetValue(true);
					cpmode = COLORCAMERA;
				}
				else {
					profileb->SetValue(true);
					cpmode = COLORPROFILE;
					edit->SetValue(wxString(p["profile"]));
					if (p["op"] == "convert") operselect->SetSelection(0); else operselect->SetSelection(1);
					if      (p["intent"] == "perceptual") intentselect->SetSelection(0);
					else if (p["intent"] == "saturation") intentselect->SetSelection(1);
					else if (p["intent"] == "relative_colorimetric") intentselect->SetSelection(2);
					else if (p["intent"] == "absolute_colorimetric") intentselect->SetSelection(3);
				}
			}


			
			myRowSizer *m = new myRowSizer(wxSizerFlags().Expand());
			m->AddRowItem(enablebox, wxSizerFlags(1).Left().Border(wxLEFT|wxTOP));
			m->AddRowItem(new wxBitmapButton(this, COLORCOPY, wxBitmap(copy_xpm), wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT), flags);
			m->AddRowItem(new wxBitmapButton(this, COLORPASTE, wxBitmap(paste_xpm), wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT), flags);
			m->NextRow(wxSizerFlags().Expand());
			m->AddRowItem(new wxStaticLine(this, wxID_ANY), wxSizerFlags(1).Left().Border(wxLEFT|wxRIGHT|wxTOP|wxBOTTOM));

			m->NextRow();
			m->AddRowItem(profileb, flags);

			//edit box grows to select button:
			m->NextRow(wxSizerFlags().Expand());
			m->AddRowItem(edit, wxSizerFlags(1).Left().Border(wxLEFT|wxRIGHT|wxTOP));
			m->AddRowItem(new wxButton(this, COLORFILE, "Select"), wxSizerFlags(0).Right().Border(wxRIGHT|wxTOP));
			m->NextRow();
			m->AddSpacer(5);
			m->AddRowItem(operselect, flags);
			m->AddRowItem(intentselect, flags);
			m->NextRow();
			m->AddRowItem(bpc, flags);

			m->NextRow(wxSizerFlags().Expand());
			m->AddRowItem(new wxStaticLine(this, wxID_ANY), wxSizerFlags(1).Left().Border(wxLEFT|wxRIGHT|wxTOP|wxBOTTOM));

			m->NextRow();
			m->AddSpacer(3);
			m->AddRowItem(camb, flags);
			m->AddRowItem(makemodel, flags);
			//m->AddRowItem(camdatstatus, flags); //stopped populating when I switched to gimage_process
			m->NextRow();
			m->AddRowItem(primaries, flags);
			m->NextRow();
			m->AddRowItem(primarysource, flags);
			m->End();
			SetSizerAndFit(m);
			m->Layout();


			Refresh();
			Update();
			SetFocus();
			Bind(wxEVT_TEXT_ENTER,&ColorspacePanel::paramChanged, this);
			Bind(wxEVT_RADIOBUTTON, &ColorspacePanel::OnRadioButton, this);
			Bind(wxEVT_BUTTON, &ColorspacePanel::selectProfile, this, COLORFILE);
			Bind(wxEVT_BUTTON, &ColorspacePanel::OnCopy, this, COLORCOPY);
			Bind(wxEVT_BUTTON, &ColorspacePanel::OnPaste, this, COLORPASTE);
			Bind(wxEVT_RADIOBOX,&ColorspacePanel::paramChanged, this);
			Bind(wxEVT_CHECKBOX, &ColorspacePanel::paramChanged, this, COLORBPC);
			Bind(wxEVT_CHECKBOX, &ColorspacePanel::onEnable, this, COLORENABLE);
			Bind(wxEVT_BUTTON, &ColorspacePanel::camstatusDialog, this, COLORCAMERASTATUS);
			Bind(wxEVT_CHAR_HOOK, &ColorspacePanel::OnKey,  this);
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

		void OnCopy(wxCommandEvent& event)
		{
			q->copyParamsToClipboard();
			((wxFrame *) GetGrandParent())->SetStatusText(wxString::Format(_("Copied command to clipboard: %s"),q->getCommand()));
			
		}

		void OnPaste(wxCommandEvent& event)
		{
			if (q->pasteParamsFromClipboard()) {

				std::map<std::string,std::string> p = q->paramMap(q->getParams().ToStdString(), "profile,op,intent,bpc");

				if (p.find("profile") != p.end()) { 
					if (p["profile"] == "camera") {
						camb->SetValue(true);
						cpmode = COLORCAMERA;
					}
					else {
						profileb->SetValue(true);
						cpmode = COLORPROFILE;
						edit->SetValue(wxString(p["profile"]));
						if (p["op"] == "convert") operselect->SetSelection(0); else operselect->SetSelection(1);
						if      (p["intent"] == "perceptual") intentselect->SetSelection(0);
						else if (p["intent"] == "saturation") intentselect->SetSelection(1);
						else if (p["intent"] == "relative_colorimetric") intentselect->SetSelection(2);
						else if (p["intent"] == "absolute_colorimetric") intentselect->SetSelection(3);
					}
				}

				q->processPic();
				((wxFrame *) GetGrandParent())->SetStatusText(wxString::Format(_("Pasted command from clipboard: %s"),q->getCommand()));
			}
			else wxMessageBox(wxString::Format(_("Invalid Paste")));
		}


		void OnRadioButton(wxCommandEvent& event)
		{
			cpmode = event.GetId();
			processCS();
		}

		void processCS()
		{
			if (cpmode == COLORPROFILE) {
				wxString profilestr = edit->GetValue();
				wxString operstr = operselect->GetString(operselect->GetSelection());
				wxString intentstr = intentselect->GetString(intentselect->GetSelection());
				if (bpc->GetValue()) intentstr.Append(",bpc");
				if  (operselect->GetSelection() == 1)
					q->setParams(wxString::Format("%s,%s",profilestr, operstr));
				else
					q->setParams(wxString::Format("%s,%s,%s",profilestr, operstr, intentstr));
				q->processPic();
			}
			else if (cpmode == COLORCAMERA) {
				q->setParams("camera,assign");
				q->processPic();
			}
			
		}

		void setParams(wxString profile, wxString oper, wxString intent)
		{
			if  (operselect->GetSelection() == 0)
				q->setParams(wxString::Format("%s,%s",profile, oper));
			else
				q->setParams(wxString::Format("%s,%s,%s",profile, oper, intent));

		}

		void setPrimaries(wxString primstring)
		{
			primaries->SetLabel(primstring);
			primaries->Refresh();
		}

		void setSource(wxString primary_source)
		{
			if (!primary_source.IsEmpty())
				primarysource->SetLabel(wxString::Format(_("Source: %s"),primary_source));
			else
				primarysource->SetLabel("");
			primarysource->Refresh();
		}

		void setMode(int mode)
		{
			cpmode = mode;
			if (mode == COLORPROFILE) {
				profileb->SetValue(true);
				wxString profilestr = edit->GetValue();
				wxString operstr = operselect->GetString(operselect->GetSelection());
				wxString intentstr = intentselect->GetString(intentselect->GetSelection());
				if (bpc->GetValue()) intentstr.Append(",bpc");
				if  (operselect->GetSelection() == 1)
					q->setParams(wxString::Format("%s,%s",profilestr, operstr));
				else
					q->setParams(wxString::Format("%s,%s,%s",profilestr, operstr, intentstr));
			}
			else if (mode == COLORCAMERA) {
				q->setParams("camera,assign");
				camb->SetValue(true);
			}
			
		}


		void selectProfile(wxCommandEvent& event)
		{
			wxFileName fname, pname;
			pname.AssignDir(wxString(myConfig::getConfig().getValueOrDefault("cms.profilepath",((PicProcessorColorSpace *) q)->getOpenFilePath().ToStdString())));

#ifdef WIN32
			pname.SetVolume(pname.GetVolume().MakeUpper());
#endif
			fname.Assign(wxFileSelector(_("Select profile"), pname.GetPath()));
			
			wxString operstr = operselect->GetString(operselect->GetSelection());
			wxString intentstr = intentselect->GetString(intentselect->GetSelection());
			if (bpc->GetValue()) intentstr.Append(",bpc");

			if (fname.FileExists()) {
				edit->SetValue(fname.GetFullName());
				if (pname.GetPath() == fname.GetPath())
					setParams(fname.GetFullName(), operstr,intentstr);
					//q->setParams(wxString::Format("%s,%s,%s",fname.GetFullName(), operstr,intentstr));
				else
					setParams(fname.GetFullPath(), operstr,intentstr);
					//q->setParams(wxString::Format("%s,%s,%s",fname.GetFullPath(), operstr,intentstr));					
				if (cpmode == COLORPROFILE) processCS();
			}
			event.Skip();
		}

		void EnableConvert(bool e)
		{
			operselect->Enable(0,e);
			if (!e) operselect->SetSelection(1);
		}

		void paramChanged(wxCommandEvent& event)
		{
			if (cpmode == COLORPROFILE) processCS();
		}
		
		void setCamdatStatus(wxString status)
		{
			camdat_status = status;
		}
		
		void camstatusDialog(wxCommandEvent& event)
		{
			if (camdat_status != "") {
				myEXIFDialog dlg(this, wxID_ANY, _("Camera Data Status"), camdat_status,  wxDefaultPosition, wxSize(500,250));
				dlg.ShowModal();
			}
		}

/*
		void paramChanged(wxCommandEvent& event)
		{

			wxString profilestr = edit->GetLineText(0);
			wxString operstr = operselect->GetString(operselect->GetSelection());
			if (event.GetId() == COLOROP) {
				if (operstr == "convert") {
					intentselect->Enable(true);
					bpc->Enable(true);
				}
				else {
					intentselect->Enable(false);
					bpc->Enable(false);
				}	
			}
			wxString intentstr = intentselect->GetString(intentselect->GetSelection());
			if (bpc->GetValue()) intentstr.Append(",bpc");

			if (profilestr != "(none)") {
				setParams(profilestr, operstr, intentstr);

				q->processPic();
			}
			event.Skip();
		}
*/

	private:
		wxCheckBox *bpc, *enablebox;
		wxRadioButton *profileb, *camb;
		wxTextCtrl *edit;
		wxRadioBox *operselect, *intentselect;
		wxStaticText *makemodel, *primaries, *primarysource;
		wxString camdat_status;
		wxBitmapButton *camdatstatus;
		int cpmode;

};

PicProcessorColorSpace::PicProcessorColorSpace(wxString name, wxString command, wxTreeCtrl *tree, PicPanel *display): PicProcessor(name, command, tree, display) 
{
	//showParams();
}

void PicProcessorColorSpace::createPanel(wxSimplebook* parent)
{
	toolpanel = new ColorspacePanel(parent, this, c);
	parent->ShowNewPage(toolpanel);
	if (getPreviousPicProcessor()->getProcessedPic().getProfile() == NULL) ((ColorspacePanel *) toolpanel)->EnableConvert(false);
	toolpanel->Refresh();
	toolpanel->Update();
}

void PicProcessorColorSpace::setOpenFilePath(wxString path)
{
	openfilepath = path;
}

wxString PicProcessorColorSpace::getOpenFilePath()
{
	return openfilepath;
}

bool PicProcessorColorSpace::processPicture(gImage *processdib) 
{
	if (!processingenabled) return true;
	
	((wxFrame*) m_display->GetParent())->SetStatusText(_("colorspace..."));
	bool ret = true;
	std::map<std::string,std::string> result;

	std::map<std::string,std::string> params;
	std::string pstr = getParams().ToStdString();
	if (!pstr.empty())
		params = parse_colorspace(std::string(pstr));
	
	if (params.find("error") != params.end()) {
		wxMessageBox(params["error"]);
		ret = false; 
	}
	else if (params.find("mode") == params.end()) {  //all variants need a mode, now...
		wxMessageBox("Error - no mode");
		ret = false;
	}
	else { 
		result = process_colorspace(*dib, params);
		
		if (result.find("error") != result.end()) {
			wxMessageBox(wxString(result["error"]));
			ret = false;
		}
		else {
			if (paramexists(result,"treelabel")) m_tree->SetItemText(id, wxString(result["treelabel"]));
			if (params["mode"] == "camera") ((ColorspacePanel *) toolpanel)->setPrimaries(wxString(result["dcraw_primaries"]));
			//if (paramexists(result,"dcraw_primaries")) ((ColorspacePanel *) toolpanel)->setPrimaries(result["dcraw_primaries"]);
			if (paramexists(result,"dcraw_source")) ((ColorspacePanel *) toolpanel)->setSource(wxString(result["dcraw_source"]));
			m_display->SetModified(true);
			if ((myConfig::getConfig().getValueOrDefault("tool.all.log","0") == "1") || 
				(myConfig::getConfig().getValueOrDefault("tool.colorspace.log","0") == "1"))
					log(wxString::Format(_("tool=colorspace,%s,imagesize=%dx%d,threads=%s,time=%s"),
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


