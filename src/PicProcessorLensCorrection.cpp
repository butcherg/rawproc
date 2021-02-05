#include "PicProcessorLensCorrection.h"
#include "PicProcPanel.h"
#include "myListCtrl.h"
#include "myListDialog.h"
#include "myRowSizer.h"
#include "util.h"
#include "gimage/strutil.h"
#include "fileutil.h"
#include "myConfig.h"

#include <wx/listctrl.h>
#include <wx/editlbox.h>

#define LENSCORRECTIONENABLE 6300
#define FILTERID 6301
#define HIDEID 6302

#define LENSID 6303
#define CAMERAID 6304

#define LENSCORRECTIONS		6305
#define LENSCORRECTION_CA	6306
#define LENSCORRECTION_VIG	6307
#define LENSCORRECTION_DIST	6308
#define LENSCORRECTION_AUTOCROP 6309
#define LENSCORRECTION_APPLY	6310

//from darktable: src/iop/lens.cc
//use to conditionally compile against lensfun 0.3.2/0.3.95
#if LF_VERSION == ((0 << 24) | (3 << 16) | (95 << 8) | 0)
#define LF_0395
#endif


class LensCorrectionPanel: public PicProcPanel
{

	public:
		LensCorrectionPanel(wxWindow *parent, PicProcessor *proc, wxString params, wxString metadata): PicProcPanel(parent, proc, params)
		{
			Freeze();
			wxSizerFlags flags = wxSizerFlags().Left().Border(wxLEFT|wxRIGHT|wxTOP);
			wxBoxSizer *b = new wxBoxSizer(wxVERTICAL); 

			wxArrayString parms = split(params, ";");

			enablebox = new wxCheckBox(this, LENSCORRECTIONENABLE, _("lenscorrection:"));
			enablebox->SetValue(true);
			cam = new wxTextCtrl(this, wxID_ANY, "", wxDefaultPosition, wxSize(200,TEXTCTRLHEIGHT),wxTE_PROCESS_ENTER);
			lens = new wxTextCtrl(this, wxID_ANY, "", wxDefaultPosition, wxSize(200,TEXTCTRLHEIGHT),wxTE_PROCESS_ENTER);
			flags = wxSizerFlags().Left().Border(wxLEFT|wxRIGHT);

			ca = new wxCheckBox(this, LENSCORRECTION_CA, _("chromatic abberation"));
			vig = new wxCheckBox(this, LENSCORRECTION_VIG, _("vignetting"));
			dist = new wxCheckBox(this, LENSCORRECTION_DIST, _("distortion"));
			crop = new wxCheckBox(this, LENSCORRECTION_AUTOCROP, _("autocrop"));
			
			wxArrayString str;
			str.Add("nearest");
			str.Add("bilinear");
			str.Add("lanczos3");
			algo = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, str);
			algo->SetStringSelection("nearest");
			
			str.Empty();
			str.Add("reticlinear");
			str.Add("fisheye");
			str.Add("panoramic");
			str.Add("equirectangular");
			str.Add("orthographic");
			str.Add("stereographic");
			str.Add("equisolid");
			str.Add("thoby");
			//disable - fully implement geometry (see further comment-outs, below)
			//geom = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, str);
			//geom->SetStringSelection("reticlinear");
			

			for (int i=0; i<parms.GetCount(); i++) {
				wxArrayString nameval = split(parms[i], "=");
				if (nameval[0] == "camera") {
					cam->SetValue(wxString(de_underscore(std::string(nameval[1].c_str())).c_str()));
				}
				if (nameval[0] == "lens") {
					lens->SetValue(wxString(de_underscore(std::string(nameval[1].c_str())).c_str()));
				}
				if (nameval[0] == "ops") {
					wxArrayString ops = split(nameval[1],",");
					for (int j=0; j<ops.GetCount(); j++) {
						if (ops[j] == "ca") ca->SetValue(true);
						if (ops[j] == "vig") vig->SetValue(true);
						if (ops[j] == "dist") dist->SetValue(true);
						if (ops[j] == "autocrop") crop->SetValue(true);
					}
				}
				if (nameval[0] == "algo") {
					algo->SetStringSelection(nameval[1]);
				}
				//if (nameval[0] == "geometry") {
				//	geom->SetStringSelection(nameval[1]);
				//}
			}

