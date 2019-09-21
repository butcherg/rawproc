#include "PicProcessorLensCorrection.h"
#include "PicProcPanel.h"
#include "myRowSizer.h"
#include "util.h"
#include "gimage/strutil.h"
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

class myListCtrl: public wxListCtrl
{
	public:
		//Constructs the list control, populated with the items passed in the listitems wxArrayString:
		myListCtrl(wxWindow *parent, wxWindowID id, wxString listname, wxArrayString listitems, const wxPoint &pos=wxDefaultPosition, const wxSize &size=wxDefaultSize):
			wxListCtrl(parent, id, pos, size, wxLC_REPORT | wxLC_NO_HEADER | wxLC_HRULES, wxDefaultValidator, listname)
		{
			//SetDoubleBuffered(true);
			name = listname;
			width = size.x;
			wxListItem col0;
			col0.SetId(0);
			col0.SetText(name );
			col0.SetWidth(width);
			InsertColumn(0, col0);

			itemlist = listitems;

			for (int i=0; i<itemlist.GetCount(); i++) {
				wxListItem item;
				item.SetId(i);
				item.SetText( itemlist[i] );
				InsertItem( item );
			}

			filter.clear();
			selected.clear();

			Bind(wxEVT_LIST_ITEM_SELECTED, &myListCtrl::Selected, this);
		}

		//Filters the list to include only entries that contain the specified string:
		void setFilter(wxString f)
		{
			wxArrayString filteredlist;
			filter = f;
			//DeleteAllItems();
			
			ClearAll();
			wxListItem col0;
			col0.SetId(0);
			col0.SetText(name);
			col0.SetWidth(width);
			InsertColumn(0, col0);
			
			
			for (int i=0; i<itemlist.GetCount(); i++) {
				//if (itemlist[i].Lower().Find(filter.Lower()) != wxNOT_FOUND) {
				if (itemlist[i].Find(filter) != wxNOT_FOUND) {
					//wxListItem item;
					//item.SetId(j);
					//item.SetText( itemlist[i] );
					//InsertItem( item );
					//j++;
					filteredlist.Add(itemlist[i]);
				}
			}
			for (int i=0; i<filteredlist.GetCount(); i++) {
				wxListItem item;
                                item.SetId(i);
                                item.SetText( filteredlist[i] );
                                InsertItem( item );
			}
			Refresh();
		}
		

		//Captures the entry selected, at selection:
		void Selected(wxListEvent& event)
		{
			selected = event.GetText();
			event.Skip();
		}


		//Returns the selected entry, as populated by the wxListEvent method:
		wxString GetSelected()
		{
			return selected;
		}


	private:
		int width;
		wxArrayString itemlist;
		wxString filter, selected, name;
};
 

class myLensDialog: public wxDialog
{
	public:
		myLensDialog(wxWindow *parent, wxWindowID id, const wxString &title, struct lfDatabase * ldb, const wxPoint &pos, const wxSize &size):
			wxDialog(parent, id, title, pos, size)
		{
			wxBoxSizer *sz = new wxBoxSizer(wxVERTICAL);
			wxBoxSizer *ct = new wxBoxSizer(wxHORIZONTAL);

			wxArrayString items;
			wxString dlgtitle;
			if (title == "Camera") {
				const struct lfCamera *const *cameras;
				cameras = lf_db_get_cameras (ldb);
				for (int i = 0; cameras [i]; i++)
				{
					items.Add(wxString((char *) cameras[i]->Model));
				}
				dlgtitle = "Cameras";
			}
			else if (title == "Lens") {
				const struct lfLens *const *lenses;
				lenses = lf_db_get_lenses (ldb);
				for (int i = 0; lenses [i]; i++)
				{
					items.Add(wxString((char *) lenses[i]->Model));
				}
				dlgtitle = "Lenses";
			}

			list = new myListCtrl(this, wxID_ANY, dlgtitle, items, wxDefaultPosition, wxSize(400,300));
			sz->Add(list, 0, wxEXPAND | wxALL, 3);
			
			ct->Add(new wxButton(this, wxID_OK, "Ok"), 0, wxALL, 10);
			ct->Add(new wxButton(this, wxID_CANCEL, "Cancel"), 0, wxALL, 10);
			ct->Add(new wxStaticText(this, wxID_ANY, "Filter: "), 0, wxALL, 10);
			fil = new wxTextCtrl(this, FILTERID, "", wxDefaultPosition, wxSize(100,TEXTCTRLHEIGHT),wxTE_PROCESS_ENTER);
			ct->Add(fil, 0, wxALL, 10);
			count = new wxStaticText(this, wxID_ANY, wxString::Format("%d items",list->GetItemCount()));
			ct->Add(count, 0, wxALL, 10);
			sz->Add(ct, 0, wxALL, 10);
			SetSizerAndFit(sz);

			fil->SetFocus();
			
			Bind(wxEVT_TEXT_ENTER, &myLensDialog::FilterGrid, this);
			Bind(wxEVT_TEXT, &myLensDialog::FilterGrid, this);
			Bind(wxEVT_BUTTON, &myLensDialog::EndDialog, this);
			list->Bind(wxEVT_LEFT_DCLICK, &myLensDialog::doubleClicked, this);
			
		}
		
