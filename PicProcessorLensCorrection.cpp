#include "PicProcessorLensCorrection.h"
#include "PicProcPanel.h"
#include "util.h"
#include "gimage/strutil.h"
#include "myConfig.h"

class LensCorrectionPanel: public PicProcPanel
{

	public:
		LensCorrectionPanel(wxWindow *parent, PicProcessor *proc, wxString params, wxString metadata): PicProcPanel(parent, proc, params)
		{
			wxSizerFlags flags = wxSizerFlags().Left().Border(wxLEFT|wxRIGHT|wxTOP);
			wxArrayString parms = split(params, ",");
			b->Add(new wxStaticText(this,-1, "lenscorrection", wxDefaultPosition, wxSize(100,20)), flags);
			b->AddSpacer(5);

			b->Add(new wxStaticText(this,-1, metadata, wxDefaultPosition, wxSize(260,40)), flags);

			cam = new wxTextCtrl(this, wxID_ANY, "", wxDefaultPosition, wxSize(200,25),wxTE_PROCESS_ENTER);
			b->Add(cam, flags);
			b->Add(new wxButton(this, wxID_ANY, "Select camera"), flags);
			lens = new wxTextCtrl(this, wxID_ANY, "", wxDefaultPosition, wxSize(200,25),wxTE_PROCESS_ENTER);
			b->Add(lens, flags);
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
			Bind(wxEVT_BUTTON, &LensCorrectionPanel::setAlternates, this);
			Bind(wxEVT_RADIOBOX,&LensCorrectionPanel::paramChanged, this);
			Bind(wxEVT_CHECKBOX, &LensCorrectionPanel::paramChanged, this);
		}

		~LensCorrectionPanel()
		{
		}
		
		void setAlternates(wxCommandEvent& event)
		{
			wxString altcam = cam->GetValue();
			wxString altlens = lens->GetValue();
			((PicProcessorLensCorrection *) q)->setAlternates(altcam, altlens);
		}

		void paramChanged(wxCommandEvent& event)
		{
			wxString cmd = "";
			wxString altcam = cam->GetValue();
			if (altcam != "") paramAppend("camera", altcam, cmd);
			wxString altlens = lens->GetValue();
			if (altlens != "") paramAppend("lens",altlens, cmd);

			wxString ops = "";
			if (ca->GetValue()) opAppend("ca",ops);
			if (vig->GetValue()) opAppend("vig",ops);
			if (dist->GetValue()) opAppend("dist",ops);

			if (ops != "") paramAppend("ops", ops, cmd);

			q->setParams(cmd);
			q->processPic();
			event.Skip();
		}


	private:
		wxCheckBox *ca, *vig, *dist;
		wxTextCtrl *cam, *lens;

};

PicProcessorLensCorrection::PicProcessorLensCorrection(wxString name, wxString command, wxTreeCtrl *tree, PicPanel *display): PicProcessor(name, command, tree, display) 
{
	lfok = true;
	setlocale (LC_ALL, "");			
	
	lfError e;
	ldb = lf_db_new ();
	if (lf_db_load (ldb) != LF_NO_ERROR) {
		wxMessageBox("Error: Cannot open lens correction database.  Delete the tool, correct the problem, and re-add the tool") ;
		lfok = false;
	}

	gImage &idib = getPreviousPicProcessor()->getProcessedPic();
	std::map<std::string,std::string> info = idib.getInfo();

	if (info.find("Model") != info.end())
		metadatacamera = wxString(info["Model"].c_str());
	else
		metadatacamera = "(none)";

	if (info.find("Lens") != info.end())
		metadatalens = wxString(info["Lens"].c_str());
	else
		metadatalens = "(none)";

}


PicProcessorLensCorrection::~PicProcessorLensCorrection()
{
	if (ldb) lf_db_destroy (ldb);
}

void PicProcessorLensCorrection::createPanel(wxSimplebook* parent)
{
	if (lfok) {
		gImage &idib = getPreviousPicProcessor()->getProcessedPic();
		wxString metadata = wxString::Format("Camera: %s\nLens: %s",metadatacamera, metadatalens);
		toolpanel = new LensCorrectionPanel(parent, this, c, metadata);
		parent->ShowNewPage(toolpanel);
		toolpanel->Refresh();
		toolpanel->Update();
	}
}

void PicProcessorLensCorrection::setAlternates(wxString acam, wxString alens)
{
	altcamera = acam;
	altlens = alens;
}