			wxString altcam = cam->GetValue();
			wxString altlens = lens->GetValue();
			((PicProcessorLensCorrection *) q)->setAlternates(altcam, altlens);

			myRowSizer *m = new myRowSizer(wxSizerFlags().Expand());
			m->AddRowItem(enablebox, wxSizerFlags(1).Left().Border(wxLEFT|wxTOP));

			m->NextRow(wxSizerFlags().Expand());
			m->AddRowItem(new wxStaticLine(this, wxID_ANY), wxSizerFlags(1).Left().Border(wxLEFT|wxRIGHT|wxTOP|wxBOTTOM));

			//m->NextRow();
			//m->AddRowItem(new wxStaticText(this,-1, " "), flags);
			
			m->NextRow();
			m->AddRowItem(new wxStaticText(this,-1, metadata, wxDefaultPosition, wxSize(260,50)), flags);
			m->NextRow();
			m->AddRowItem(new wxButton(this, CAMERAID, _("Camera:"), wxDefaultPosition, wxSize(70,-1)), flags);
			m->AddRowItem(cam, flags);
			m->NextRow();
			m->AddRowItem(new wxButton(this, LENSID, _("Lens:"), wxDefaultPosition, wxSize(70,-1)), flags);
			m->AddRowItem(lens, flags);
			m->NextRow();
			m->AddRowItem(new wxStaticText(this,-1, " "), flags);

			m->NextRow(wxSizerFlags().Expand());
			m->AddRowItem(new wxStaticLine(this, wxID_ANY), wxSizerFlags(1).Left().Border(wxLEFT|wxRIGHT|wxTOP|wxBOTTOM));

			//m->NextRow();
			//m->AddRowItem(new wxStaticText(this,-1, " "), flags);
			
			m->NextRow(wxSizerFlags().Expand());
			m->AddRowItem(ca, flags);
			m->NextRow();
			m->AddRowItem(vig, flags);
			m->NextRow();
			m->AddRowItem(dist, flags);
			m->NextRow();
			m->AddRowItem(crop, flags);
			
			m->NextRow();
			m->AddRowItem(new wxStaticText(this,-1, " "), flags);
			
			m->NextRow(wxSizerFlags().Expand());
			m->AddRowItem(new wxStaticText(this,-1, "algorithm:"), wxSizerFlags(0).Right().CenterVertical());
			m->AddRowItem(algo, wxSizerFlags(0).Right().CenterVertical().Border(wxRIGHT|wxTOP));
			//m->AddRowItem(new wxStaticText(this,-1, "geometry:"), wxSizerFlags(0).Right().CenterVertical());
			//m->AddRowItem(geom, wxSizerFlags(0).Right().CenterVertical().Border(wxRIGHT|wxTOP));
			
			
			m->NextRow();
			m->AddRowItem(new wxStaticText(this,-1, " "), flags);
			m->NextRow();
			m->AddRowItem(new wxButton(this, LENSCORRECTION_APPLY, _("Apply"), wxDefaultPosition, wxSize(70,-1)), flags);
			m->End();

			SetSizerAndFit(m);
			SetFocus();
			Bind(wxEVT_TEXT,&LensCorrectionPanel::setAlternates, this);
			Bind(wxEVT_BUTTON, &LensCorrectionPanel::lensDialog, this);
			Bind(wxEVT_RADIOBOX,&LensCorrectionPanel::paramChanged, this);
			//Bind(wxEVT_CHECKBOX, &LensCorrectionPanel::paramChanged, this, LENSCORRECTION_CA, LENSCORRECTION_AUTOCROP);
			Bind(wxEVT_BUTTON, &LensCorrectionPanel::paramChanged, this, LENSCORRECTION_APPLY);
			Bind(wxEVT_CHECKBOX, &LensCorrectionPanel::onEnable, this, LENSCORRECTIONENABLE);
			Bind(wxEVT_CHAR_HOOK, &LensCorrectionPanel::OnKey,  this);
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

