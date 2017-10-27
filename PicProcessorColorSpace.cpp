#include "PicProcessorColorSpace.h"
#include "PicProcPanel.h"
#include "util.h"
//#include "gimage/curve.h"
#include <wx/fileconf.h>


class ColorspacePanel: public PicProcPanel
{

	public:
		ColorspacePanel(wxWindow *parent, PicProcessor *proc, wxString params): PicProcPanel(parent, proc, params)
		{

			s = new wxBoxSizer(wxHORIZONTAL); 
			wxSizerFlags flags = wxSizerFlags().Left().Border(wxLEFT|wxRIGHT);
			wxArrayString parms = split(params, ",");
			b->Add(new wxStaticText(this,-1, "colorspace", wxDefaultPosition, wxSize(100,20)), flags);
			edit = new wxTextCtrl(this, wxID_ANY, parms[0], wxDefaultPosition, wxSize(200,25),wxTE_PROCESS_ENTER);
			b->Add(edit, flags);
			b->Add(new wxButton(this, wxID_ANY, "Select profile"), flags);
			b->AddSpacer(10);

			wxArrayString opers;
			opers.Add("convert");
			opers.Add("assign");

			operselect = new wxRadioBox (this, wxID_ANY, "Operation", wxDefaultPosition, wxDefaultSize,  opers, 1, wxRA_SPECIFY_COLS);
			if (parms[1] != "-") operselect->SetSelection(operselect->FindString(parms[1]));
			s->Add(operselect,flags);
			
			wxArrayString intents;
			intents.Add("perceptual");
			intents.Add("saturation");
			intents.Add("relative_colorimetric");
			intents.Add("absolute_colorimetric");
			
			intentselect = new wxRadioBox (this, wxID_ANY, "Rendering Intent", wxDefaultPosition, wxDefaultSize,  intents, 1, wxRA_SPECIFY_COLS);
			if (parms[2] != "-") intentselect->SetSelection(intentselect->FindString(parms[1]));
			s->Add(intentselect,flags);
			
			b->Add(s,flags);
			
			SetSizerAndFit(b);
			b->Layout();
			Refresh();
			Update();
			SetFocus();
			Bind(wxEVT_TEXT_ENTER,&ColorspacePanel::paramChanged, this);
			Bind(wxEVT_BUTTON, &ColorspacePanel::selectProfile, this);
			Bind(wxEVT_RADIOBOX,&ColorspacePanel::paramChanged, this);
		}

		~ColorspacePanel()
		{
			if (s) s->~wxBoxSizer();
		}
		
		void selectProfile(wxCommandEvent& event)
		{
			wxFileName fname, pname;
			pname.AssignDir(wxConfigBase::Get()->Read("cms.profilepath",""));
#ifdef WIN32
			pname.SetVolume(pname.GetVolume().MakeUpper());
#endif
			fname.Assign(wxFileSelector("Select profile", pname.GetPath()));
			
			wxString operstr = operselect->GetString(operselect->GetSelection());
			wxString intentstr = intentselect->GetString(intentselect->GetSelection());

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

		void paramChanged(wxCommandEvent& event)
		{
			//wxString intentstr = wxConfigBase::Get()->Read("display.cms.renderingintent","perceptual");
			wxString profilestr = edit->GetLineText(0);
			wxString operstr = operselect->GetString(operselect->GetSelection());
			wxString intentstr = intentselect->GetString(intentselect->GetSelection());

			if (profilestr != "(none)") {
				q->setParams(wxString::Format("%s,%s,%s",profilestr, operstr, intentstr));
				q->processPic();
			}
			event.Skip();
		}


	private:
		wxBoxSizer *s;
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
	toolpanel->Refresh();
	toolpanel->Update();
}

bool PicProcessorColorSpace::processPic(bool processnext) 
{
	((wxFrame*) m_display->GetParent())->SetStatusText("colorspace...");
	bool result = true;
	int ret;
	
	wxArrayString cp = split(getParams(),",");
	wxString intentstr;
	if (cp.Count() >=3) intentstr = cp[2];
	
	cmsUInt32Number intent = INTENT_PERCEPTUAL;
	if (intentstr == "perceptual") intent = INTENT_PERCEPTUAL;
	if (intentstr == "saturation") intent = INTENT_SATURATION;
	if (intentstr == "relative_colorimetric") intent = INTENT_RELATIVE_COLORIMETRIC;
	if (intentstr == "absolute_colorimetric") intent = INTENT_ABSOLUTE_COLORIMETRIC;

	wxFileName fname;
	fname.AssignDir(wxConfigBase::Get()->Read("cms.profilepath",""));
	if (cp.GetCount() > 0) fname.SetFullName(cp[0]);

	int threadcount;
	wxConfigBase::Get()->Read("tool.contrast.cores",&threadcount,0);
	if (threadcount == 0) 
		threadcount = gImage::ThreadCount();
	else if (threadcount < 0) 
		threadcount = std::max(gImage::ThreadCount() + threadcount,0);

	mark();
	if (dib) delete dib;
	dib = new gImage(getPreviousPicProcessor()->getProcessedPic());
	
	if (fname.IsOk() & fname.FileExists()) {

		if (cp[1] == "convert") {
			ret = dib->ApplyColorspace(std::string(fname.GetFullPath().c_str()),intent, threadcount);
			switch (ret) {
				case 0:
					result = true;
					break;
				case 1:
					wxMessageBox("ColorSpace apply: no input profile in image.");
					result = false;
					break;
				case 2:
					wxMessageBox("ColorSpace apply: input profile doesn't support rendering intent.");
					result = false;
					break;
				case 3:
					wxMessageBox("ColorSpace apply: output profile doesn't support rendering intent.");
					result = false;
					break;
				case 4:
					wxMessageBox("ColorSpace apply: colorspace transform creation failed.");
					result = false;
					break;
				default:
					result = false;
			}
			wxString d = duration();

			if (result)
				if ((wxConfigBase::Get()->Read("tool.all.log","0") == "1") || (wxConfigBase::Get()->Read("tool.colorspace.log","0") == "1"))
					log(wxString::Format("tool=colorspace_convert,imagesize=%dx%d,threads=%d,time=%s",dib->getWidth(), dib->getHeight(),threadcount,d));
		}
		else if (cp[1] == "assign") {
			if (!dib->AssignColorspace(std::string(fname.GetFullPath().c_str()))) {
				wxMessageBox("ColorSpace assign failed.");
				result = false;
			}
			wxString d = duration();

			if (result) 
				if ((wxConfigBase::Get()->Read("tool.all.log","0") == "1") || (wxConfigBase::Get()->Read("tool.colorspace.log","0") == "1"))
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



