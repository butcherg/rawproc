#include "PicProcessorOCIO.h"
#include "PicProcPanel.h"
#include "util.h"
#include "myConfig.h"
#include <OpenColorIO/OpenColorIO.h>
namespace OCIO = OCIO_NAMESPACE;

class OCIOPanel: public PicProcPanel
{

	public:
		OCIOPanel(wxWindow *parent, PicProcessor *proc, wxString params): PicProcPanel(parent, proc, params)
		{
			wxBoxSizer *b = new wxBoxSizer(wxVERTICAL);
			wxBoxSizer *s = new wxBoxSizer(wxHORIZONTAL); 
			wxSizerFlags flags = wxSizerFlags().Left().Border(wxLEFT|wxRIGHT|wxTOP);
			//wxArrayString parms = split(params, ",");
			b->Add(new wxStaticText(this,-1, "ociotransform", wxDefaultPosition, wxSize(100,20)), flags);
			edit = new wxTextCtrl(this, wxID_ANY, "", wxDefaultPosition, wxSize(200,25),wxTE_PROCESS_ENTER);
			b->Add(edit, flags);
			b->Add(new wxButton(this, wxID_ANY, "Select profile"), flags);
			b->AddSpacer(10);

			wxArrayString opers;
			opers.Add("convert");
			opers.Add("assign");

			operselect = new wxRadioBox (this, wxID_ANY, "Operation", wxDefaultPosition, wxDefaultSize,  opers, 1, wxRA_SPECIFY_COLS);
			//if (parms[1] != "-") operselect->SetSelection(operselect->FindString(parms[1]));
			s->Add(operselect,flags);
			
			wxArrayString intents;
			intents.Add("perceptual");
			intents.Add("saturation");
			intents.Add("relative_colorimetric");
			intents.Add("absolute_colorimetric");
			
			intentselect = new wxRadioBox (this, wxID_ANY, "Rendering Intent", wxDefaultPosition, wxDefaultSize,  intents, 1, wxRA_SPECIFY_COLS);
			//if (parms[2] != "-") intentselect->SetSelection(intentselect->FindString(parms[2]));
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
			Bind(wxEVT_TEXT_ENTER,&OCIOPanel::paramChanged, this);
			Bind(wxEVT_BUTTON, &OCIOPanel::selectProfile, this);
			Bind(wxEVT_RADIOBOX,&OCIOPanel::paramChanged, this);
			Bind(wxEVT_CHECKBOX, &OCIOPanel::paramChanged, this);
		}

		~OCIOPanel()
		{
			//if (s) s->~wxBoxSizer();
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


		void paramChanged(wxCommandEvent& event)
		{
			wxString profilestr = edit->GetLineText(0);
			wxString operstr = operselect->GetString(operselect->GetSelection());
			wxString intentstr = intentselect->GetString(intentselect->GetSelection());
			if (bpc->GetValue()) intentstr.Append(",bpc");

			if (profilestr != "(none)") {
				q->setParams(wxString::Format("%s,%s,%s",profilestr, operstr, intentstr));
				q->processPic();
			}
			event.Skip();
		}


	private:
		//wxBoxSizer *s;
		wxCheckBox *bpc;
		wxTextCtrl *edit;
		wxRadioBox *operselect, *intentselect;

};

PicProcessorOCIO::PicProcessorOCIO(wxString name, wxString command, wxTreeCtrl *tree, PicPanel *display): PicProcessor(name, command, tree, display) 
{
	//showParams();
}

void PicProcessorOCIO::createPanel(wxSimplebook* parent)
{
	toolpanel = new OCIOPanel(parent, this, c);
	parent->ShowNewPage(toolpanel);
	toolpanel->Refresh();
	toolpanel->Update();
}

bool PicProcessorOCIO::processPic(bool processnext) 
{
	((wxFrame*) m_display->GetParent())->SetStatusText("ocio...");
	bool result = true;
	GIMAGE_ERROR ret;
	
	wxArrayString cp = split(getParams(),",");

	int threadcount =  atoi(myConfig::getConfig().getValueOrDefault("tool.ocio.cores","0").c_str());
	if (threadcount == 0) 
		threadcount = gImage::ThreadCount();
	else if (threadcount < 0) 
		threadcount = std::max(gImage::ThreadCount() + threadcount,0);

	mark();
	if (dib) delete dib;
	dib = new gImage(getPreviousPicProcessor()->getProcessedPic());
	
	//do OCIO transform on in-place image
	try
	{
		// Get the global OpenColorIO config
		// This will auto-initialize (using $OCIO) on first use
		//OCIO::ConstConfigRcPtr config = OCIO::GetCurrentConfig();
	
		OCIO::ConstConfigRcPtr config = OCIO::Config::Create();
		std::string configfile = myConfig::getConfig().getValueOrDefault("tool.ocio.configpath","");
		configfile.append(myConfig::getConfig().getValue("tool.ocio.config"));
		config = OCIO::Config::CreateFromFile(configfile.c_str());

		// Get the processor corresponding to this transform.
		OCIO::ConstProcessorRcPtr processor = config->getProcessor(OCIO::ROLE_SCENE_LINEAR,
                                                               OCIO::ROLE_DEFAULT);

		// Wrap the image in a light-weight ImageDescription
		//OCIO::PackedImageDesc img((float *)dib->getImageDataRaw(), dib->getWidth(), dib->getHeight(), 3);

		// Apply the color transformation (in place)
		//processor->apply(img);
		
		unsigned w = dib->getWidth();
		unsigned h = dib->getHeight();
		pix * image = dib->getImageDataRaw();
		
		#pragma omp parallel for num_threads(threadcount)
		for (unsigned y=0; y<h; y++) {
			unsigned pos = y*w;
			float * im = (float *) &image[pos];
			OCIO::PackedImageDesc img(im, w, 1, 3);
			processor->apply(img);
		}
		
	}
	catch(OCIO::Exception & exception)
	{
		wxMessageBox(wxString::Format("OCIO Exception: %s", wxString(exception.what())));
	}
	
	wxString d = duration();

	if ((myConfig::getConfig().getValueOrDefault("tool.all.log","0") == "1") || (myConfig::getConfig().getValueOrDefault("tool.ocio.log","0") == "1"))
		log(wxString::Format("tool=ocio,imagesize=%dx%d,time=%s",dib->getWidth(), dib->getHeight(),d));
	
	dirty = false;

	((wxFrame*) m_display->GetParent())->SetStatusText("");
	if (processnext) processNext();
	
	return result;
}