		void lensDialog(wxCommandEvent& event)
		{
			
			wxArrayString items;
			wxString dlgtitle;

			
			switch (event.GetId()) {
				case LENSID: {
					const struct lfLens *const *lenses;
					lenses = lf_db_get_lenses (((PicProcessorLensCorrection *)q)->getLensDatabase());
					for (int i = 0; lenses [i]; i++)
					{
						items.Add(wxString((char *) lenses[i]->Model));
					}
					dlgtitle = "Lenses";
				
					myListDialog dlg(this, wxID_ANY, _("Lens"), items, wxDefaultPosition, wxDefaultSize);
					if ( dlg.ShowModal() == wxID_OK )
						lens->SetValue(dlg.getSelection());
					break;
				}
				case CAMERAID: {
					const struct lfCamera *const *cameras;
					cameras = lf_db_get_cameras (((PicProcessorLensCorrection *)q)->getLensDatabase());
					for (int i = 0; cameras [i]; i++)
					{
						items.Add(wxString((char *) cameras[i]->Model));
					}
					dlgtitle = "Cameras";
					myListDialog dlg(this, wxID_ANY, _("Camera"), items, wxDefaultPosition, wxDefaultSize);
					if ( dlg.ShowModal() == wxID_OK )
						cam->SetValue(dlg.getSelection());
					break;
				}
			}
			((PicProcessorLensCorrection *) q)->setAlternates(cam->GetValue(), lens->GetValue());
		}

		
		void setAlternates(wxCommandEvent& event)
		{
			wxString altcam = cam->GetValue();
			wxString altlens = lens->GetValue();
			((PicProcessorLensCorrection *) q)->setAlternates(altcam, altlens);
		}

		void paramChanged(wxCommandEvent& event)
		{
			wxString cmd;
			wxString altcam = cam->GetValue();
			if (altcam != "") paramAppend("camera", wxString(underscore(std::string(altcam.c_str())).c_str()), cmd);
			wxString altlens = lens->GetValue();
			if (altlens != "") paramAppend("lens",wxString(underscore(std::string(altlens.c_str())).c_str()), cmd);

			wxString ops;
			if (ca->GetValue()) opAppend("ca",ops);
			if (vig->GetValue()) opAppend("vig",ops);
			if (dist->GetValue()) opAppend("dist",ops);
			if (crop->GetValue()) opAppend("autocrop",ops);

			if (ops != "") paramAppend("ops", ops, cmd);
			if (dist->GetValue()) paramAppend("algo", algo->GetString(algo->GetSelection()), cmd);
			
			//wxString geometry = geom->GetString(geom->GetSelection()); 
			//if (geometry != "reticlinear") paramAppend("geometry", geometry, cmd);

			q->setParams(cmd);
			q->processPic();
			event.Skip();
		}


	private:
		wxChoice *algo, *geom;
		wxCheckBox *ca, *vig, *dist, *crop;
		wxTextCtrl *cam, *lens;
		wxCheckBox *enablebox;

};

lfDatabase * PicProcessorLensCorrection::findLensfunDatabase()
{
	bool lfok = false;
	lfError e = LF_NO_DATABASE;
	lfDatabase * lfdb = new lfDatabase();

	//parm tool.lenscorrection.databasepath: If specified, use this path to update and retrive for use the lensfun database.  The path specified should be the one that contains the 'version_x' folder that contains the lensfun XML files.  If none is specified, the same directory as the configuration file location is used. (rawproc does not allow lensfun to use the system data directories.)
	std::string lensfundatadir = myConfig::getConfig().getValueOrDefault("tool.lenscorrection.databasepath",getAppConfigDir());
	
	lensfundatadir.append(string_format("/version_%d", LF_MAX_DATABASE_VERSION));

	if (lensfundatadir != "") {
#ifdef LF_0395
		e = lfdb->Load(lensfundatadir.c_str());
		if (e == LF_NO_DATABASE) wxMessageBox(wxString::Format(_("Error: Cannot open lens correction database at %s"),wxString(lensfundatadir)));
		if (e == LF_WRONG_FORMAT) wxMessageBox(wxString::Format(_("Error: Lens correction database at %s format is incorrect"),wxString(lensfundatadir)));
#else
		if (lfdb->LoadDirectory(lensfundatadir.c_str())) 
			e = LF_NO_ERROR;
		else 
			wxMessageBox(wxString::Format(_("Error: Cannot open lens correction database at %s"),wxString(lensfundatadir)));
#endif
	}
	else {
		e = lfdb->Load();
		if (e == LF_NO_DATABASE) wxMessageBox(wxString::Format(_("Error: Cannot open lens correction database at a system location")));
		if (e == LF_WRONG_FORMAT) wxMessageBox(wxString::Format(_("Error: Lens correction database (system location) format is incorrect")));
	}
		
	if (e == LF_NO_ERROR) return lfdb;
	wxMessageBox(_("Error: lens correction database read failed"));
	delete lfdb;
	return NULL;
}