		void EndDialog(wxCommandEvent& event)
		{
			if (event.GetId() == wxID_OK)
				EndModal(wxID_OK);
			else
				EndModal(wxID_CANCEL);
		}

		void doubleClicked(wxMouseEvent& event)
		{
			EndModal(wxID_OK);
			event.Skip();
		}
		
		void FilterGrid(wxCommandEvent& event)
		{
			list->setFilter(fil->GetValue());
			count->SetLabelText(wxString::Format("%d items",list->GetItemCount()));
		}
		
		
		wxString GetSelection()
		{
			return list->GetSelected();
		}
		
	private:
		myListCtrl *list;
		wxTextCtrl *fil;
		wxStaticText *count;
		wxString lens;
	
	
	
};

class LensCorrectionPanel: public PicProcPanel
{

	public:
		LensCorrectionPanel(wxWindow *parent, PicProcessor *proc, wxString params, wxString metadata): PicProcPanel(parent, proc, params)
		{
			wxSizerFlags flags = wxSizerFlags().Left().Border(wxLEFT|wxRIGHT|wxTOP);
			wxBoxSizer *b = new wxBoxSizer(wxVERTICAL); 

			wxArrayString parms = split(params, ";");

			enablebox = new wxCheckBox(this, LENSCORRECTIONENABLE, "lenscorrection:");
			enablebox->SetValue(true);
			cam = new wxTextCtrl(this, wxID_ANY, "", wxDefaultPosition, wxSize(200,TEXTCTRLHEIGHT),wxTE_PROCESS_ENTER);
			lens = new wxTextCtrl(this, wxID_ANY, "", wxDefaultPosition, wxSize(200,TEXTCTRLHEIGHT),wxTE_PROCESS_ENTER);
			flags = wxSizerFlags().Left().Border(wxLEFT|wxRIGHT);

			ca = new wxCheckBox(this, LENSCORRECTION_CA, "chromatic abberation");
			vig = new wxCheckBox(this, LENSCORRECTION_VIG, "vignetting");
			dist = new wxCheckBox(this, LENSCORRECTION_DIST, "distortion");
			crop = new wxCheckBox(this, LENSCORRECTION_AUTOCROP, "autocrop");

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
			}

			wxString altcam = cam->GetValue();
			wxString altlens = lens->GetValue();
			((PicProcessorLensCorrection *) q)->setAlternates(altcam, altlens);

			myRowSizer *m = new myRowSizer(wxSizerFlags().Expand());
			m->AddRowItem(enablebox, wxSizerFlags(1).Left().Border(wxLEFT|wxTOP));

			m->NextRow(wxSizerFlags().Expand());
			m->AddRowItem(new wxStaticLine(this, wxID_ANY), wxSizerFlags(1).Left().Border(wxLEFT|wxRIGHT|wxTOP|wxBOTTOM));

