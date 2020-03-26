#include "PicProcessorColorSpace.h"
#include "PicProcPanel.h"
#include "myRowSizer.h"
#include "util.h"
#include "gimage/strutil.h"
#include "fileutil.h"
#include "myConfig.h"
#include "myEXIFDialog.h"
#include "CameraData.h"
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
			camdatstatus = new wxBitmapButton(this, COLORCAMERASTATUS, wxBitmap(listview_xpm), wxPoint(0,0), wxSize(-1,-1), wxBU_EXACTFIT);
			camdatstatus->SetToolTip(_("Open a dialog to review the sources found and loaded for camera data"));

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
			m->AddRowItem(camdatstatus, flags);
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
	((wxFrame*) m_display->GetParent())->SetStatusText(_("colorspace..."));
	bool result = true;
	GIMAGE_ERROR ret;
	
	wxArrayString cp = split(getParams(),",");
	if (cp.Count() >= 8) {
		for (int i=0; i<8; i++) {
			cp[0].Append(wxString::Format(",%s",cp[1]));
			cp.RemoveAt(1);
		}
	}

	wxString intentstr;
	bool bpc = false;
	if (cp.Count() >=3) intentstr = cp[2];
	if (cp.Count() >=4) if (cp[3] == "bpc") bpc =  true;
	
	cmsUInt32Number intent = INTENT_PERCEPTUAL;
	if (intentstr == "perceptual") intent = INTENT_PERCEPTUAL;
	if (intentstr == "saturation") intent = INTENT_SATURATION;
	if (intentstr == "relative_colorimetric") intent = INTENT_RELATIVE_COLORIMETRIC;
	if (intentstr == "absolute_colorimetric") intent = INTENT_ABSOLUTE_COLORIMETRIC;

	wxFileName fname;
	fname.AssignDir(wxString(myConfig::getConfig().getValueOrDefault("cms.profilepath","").c_str()));
	if (cp.GetCount() > 0) fname.SetFullName(cp[0]);

	int threadcount =  atoi(myConfig::getConfig().getValueOrDefault("tool.colorspace.cores","0").c_str());
	if (threadcount == 0) 
		threadcount = gImage::ThreadCount();
	else if (threadcount < 0) 
		threadcount = std::max(gImage::ThreadCount() + threadcount,0);

	dib = processdib;

	if (!global_processing_enabled) return true;

	if (processingenabled) {
		mark();
	
		if (fname.IsOk() & fname.FileExists()) {

			if (cp[1] == "convert") {
				if (fname.GetExt() == "json") {
					FILE* f = fopen(fname.GetFullPath().c_str(), "r");
	
					fseek(f, 0, SEEK_END);
					size_t size = ftell(f);
	
					char* jsonprof = new char[size];
	
					rewind(f);
					size_t result = fread(jsonprof, sizeof(char), size, f);
					ret = dib->ApplyColorspace(std::string(jsonprof),intent, bpc, threadcount);
					delete[] jsonprof;
				}
				else {
					ret = dib->ApplyColorspace(std::string(fname.GetFullPath().c_str()),intent, bpc, threadcount);
				}
				switch (ret) {
					case GIMAGE_OK:
						result = true;
						m_tree->SetItemText(id, wxString::Format(_("colorspace:profile,convert")));
						m_display->SetModified(true);
						break;
					case GIMAGE_APPLYCOLORSPACE_BADPROFILE:
						wxMessageBox(_("ColorSpace apply: no input profile in image."));
						result = false;
						break;
					case GIMAGE_APPLYCOLORSPACE_BADINTENT_INPUT:
						wxMessageBox(_("ColorSpace apply: input profile doesn't support rendering intent."));
						result = false;
						break;
					case GIMAGE_APPLYCOLORSPACE_BADINTENT_OUTPUT:
						wxMessageBox(_("ColorSpace apply: output profile doesn't support rendering intent."));
						result = false;
						break;
/*
					case 3:
					wxMessageBox(_("ColorSpace apply: output profile doesn't support rendering intent."));
						result = false;
						break;
*/
					case GIMAGE_APPLYCOLORSPACE_BADTRANSFORM:
						wxMessageBox(_("ColorSpace apply: colorspace transform creation failed."));
						result = false;
						break;
					default:
						result = false;
				}
				wxString d = duration();

				if (result)
					if ((myConfig::getConfig().getValueOrDefault("tool.all.log","0") == "1") || (myConfig::getConfig().getValueOrDefault("tool.colorspace.log","0") == "1"))
							log(wxString::Format(_("tool=colorspace_convert,imagesize=%dx%d,threads=%d,time=%s"),dib->getWidth(), dib->getHeight(),threadcount,d));
			}
			else if (cp[1] == "assign") {
				if (dib->AssignColorspace(std::string(fname.GetFullPath().c_str())) != GIMAGE_OK) {
					wxMessageBox(_("ColorSpace assign failed."));
					result = false;
				}
				else m_tree->SetItemText(id, wxString::Format(_("colorspace:profile,assign")));
				wxString d = duration();

				if (result) 
					if ((myConfig::getConfig().getValueOrDefault("tool.all.log","0") == "1") || (myConfig::getConfig().getValueOrDefault("tool.colorspace.log","0") == "1"))
						log(wxString::Format(_("tool=colorspace_assign,imagesize=%dx%d,time=%s"),dib->getWidth(), dib->getHeight(),d));
			}
	
		}

		else if (cp[0] == "camera") {

			std::string makemodel = dib->getInfoValue("Make");
			makemodel.append(" ");
			makemodel.append(dib->getInfoValue("Model"));

			std::string dcrawpath;
			std::string camconstpath;

			if (dcraw_primaries.empty()) { //Look in dcraw.c and camconst.json
				CameraData c;
				dcrawpath = c.findFile("dcraw.c","tool.colorspace.dcrawpath");
				camconstpath = c.findFile("camconst.json","tool.colorspace.camconstpath");
				if (file_exists(dcrawpath)) c.parseDcraw(dcrawpath);
				if (file_exists(camconstpath)) c.parseCamconst(camconstpath);
				dcraw_primaries = c.getItem(makemodel, "dcraw_matrix");
				primary_source = c.getItem(makemodel, "primary_source");
				((ColorspacePanel *) toolpanel)->setCamdatStatus(wxString(c.getStatus()));
			}
			
			if (dcraw_primaries.empty()) { //Last resort, look in the LibRaw metadata
				std::string libraw_primaries = dib->getInfoValue("Libraw.CamXYZ");
				std::vector<std::string> primaries = split(libraw_primaries, ",");
				if (primaries.size() >= 9 & atof(primaries[0].c_str()) != 0.0) {
					dcraw_primaries = string_format("%d,%d,%d,%d,%d,%d,%d,%d,%d",
						int(atof(primaries[0].c_str()) * 10000),
						int(atof(primaries[1].c_str()) * 10000),
						int(atof(primaries[2].c_str()) * 10000),
						int(atof(primaries[3].c_str()) * 10000),
						int(atof(primaries[4].c_str()) * 10000),
						int(atof(primaries[5].c_str()) * 10000),
						int(atof(primaries[6].c_str()) * 10000),
						int(atof(primaries[7].c_str()) * 10000),
						int(atof(primaries[8].c_str()) * 10000));
					primary_source = "LibRaw";
				}
			}

			if (!dcraw_primaries.empty()) {
				std::string cam =  dcraw_primaries.ToStdString();
				((ColorspacePanel *) toolpanel)->setPrimaries(dcraw_primaries);
				((ColorspacePanel *) toolpanel)->setSource(primary_source);
				if (cp[1] == "convert") {
					if (dib->ApplyColorspace(cam,intent, bpc, threadcount) != GIMAGE_OK) {
						wxMessageBox(_("ColorSpace convert failed."));
						result = false;
					}
					else {
						m_tree->SetItemText(id, wxString::Format(_("colorspace:camera,convert")));
						m_display->SetModified(true);
					}
					wxString d = duration();

					if (result) 
						if ((myConfig::getConfig().getValueOrDefault("tool.all.log","0") == "1") || (myConfig::getConfig().getValueOrDefault("tool.colorspace.log","0") == "1"))
							log(wxString::Format(_("tool=colorspace_convert,imagesize=%dx%d,time=%s"),dib->getWidth(), dib->getHeight(),d));
				}
				else if (cp[1] == "assign") {
					if (dib->AssignColorspace(cam) != GIMAGE_OK) {
						wxMessageBox(_("ColorSpace assign failed."));
						result = false;
					}
					else m_tree->SetItemText(id, wxString::Format(_("colorspace:camera,assign")));
					wxString d = duration();

					if (result) 
						if ((myConfig::getConfig().getValueOrDefault("tool.all.log","0") == "1") || (myConfig::getConfig().getValueOrDefault("tool.colorspace.log","0") == "1"))
							log(wxString::Format(_("tool=colorspace_assign,imagesize=%dx%d,time=%s"),dib->getWidth(), dib->getHeight(),d));
				}
			}
			else {
				((ColorspacePanel *) toolpanel)->setMode(COLORPROFILE);
				wxMessageBox(wxString::Format(_("primaries not found for -%s-"),wxString(makemodel)));
			}
		}

		else if (cp[0].Freq(',') == 8 ) { //comma-separated adobe_coeff string (e.g., e.g., D7000: 8198,-2239,-724,-4871,12389,2798,-1043,2050,7181), make a D65 profile from it
			if (cp[1] == "convert") {
				if (dib->ApplyColorspace(cp[0].ToStdString(),intent, bpc, threadcount) != GIMAGE_OK) {
					wxMessageBox(_("ColorSpace convert failed."));
					result = false;
				}
				else m_tree->SetItemText(id, wxString::Format(_("colorspace:dcraw,convert")));
				wxString d = duration();

				if (result) 
					if ((myConfig::getConfig().getValueOrDefault("tool.all.log","0") == "1") || (myConfig::getConfig().getValueOrDefault("tool.colorspace.log","0") == "1"))
						log(wxString::Format(_("tool=colorspace_convert,imagesize=%dx%d,time=%s"),dib->getWidth(), dib->getHeight(),d));
			}
			else if (cp[1] == "assign") {
				if (dib->AssignColorspace(cp[0].ToStdString()) != GIMAGE_OK) {
					wxMessageBox(_("ColorSpace assign failed."));
					result = false;
				}
				else m_tree->SetItemText(id, wxString::Format(_("colorspace:dcraw,assign")));
				wxString d = duration();

				if (result) 
					if ((myConfig::getConfig().getValueOrDefault("tool.all.log","0") == "1") || (myConfig::getConfig().getValueOrDefault("tool.colorspace.log","0") == "1"))
						log(wxString::Format(_("tool=colorspace_assign,imagesize=%dx%d,time=%s"),dib->getWidth(), dib->getHeight(),d));
			}
		}

		else if (cp[0] == "srgb" | cp[0] == "wide" | cp[0] == "adobe" | cp[0] == "prophoto" | cp[0] == "identity") {
			if (cp[1] == "convert") {
				if (dib->ApplyColorspace(cp[0].ToStdString(), intent, bpc, threadcount) != GIMAGE_OK) {
					wxMessageBox(_("ColorSpace convert failed."));
					result = false;
				}
				else m_tree->SetItemText(id, wxString::Format(_("colorspace:%s,convert"),wxString(cp[0])));
				wxString d = duration();
			}
			else if (cp[1] == "assign") {
				if (dib->AssignColorspace(cp[0].ToStdString()) != GIMAGE_OK) {
					wxMessageBox(_("ColorSpace assign failed."));
					result = false;
				}
				else m_tree->SetItemText(id, wxString::Format(_("colorspace:%s,assign"),wxString(cp[0])));
				wxString d = duration();
			}
		}

		else if (cp[0] != "(none)") {
			wxMessageBox(wxString::Format(_("profile %s not found."),fname.GetFullName().c_str()));
		}
	}  //processingenabled
	
	dirty = false;

	((wxFrame*) m_display->GetParent())->SetStatusText("");
	
	return result;
}