PicProcessorLensCorrection::PicProcessorLensCorrection(lfDatabase * lfdatabase, wxString name, wxString command, wxTreeCtrl *tree, PicPanel *display): PicProcessor(name, command, tree, display) 
{
	setlocale (LC_ALL, "");		

	altcamera.clear();
	altlens.clear();
	
	ldb = lfdatabase;

	gImage &idib = getPreviousPicProcessor()->getProcessedPic();
	std::map<std::string,std::string> info = idib.getInfo();

	if (info.find("Model") != info.end())
		metadatacamera = wxString(info["Model"].c_str());
	else
		metadatacamera = "(none)";

	if (info.find("Lens") != info.end())
		metadatalens = wxString(info["Lens"].c_str());
	else if (info.find("LensModel") != info.end())
		metadatalens = wxString(info["LensModel"].c_str());
	else
		metadatalens = "(none)";

}


PicProcessorLensCorrection::~PicProcessorLensCorrection()
{
	//if (ldb) 
		delete ldb;
}

void PicProcessorLensCorrection::createPanel(wxSimplebook* parent)
{
	gImage &idib = getPreviousPicProcessor()->getProcessedPic();
	wxString metadata = wxString::Format(_("Camera: %s\nLens: %s"),metadatacamera, metadatalens);
	toolpanel = new LensCorrectionPanel(parent, this, c, metadata);
	parent->ShowNewPage(toolpanel);
	toolpanel->Refresh();
	toolpanel->Update();
}

void PicProcessorLensCorrection::setAlternates(wxString acam, wxString alens)
{
	altcamera = acam;
	altlens = alens;
}

lfDatabase * PicProcessorLensCorrection::getLensDatabase()
{
	return ldb;
}