			m->NextRow();
			m->AddRowItem(new wxStaticText(this,-1, " "), flags);
			m->NextRow();
			m->AddRowItem(new wxStaticText(this,-1, metadata, wxDefaultPosition, wxSize(260,50)), flags);
			m->NextRow();
			m->AddRowItem(new wxButton(this, CAMERAID, "Camera:", wxDefaultPosition, wxSize(70,-1)), flags);
			m->AddRowItem(cam, flags);
			m->NextRow();
			m->AddRowItem(new wxButton(this, LENSID, "Lens:", wxDefaultPosition, wxSize(70,-1)), flags);
			m->AddRowItem(lens, flags);
			m->NextRow();
			m->AddRowItem(new wxStaticText(this,-1, " "), flags);

			m->NextRow(wxSizerFlags().Expand());
			m->AddRowItem(new wxStaticLine(this, wxID_ANY), wxSizerFlags(1).Left().Border(wxLEFT|wxRIGHT|wxTOP|wxBOTTOM));

			m->NextRow();
			m->AddRowItem(new wxStaticText(this,-1, " "), flags);
			m->NextRow();
			m->AddRowItem(ca, flags);
			m->NextRow();
			m->AddRowItem(vig, flags);
			m->NextRow();
			m->AddRowItem(dist, flags);
			m->NextRow();
			m->AddRowItem(crop, flags);
			m->NextRow();
			m->AddRowItem(new wxStaticText(this,-1, " "), flags);
			m->NextRow();
			m->AddRowItem(new wxButton(this, LENSCORRECTION_APPLY, "Apply", wxDefaultPosition, wxSize(70,-1)), flags);
			m->End();
			SetSizerAndFit(m);
			m->Layout();