bool PicProcessorLensCorrection::processPic(bool processnext) 
{
	((wxFrame*) m_display->GetParent())->SetStatusText("lenscorrection...");
	bool result = true;
	GIMAGE_ERROR ret;


	
	//wxArrayString cp = split(getParams(),",");
	std::map<std::string, std::string> cp = parseparams(std::string(getParams().c_str()));

	int threadcount =  atoi(myConfig::getConfig().getValueOrDefault("tool.lenscorrection.cores","0").c_str());
	if (threadcount == 0) 
		threadcount = gImage::ThreadCount();
	else if (threadcount < 0) 
		threadcount = std::max(gImage::ThreadCount() + threadcount,0);

	mark();
	if (dib) delete dib;
	dib = new gImage(getPreviousPicProcessor()->getProcessedPic());
	pix * newimg = dib->getImageDataRaw();
	
	gImage * olddib = new gImage(getPreviousPicProcessor()->getProcessedPic());
	std::map<std::string, std::string> info = olddib->getInfo();
	pix * img = olddib->getImageDataRaw();

	const lfCamera *cam = NULL;
	const lfCamera ** cameras = ldb->FindCamerasExt(NULL, info["Model"].c_str());
	if (cameras)
		cam = cameras[0];
	else
		wxMessageBox("Cannot find a camera matching `%s' in database\n", info["Model"].c_str());
	lf_free (cameras);

	// try to find a matching lens in the database
	const lfLens *lens = NULL;
	const lfLens **lenses = ldb->FindLenses (cam, NULL, info["Lens"].c_str());
	if (lenses)
		lens = lenses [0];
	else
		wxMessageBox("Cannot find a lens matching `%s' in database\n", info["Lens"].c_str());
	lf_free (lenses);
	//wxMessageBox(wxString::Format("Lens: %s",lens->Model));
	
	int ModifyFlags = 0;  //ops=ca,vig,dist 
	if (cp.find("ops") != cp.end()) {
		std::vector<std::string> ops = split(cp["ops"], ",");
		for (unsigned i=0; i<ops.size(); i++) {
			if (ops[i] == "ca")   ModifyFlags |= LF_MODIFY_TCA;
			if (ops[i] == "vig")  ModifyFlags |= LF_MODIFY_VIGNETTING;
			if (ops[i] == "dist") ModifyFlags |= LF_MODIFY_DISTORTION;
		}
	}
	
	lfModifier *mod = lfModifier::Create (lens, lens->CropFactor, dib->getWidth(), dib->getHeight());
	int modflags = mod->Initialize (
        lens, 
		LF_PF_F32, 
		atof(info["FocalLength"].c_str()), 
		atof(info["FNumber"].c_str()),
        1.0f, //opts.Distance
		1.0f, //opts.Scale, 
		LF_RECTILINEAR, //opts.TargetGeom,
        ModifyFlags, 
		false //opts.Inverse
	);
	
	unsigned w = dib->getWidth();
	unsigned h = dib->getHeight();
	
	if (ModifyFlags & LF_MODIFY_TCA) {
		bool ok = true;
		int lwidth = w * 2;
		float *pos = new float [lwidth];
		for (unsigned y = 0; ok && y < h; y++) {
			ok = mod->ApplySubpixelDistortion (0.0, y, w, 1, pos);
			if (ok)
			{
				float *src = pos;
				for (unsigned x = 0; x < w; x++)
				{
					unsigned p = x + y*h;
					newimg[p].r = img[(unsigned) (src[0] + src[1] * w)].r; //img->GetR (src [0], src [1]);
					newimg[p].g = img[(unsigned) (src[2] + src[3] * w)].g;//img->GetG (src [2], src [3]);
					newimg[p].b = img[(unsigned) (src[4] + src[5] * w)].b;//img->GetB (src [4], src [5]);
					src += 2 * 3;
					///dst++;
				}
			}
		}
	}
	
	
/*	
	
//#ifdef COMBINE_13
//    int lwidth = img->width * 2 * 3;
//#else
    int lwidth = img->width * 2;
    if (modflags & LF_MODIFY_TCA)
        lwidth *= 3;
//#endif
    float *pos = new float [lwidth];
    int step_start = reverse ? 2 : 0;
    int step_delta = reverse ? -1 : +1;
    int step_finish = reverse ? -1 : 3;
    for (int step = step_start; step != step_finish; step += step_delta)
    {
        RGBpixel *dst = newimg->image;
        char *imgdata = (char *)img->image;
        bool ok = true;
        img->InitInterpolation (opts.Interpolation);
        for (unsigned y = 0; ok && y < img->height; y++)
            switch (step)
            {
//#ifdef COMBINE_13
//                case 0:
//                    ok = false;
//                    break;
//                case 2:
//                    // TCA and geometry correction 
//                    ok = mod->ApplySubpixelGeometryDistortion (0.0, y, img->width, 1, pos);
//#else
                case 0:
                    // TCA correction 
                    ok = mod->ApplySubpixelDistortion (0.0, y, img->width, 1, pos);
//#endif
                    if (ok)
                    {
                        float *src = pos;
                        for (unsigned x = 0; x < img->width; x++)
                        {
                            dst->red   = img->GetR (src [0], src [1]);
                            dst->green = img->GetG (src [2], src [3]);
                            dst->blue  = img->GetB (src [4], src [5]);
                            src += 2 * 3;
                            dst++;
                        }
                    }
                    break;
                case 1:
                    // Colour correction: vignetting 
                    ok = mod->ApplyColorModification (imgdata, 0.0, y, img->width, 1,
                        LF_CR_4 (RED, GREEN, BLUE, UNKNOWN), 0);
                    imgdata += img->width * 4;
                    break;
#ifndef COMBINE_13
                case 2:
                    // Distortion and geometry correction, scaling 
                    ok = mod->ApplyGeometryDistortion (0.0, y, newimg->width, 1, pos);
                    if (ok)
                    {
                        float *src = pos;
                        for (unsigned x = 0; x < img->width; x++)
                        {
                            img->Get (*dst, src [0], src [1]);
                            src += 2;
                            dst++;
                        }
                    }
                    break;
#endif
            }
        // After TCA and distortion steps switch img and newimg.
        // This is crucial since newimg is now the input image
        // to the next stage.
//        if (ok && (step == 0 || step == 2))
//        {
//            Image *tmp = newimg;
//            newimg = img;
//            img = tmp;
//        }
    }
	
*/


	//get integer image array, lensfun it, and use it to create the new gImage dib.
	wxString d = duration();

	if ((myConfig::getConfig().getValueOrDefault("tool.all.log","0") == "1") || (myConfig::getConfig().getValueOrDefault("tool.lenscorrection.log","0") == "1"))
		log(wxString::Format("tool=lenscorrection,imagesize=%dx%d,time=%s",dib->getWidth(), dib->getHeight(),d));
	
	dirty = false;

	((wxFrame*) m_display->GetParent())->SetStatusText("");
	if (processnext) processNext();
	
	return result;
}



