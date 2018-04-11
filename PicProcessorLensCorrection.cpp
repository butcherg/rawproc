#include "PicProcessorLensCorrection.h"
#include "PicProcPanel.h"
#include "util.h"
#include "myConfig.h"

#include <locale.h>
#include <lensfun/lensfun.h>


class LensCorrectionPanel: public PicProcPanel
{

	public:
		LensCorrectionPanel(wxWindow *parent, PicProcessor *proc, wxString params): PicProcPanel(parent, proc, params)
		{

			wxSizerFlags flags = wxSizerFlags().Left().Border(wxLEFT|wxRIGHT|wxTOP);
			wxArrayString parms = split(params, ",");
			b->Add(new wxStaticText(this,-1, "lenscorrection", wxDefaultPosition, wxSize(100,20)), flags);

			edit = new wxTextCtrl(this, wxID_ANY, "", wxDefaultPosition, wxSize(200,25),wxTE_PROCESS_ENTER);
			b->Add(edit, flags);
			b->Add(new wxButton(this, wxID_ANY, "Select lens"), flags);
			b->AddSpacer(10);
			
			ca = new wxCheckBox(this, wxID_ANY, "chromatic abberation");
			b->Add(ca , flags);
			vig = new wxCheckBox(this, wxID_ANY, "vignetting");
			b->Add(vig , flags);
			dist = new wxCheckBox(this, wxID_ANY, "distortion");
			b->Add(dist , flags);
			
			SetSizerAndFit(b);
			b->Layout();
			Refresh();
			Update();
			SetFocus();
			Bind(wxEVT_TEXT_ENTER,&LensCorrectionPanel::paramChanged, this);
			Bind(wxEVT_BUTTON, &LensCorrectionPanel::selectProfile, this);
			Bind(wxEVT_RADIOBOX,&LensCorrectionPanel::paramChanged, this);
			Bind(wxEVT_CHECKBOX, &LensCorrectionPanel::paramChanged, this);
		}

		~LensCorrectionPanel()
		{
		}
		
		void selectProfile(wxCommandEvent& event)
		{


/*
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
*/
			event.Skip();
		}


		void paramChanged(wxCommandEvent& event)
		{


/*
			wxString profilestr = edit->GetLineText(0);
			wxString operstr = operselect->GetString(operselect->GetSelection());
			wxString intentstr = intentselect->GetString(intentselect->GetSelection());
			if (bpc->GetValue()) intentstr.Append(",bpc");

			if (profilestr != "(none)") {
				q->setParams(wxString::Format("%s,%s,%s",profilestr, operstr, intentstr));
				q->processPic();
			}
*/
			event.Skip();
		}


	private:
		wxCheckBox *ca, *vig, *dist;
		wxTextCtrl *edit;

};

PicProcessorLensCorrection::PicProcessorLensCorrection(wxString name, wxString command, wxTreeCtrl *tree, PicPanel *display): PicProcessor(name, command, tree, display) 
{
	//showParams();
}

void PicProcessorLensCorrection::createPanel(wxSimplebook* parent)
{
	toolpanel = new LensCorrectionPanel(parent, this, c);
	parent->ShowNewPage(toolpanel);
	toolpanel->Refresh();
	toolpanel->Update();
}

bool PicProcessorLensCorrection::processPic(bool processnext) 
{
	((wxFrame*) m_display->GetParent())->SetStatusText("lenscorrection...");
	bool result = true;
	GIMAGE_ERROR ret;
	
	wxArrayString cp = split(getParams(),",");

	int threadcount =  atoi(myConfig::getConfig().getValueOrDefault("tool.lenscorrection.cores","0").c_str());
	if (threadcount == 0) 
		threadcount = gImage::ThreadCount();
	else if (threadcount < 0) 
		threadcount = std::max(gImage::ThreadCount() + threadcount,0);

	mark();
	if (dib) delete dib;
	dib = new gImage(getPreviousPicProcessor()->getProcessedPic());
	//get integer image array, lensfun it, and use it to create the new gImage dib.
	wxString d = duration();

	if ((myConfig::getConfig().getValueOrDefault("tool.all.log","0") == "1") || (myConfig::getConfig().getValueOrDefault("tool.lenscorrection.log","0") == "1"))
		log(wxString::Format("tool=lenscorrection,imagesize=%dx%d,time=%s",dib->getWidth(), dib->getHeight(),d));
	
	dirty = false;

	((wxFrame*) m_display->GetParent())->SetStatusText("");
	if (processnext) processNext();
	
	return result;
}