			Refresh();
			Update();
			SetFocus();
			Bind(wxEVT_TEXT,&LensCorrectionPanel::setAlternates, this);
			Bind(wxEVT_BUTTON, &LensCorrectionPanel::lensDialog, this);
			Bind(wxEVT_RADIOBOX,&LensCorrectionPanel::paramChanged, this);
			//Bind(wxEVT_CHECKBOX, &LensCorrectionPanel::paramChanged, this, LENSCORRECTION_CA, LENSCORRECTION_AUTOCROP);
			Bind(wxEVT_BUTTON, &LensCorrectionPanel::paramChanged, this, LENSCORRECTION_APPLY);
			Bind(wxEVT_CHECKBOX, &LensCorrectionPanel::onEnable, this, LENSCORRECTIONENABLE);

		}

		~LensCorrectionPanel()
		{
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
			switch (event.GetId()) {
				case LENSID: {
					myLensDialog dlg(this, wxID_ANY, "Lens", ((PicProcessorLensCorrection *)q)->getLensDatabase(), wxDefaultPosition, wxDefaultSize);
					if ( dlg.ShowModal() == wxID_OK )
						//wxMessageBox(dlg.GetSelection());
						lens->SetValue(dlg.GetSelection());
					break;
				}
				case CAMERAID: {
					myLensDialog dlg(this, wxID_ANY, "Camera", ((PicProcessorLensCorrection *)q)->getLensDatabase(), wxDefaultPosition, wxDefaultSize);
					if ( dlg.ShowModal() == wxID_OK )
						//wxMessageBox(dlg.GetSelection());
						cam->SetValue(dlg.GetSelection());
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

			q->setParams(cmd);
			q->processPic();
			event.Skip();
		}


	private:
		wxCheckBox *ca, *vig, *dist, *crop;
		wxTextCtrl *cam, *lens;
		wxCheckBox *enablebox;

};

PicProcessorLensCorrection::PicProcessorLensCorrection(wxString name, wxString command, wxTreeCtrl *tree, PicPanel *display): PicProcessor(name, command, tree, display) 
{
	lfok = false;
	setlocale (LC_ALL, "");		

	altcamera.clear();
	altlens.clear();
	
	lfError e;
	//ldb = lf_db_new ();
	//if (lf_db_load (ldb) != LF_NO_ERROR) {

	ldb = lfDatabase::Create();
	//parm tool.lenscorrection.databasepath: If specified, use this path instead of the standard lensfun directory.
	std::string lensfundatadir = myConfig::getConfig().getValueOrDefault("tool.lenscorrection.databasepath","");
	if (lensfundatadir != "") {
		if (ldb->LoadDirectory(lensfundatadir.c_str())) {
			e = LF_NO_ERROR;
			lfok = true;
		}
		else {
			wxMessageBox(wxString::Format("Error: Cannot open lens correction database at %s, trying standard directories...",wxString(lensfundatadir)));
			lfok = false;
		}
	}
	if (!lfok) {
		if (ldb->Load() != LF_NO_ERROR) {
			wxMessageBox("Error: Cannot open lens correction database at any standard directory") ;
			lfok = false;
		}
		else lfok = true;
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

lfDatabase * PicProcessorLensCorrection::getLensDatabase()
{
	return ldb;
}

bool PicProcessorLensCorrection::processPic(bool processnext) 
{
	((wxFrame*) m_display->GetParent())->SetStatusText("lenscorrection...");
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

	if (dib) delete dib;
	dib = new gImage(getPreviousPicProcessor()->getProcessedPic());
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
				wxMessageBox(wxString::Format("Cannot find a camera matching %s in database\n", camspec.c_str()));
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
					wxMessageBox(wxString::Format("Cannot find a lens matching %s in database\n", lensspec.c_str()));
					success = false;
				}
				lf_free (lenses);
			}

			if (success) {
				lfModifier *mod = lfModifier::Create (lens, lens->CropFactor, dib->getWidth(), dib->getHeight());
				int modflags = mod->Initialize (
	        			lens, 
					LF_PF_F32, 
					atof(info["FocalLength"].c_str()), 
					atof(info["FNumber"].c_str()),
	        			1.0f, //opts.Distance
					0.0f, //opts.Scale, 
					LF_RECTILINEAR, //opts.TargetGeom,
					ModifyFlags, 
					false //opts.Inverse
				);

				if (ModifyFlags & LF_MODIFY_SCALE) mod->AddCoordCallbackScale(0.0);

				unsigned w = dib->getWidth();
				unsigned h = dib->getHeight();

				if (ModifyFlags & LF_MODIFY_VIGNETTING) {  //#1
					((wxFrame*) m_display->GetParent())->SetStatusText("lenscorrection: vignetting...");
					pix * newimg = dib->getImageDataRaw();
					bool ok = true;

					#pragma omp parallel for num_threads(threadcount)
					for (unsigned y = 0; y < h; y++) {
						unsigned p = y*w;
						ok = mod->ApplyColorModification (&newimg[p], 0.0, y, w, 1, LF_CR_3 (RED, GREEN, BLUE), w);
					}
				}

				if ((ModifyFlags & LF_MODIFY_DISTORTION) & (ModifyFlags & LF_MODIFY_TCA)) { //both #2 and #3
					((wxFrame*) m_display->GetParent())->SetStatusText("lenscorrection: chromatic abberation and distortion...");
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
						((wxFrame*) m_display->GetParent())->SetStatusText("lenscorrection: distortion...");
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
						((wxFrame*) m_display->GetParent())->SetStatusText("lenscorrection: chromatic abberation...");
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
			}
			
			m_display->SetModified(true);
			wxString d = duration();

			if ((myConfig::getConfig().getValueOrDefault("tool.all.log","0") == "1") || (myConfig::getConfig().getValueOrDefault("tool.lenscorrection.log","0") == "1"))
				log(wxString::Format("tool=lenscorrection,%s,imagesize=%dx%d,time=%s",getParams(), dib->getWidth(), dib->getHeight(),d));
	
		}
	}

	dirty = false;
	if (processnext) processNext();
	((wxFrame*) m_display->GetParent())->SetStatusText("");
	
	return result;
}