bool PicProcessorLensCorrection::processPicture(gImage *processdib) 
{
	((wxFrame*) m_display->GetParent())->SetStatusText(_("lenscorrection..."));
	bool result = true;
	GIMAGE_ERROR ret;

	std::map<std::string, std::string> cp = parseparams(std::string(getParams().c_str()));
	
	std::string camspec, lensspec;
	
	if (altcamera != "")
		camspec = de_underscore(std::string(altcamera));
	else if (metadatacamera != "(none)")
		camspec = metadatacamera;
	else
		camspec = "(none)";
	
	if (altlens != "")
		lensspec = de_underscore(std::string(altlens));
	else if (metadatalens != "(none)")
		lensspec = metadatalens;
	else
		lensspec = "(none)";
	
	int ModifyFlags = 0;  //ops=ca,vig,dist 
	if (cp.find("ops") != cp.end()) {
		std::vector<std::string> ops = split(cp["ops"], ",");
		for (unsigned i=0; i<ops.size(); i++) {
			if (ops[i] == "ca")   ModifyFlags |= LF_MODIFY_TCA;
			if (ops[i] == "vig")  ModifyFlags |= LF_MODIFY_VIGNETTING;
			if (ops[i] == "dist") ModifyFlags |= LF_MODIFY_DISTORTION;
			if (ops[i] == "autocrop") ModifyFlags |= LF_MODIFY_SCALE;
		}
	}
	
	int threadcount =  atoi(myConfig::getConfig().getValueOrDefault("tool.lenscorrection.cores","0").c_str());
	if (threadcount == 0) 
		threadcount = gImage::ThreadCount();
	else if (threadcount < 0) 
		threadcount = std::max(gImage::ThreadCount() + threadcount,0);

	dib = processdib;
	if (!global_processing_enabled) return true;

	std::map<std::string, std::string> info = dib->getInfo();

	if (processingenabled) {
		if (ModifyFlags) {
			mark();
			bool success = false;

			const lfCamera *cam = NULL;
			const lfCamera ** cameras = ldb->FindCamerasExt(NULL, camspec.c_str());
			if (cameras) {
				cam = cameras[0];
				success = true;
			}
			else {
				wxMessageBox(wxString::Format(_("Cannot find a camera matching %s in database\n"), camspec.c_str()));
				success = false;
			}
			lf_free (cameras);

			const lfLens *lens = NULL;
			const lfLens **lenses = NULL;
			if (success) {
				// try to find a matching lens in the database
				lenses = ldb->FindLenses (cam, NULL, lensspec.c_str());
				if (lenses) {
					lens = lenses [0];
					success = true;
				}
				else {
					wxMessageBox(wxString::Format(_("Cannot find a lens matching %s in database\n"), lensspec.c_str()));
					success = false;
				}
				lf_free (lenses);
			}


			if (success) {
				if (cp.find("algo") != cp.end()) {
					if (cp["algo"] == "nearest") dib->initInterpolation(FILTER_BOX);
					if (cp["algo"] == "bilinear") dib->initInterpolation(FILTER_BILINEAR);
					if (cp["algo"] == "lanczos3") dib->initInterpolation(FILTER_LANCZOS3);
				}
				
#ifdef LF_0395
/*"old" 0.3.95:
				lfModifier *mod = new lfModifier (cam->CropFactor, dib->getWidth(), dib->getHeight(), LF_PF_F32, false);

				// Enable desired modifications
				int modflags = 0;

				if (ModifyFlags & LF_MODIFY_TCA)
					modflags |= mod->EnableTCACorrection(lens, atof(info["FocalLength"].c_str()));
				if (ModifyFlags & LF_MODIFY_VIGNETTING)
					modflags |= mod->EnableVignettingCorrection(lens, atof(info["FocalLength"].c_str()), atof(info["FNumber"].c_str()), 1000.0f);
				if (ModifyFlags & LF_MODIFY_DISTORTION)
					modflags |= mod->EnableDistortionCorrection(lens, atof(info["FocalLength"].c_str()));
				if (ModifyFlags & LF_MODIFY_GEOMETRY)
					modflags |= mod->EnableProjectionTransform(lens, atof(info["FocalLength"].c_str()), LF_RECTILINEAR);
				if (ModifyFlags & LF_MODIFY_SCALE)
					modflags |= mod->EnableScaling(1.0);
*/
//"new" 0.3.95, master:
				lfModifier *mod = new lfModifier (lens, atof(info["FocalLength"].c_str()), cam->CropFactor, dib->getWidth(), dib->getHeight(), LF_PF_F32, false);

				// Enable desired modifications
				int modflags = 0;

				if (ModifyFlags & LF_MODIFY_TCA)
					modflags |= mod->EnableTCACorrection();
				if (ModifyFlags & LF_MODIFY_VIGNETTING)
					modflags |= mod->EnableVignettingCorrection(atof(info["FNumber"].c_str()), 1000.0f);
				if (ModifyFlags & LF_MODIFY_DISTORTION)
					modflags |= mod->EnableDistortionCorrection();
				if (ModifyFlags & LF_MODIFY_GEOMETRY)
					modflags |= mod->EnableProjectionTransform(LF_RECTILINEAR);
				if (ModifyFlags & LF_MODIFY_SCALE)
					modflags |= mod->EnableScaling(mod->GetAutoScale(false));

#else
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

				if (ModifyFlags & LF_MODIFY_SCALE) mod->AddCoordCallbackScale(0.0);
#endif

				unsigned w = dib->getWidth();
				unsigned h = dib->getHeight();

				if (ModifyFlags & LF_MODIFY_VIGNETTING) {  //#1
					((wxFrame*) m_display->GetParent())->SetStatusText(_("lenscorrection: vignetting..."));
					pix * newimg = dib->getImageDataRaw();
					bool ok = true;

					#pragma omp parallel for num_threads(threadcount)
					for (unsigned y = 0; y < h; y++) {
						unsigned p = y*w;
						ok = mod->ApplyColorModification (&newimg[p], 0.0, y, w, 1, LF_CR_3 (RED, GREEN, BLUE), w);
					}
				}

				if ((ModifyFlags & LF_MODIFY_DISTORTION) & (ModifyFlags & LF_MODIFY_TCA)) { //both #2 and #3
					((wxFrame*) m_display->GetParent())->SetStatusText(_("lenscorrection: chromatic abberation and distortion..."));
					gImage olddib(*dib);
					pix * newimg = dib->getImageDataRaw();
					bool ok = true;
					int lwidth = w * 2 * 3;
			
					#pragma omp parallel for num_threads(threadcount)
					for (unsigned y = 0; y < h; y++) {
						float pos[lwidth];
						ok = mod->ApplySubpixelGeometryDistortion (0.0, y, w, 1, pos);
						if (ok) {
							unsigned s=0;
							for (unsigned x = 0; x < w; x++) {
								unsigned p = x + y*w;
								newimg[p].r = olddib.getR (pos [s], pos [s+1]);
								newimg[p].g = olddib.getG (pos [s+2], pos [s+3]);
								newimg[p].b = olddib.getB (pos [s+4], pos [s+5]);
								s += 2 * 3;
							}
						}
					}
	
				}

				else {  //#2, or #3
	
					if (ModifyFlags & LF_MODIFY_DISTORTION) {  //#2
						((wxFrame*) m_display->GetParent())->SetStatusText(_("lenscorrection: distortion..."));
						gImage olddib(*dib);
						pix * newimg = dib->getImageDataRaw();
						bool ok = true;
						int lwidth = w * 2;
				
						#pragma omp parallel for num_threads(threadcount)
						for (unsigned y = 0; y < h; y++) {
							float pos[lwidth];
							ok = mod->ApplyGeometryDistortion (0.0, y, w, 1, pos);
							if (ok) {
								unsigned s=0;
								for (unsigned x = 0; x < w; x++) {
									unsigned p = x + y*w;
									newimg[p] = olddib.getRGB (pos [s], pos [s+1]);
									s += 2;
								}
							}
						}
					}
		
					if (ModifyFlags & LF_MODIFY_TCA) {  //#3
						((wxFrame*) m_display->GetParent())->SetStatusText(_("lenscorrection: chromatic abberation..."));
						gImage olddib(*dib);
						pix * newimg = dib->getImageDataRaw();
						bool ok = true;
						int lwidth = w * 2 * 3;
				
						#pragma omp parallel for num_threads(threadcount)
						for (unsigned y = 0; y < h; y++) {
							float pos[lwidth];
							ok = mod->ApplySubpixelDistortion (0.0, y, w, 1, pos);
							if (ok) {
								unsigned s=0;
								for (unsigned x = 0; x < w; x++) {
									unsigned p = x + y*w;
									newimg[p].r = olddib.getR (pos [s], pos [s+1]);
									newimg[p].g = olddib.getG (pos [s+2], pos [s+3]);
									newimg[p].b = olddib.getB (pos [s+4], pos [s+5]);
									s += 2 * 3;
								}
							}
						}
					}
				}
#ifdef LF_0395
				delete mod;
#else
				mod->Destroy();
#endif
			}
			
			m_display->SetModified(true);
			wxString d = duration();

			if ((myConfig::getConfig().getValueOrDefault("tool.all.log","0") == "1") || (myConfig::getConfig().getValueOrDefault("tool.lenscorrection.log","0") == "1"))
				log(wxString::Format(_("tool=lenscorrection,%s,imagesize=%dx%d,time=%s"),getParams(), dib->getWidth(), dib->getHeight(),d));
	
		}
	}

	dirty = false;

	((wxFrame*) m_display->GetParent())->SetStatusText("");
	
	return result;
}



