#include "PicProcessorColorSpace.h"
#include "PicProcPanel.h"
#include "util.h"
//#include "gimage/curve.h"
#include <wx/fileconf.h>


class ColorspacePanel: public PicProcPanel
{

	public:
		ColorspacePanel(wxPanel *parent, PicProcessor *proc, wxString params): PicProcPanel(parent, proc, params)
		{
			wxSizerFlags flags = wxSizerFlags().Left().Border(wxLEFT|wxRIGHT).Expand();
			b->Add(new wxStaticText(this,-1, "colorspace", wxDefaultPosition, wxSize(100,20)), flags);
			edit = new wxTextCtrl(this, wxID_ANY, p, wxDefaultPosition, wxSize(200,20),wxTE_PROCESS_ENTER);
			b->Add(edit, flags);
			SetSizerAndFit(b);
			b->Layout();
			Refresh();
			Update();
			//SetFocus();
			Bind(wxEVT_TEXT_ENTER,&ColorspacePanel::paramChanged, this);
		}

		~ColorspacePanel()
		{
			edit->~wxTextCtrl();
		}

		void paramChanged(wxCommandEvent& event)
		{
			q->setParams(edit->GetLineText(0));
			q->processPic();
			event.Skip();
			Refresh();
			Update();
		}


	private:
		wxTextCtrl *edit;

};

PicProcessorColorSpace::PicProcessorColorSpace(wxString name, wxString command, wxTreeCtrl *tree, PicPanel *display, wxPanel *parameters): PicProcessor(name, command, tree, display, parameters) {
	showParams();

}

void PicProcessorColorSpace::showParams()
{
	if (!m_parameters) return;
	m_parameters->DestroyChildren();
	r = new ColorspacePanel(m_parameters, this, c);
}


bool PicProcessorColorSpace::processPic() 
{
	((wxFrame*) m_display->GetParent())->SetStatusText("colorspace...");
	bool result = true;

	wxFileName fname;
	fname.AssignDir(wxConfigBase::Get()->Read("cms.profilepath",""));
	fname.SetFullName(c);

	mark();
	if (dib) delete dib;
	dib = new gImage(getPreviousPicProcessor()->getProcessedPic());
	dib->ApplyColorspace(std::string(fname.GetFullPath().c_str()),INTENT_RELATIVE_COLORIMETRIC);
	wxString d = duration();

	if ((wxConfigBase::Get()->Read("tool.all.log","0") == "1") || (wxConfigBase::Get()->Read("tool.colorspace.log","0") == "1"))
		log(wxString::Format("tool=colorspace,imagesize=%dx%d,time=%s",dib->getWidth(), dib->getHeight(),d));

	dirty = false;

	((wxFrame*) m_display->GetParent())->SetStatusText("");
	processNext();
	
	return result;
}





