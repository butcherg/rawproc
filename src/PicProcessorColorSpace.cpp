#include "PicProcessorColorSpace.h"
#include "PicProcPanel.h"
#include "util.h"
#include "myConfig.h"

#define COLOROP 6500
#define COLORINTENT 6501


class ColorspacePanel: public PicProcPanel
{

	public:
		ColorspacePanel(wxWindow *parent, PicProcessor *proc, wxString params): PicProcPanel(parent, proc, params)
		{

			s = new wxBoxSizer(wxHORIZONTAL); 
			wxSizerFlags flags = wxSizerFlags().Left().Border(wxLEFT|wxRIGHT|wxTOP);
			wxArrayString parms = split(params, ",");
			b->Add(new wxStaticText(this,-1, "colorspace", wxDefaultPosition, wxSize(100,20)), flags);
			edit = new wxTextCtrl(this, wxID_ANY, parms[0], wxDefaultPosition, wxSize(200,25),wxTE_PROCESS_ENTER);
			b->Add(edit, flags);
			b->Add(new wxButton(this, wxID_ANY, "Select profile"), flags);
			b->AddSpacer(10);

			wxArrayString opers;
			opers.Add("convert");
			opers.Add("assign");

			operselect = new wxRadioBox (this, COLOROP, "Operation", wxDefaultPosition, wxDefaultSize,  opers, 1, wxRA_SPECIFY_COLS);
			if (parms[1] != "-") operselect->SetSelection(operselect->FindString(parms[1]));
			s->Add(operselect,flags);
			
			wxArrayString intents;
			intents.Add("perceptual");
			intents.Add("saturation");
			intents.Add("relative_colorimetric");
			intents.Add("absolute_colorimetric");
			
			intentselect = new wxRadioBox (this, COLORINTENT, "Rendering Intent", wxDefaultPosition, wxDefaultSize,  intents, 1, wxRA_SPECIFY_COLS);
			if (parms[2] != "-") intentselect->SetSelection(intentselect->FindString(parms[2]));
			s->Add(intentselect,flags);
			
			b->Add(s,flags);
			
			bpc = new wxCheckBox(this, wxID_ANY, "black point compensation");
			b->Add(bpc , flags);
			//bpc->SetValue(bpc);
			
			SetSizerAndFit(b);
			b->Layout();
			Refresh();
			Update();
			SetFocus();
			Bind(wxEVT_TEXT_ENTER,&ColorspacePanel::paramChanged, this);
			Bind(wxEVT_BUTTON, &ColorspacePanel::selectProfile, this);
			Bind(wxEVT_RADIOBOX,&ColorspacePanel::paramChanged, this);
			Bind(wxEVT_CHECKBOX, &ColorspacePanel::paramChanged, this);
		}

		~ColorspacePanel()
		{

		}
		
		void selectProfile(wxCommandEvent& event)
		{
			wxFileName fname, pname;
			pname.AssignDir(wxString(myConfig::getConfig().getValueOrDefault("cms.profilepath","").c_str()));
#ifdef WIN32
			pname.SetVolume(pname.GetVolume().MakeUpper());
#endif
			fname.Assign(wxFileSelector("Select profile", pname.GetPath()));
			
			wxString operstr = operselect->GetString(operselect->GetSelection());
			wxString intentstr = intentselect->GetString(intentselect->GetSelection());
			if (bpc->GetValue()) intentstr.Append(",bpc");

			if (fname.FileExists()) {
				edit->SetValue(fname.GetFullName());
				if (pname.GetPath() == fname.GetPath())
					q->setParams(wxString::Format("%s,%s,%s",fname.GetFullName(), operstr,intentstr));
				else
					q->setParams(wxString::Format("%s,%s,%s",fname.GetFullPath(), operstr,intentstr));					
				q->processPic();
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
				q->setParams(wxString::Format("%s,%s,%s",profilestr, operstr, intentstr));
				q->processPic();
			}
			event.Skip();
		}


	private:
		wxBoxSizer *s;
		wxCheckBox *bpc;
		wxTextCtrl *edit;
		wxRadioBox *operselect, *intentselect;

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

bool PicProcessorColorSpace::processPic(bool processnext) 
{
	((wxFrame*) m_display->GetParent())->SetStatusText("colorspace...");
	bool result = true;
	GIMAGE_ERROR ret;
	
	wxArrayString cp = split(getParams(),",");
	wxString intentstr;
	bool bpc = false;
	if (cp.Count() >=3) intentstr = cp[2];
	if (cp.Count() >=4) if (cp[3] == "bpc") bpc =  true;
	
	if (bpc) ((wxFrame*) m_display->GetParent())->SetStatusText("colorspace w/bpc...");
	
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

	mark();
	if (dib) delete dib;
	dib = new gImage(getPreviousPicProcessor()->getProcessedPic());
	
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
					break;
				case GIMAGE_APPLYCOLORSPACE_BADPROFILE:
					wxMessageBox("ColorSpace apply: no input profile in image.");
					result = false;
					break;
				case GIMAGE_APPLYCOLORSPACE_BADINTENT:
					wxMessageBox("ColorSpace apply: input/output profile doesn't support rendering intent.");
					result = false;
					break;
/*
				case 3:
					wxMessageBox("ColorSpace apply: output profile doesn't support rendering intent.");
					result = false;
					break;
*/
				case GIMAGE_APPLYCOLORSPACE_BADTRANSFORM:
					wxMessageBox("ColorSpace apply: colorspace transform creation failed.");
					result = false;
					break;
				default:
					result = false;
			}
			wxString d = duration();

			if (result)
				if ((myConfig::getConfig().getValueOrDefault("tool.all.log","0") == "1") || (myConfig::getConfig().getValueOrDefault("tool.colorspace.log","0") == "1"))

					log(wxString::Format("tool=colorspace_convert,imagesize=%dx%d,threads=%d,time=%s",dib->getWidth(), dib->getHeight(),threadcount,d));
		}
		else if (cp[1] == "assign") {
			if (dib->AssignColorspace(std::string(fname.GetFullPath().c_str())) != GIMAGE_OK) {
				wxMessageBox("ColorSpace assign failed.");
				result = false;
			}
			wxString d = duration();

			if (result) 
				if ((myConfig::getConfig().getValueOrDefault("tool.all.log","0") == "1") || (myConfig::getConfig().getValueOrDefault("tool.colorspace.log","0") == "1"))
					log(wxString::Format("tool=colorspace_assign,imagesize=%dx%d,time=%s",dib->getWidth(), dib->getHeight(),d));
		}
	
	}
	else if (cp[0] != "(none)") {
		wxMessageBox(wxString::Format("profile %s not found.",fname.GetFullName().c_str()));
	}
	
	dirty = false;

	((wxFrame*) m_display->GetParent())->SetStatusText("");
	if (processnext) processNext();
	
	return result;
}



