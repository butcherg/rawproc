#include "PicProcessorLensCorrection.h"
#include "PicProcPanel.h"
#include "util.h"
#include "gimage/strutil.h"
#include "myConfig.h"

#include <wx/listctrl.h>
#include <wx/editlbox.h>

#define FILTERID 8500
#define HIDEID 8503

#define LENSID 8501
#define CAMERAID 8502



class myListCtrl: public wxListCtrl
{
	public:
		//Constructs the list control, populated with the items passed in the listitems wxArrayString:
		myListCtrl(wxWindow *parent, wxWindowID id, wxString listname, wxArrayString listitems, const wxPoint &pos=wxDefaultPosition, const wxSize &size=wxDefaultSize):
			wxListCtrl(parent, id, pos, size, wxLC_REPORT | wxLC_NO_HEADER | wxLC_HRULES, wxDefaultValidator, listname)
		{
			SetDoubleBuffered(true);
			wxListItem col0;
			col0.SetId(0);
			col0.SetText( _(listname) );
			col0.SetWidth(size.x);
			InsertColumn(0, col0);

			itemlist = listitems;

			for (int i=0; i<itemlist.GetCount(); i++) {
				wxListItem item;
				item.SetId(i);
				item.SetText( itemlist[i] );
				InsertItem( item );
			}

			filter = "";
			selected = "";

			Bind(wxEVT_LIST_ITEM_SELECTED, &myListCtrl::Selected, this);
		}

		//Filters the list to include only entries that contain the specified string:
		void setFilter(wxString f)
		{
			filter = f;
			DeleteAllItems();
			
			for (int i=0; i<itemlist.GetCount(); i++) {
				if (itemlist[i].Find(filter) != wxNOT_FOUND) {
					wxListItem item;
					item.SetId(i);
					item.SetText( itemlist[i] );
					InsertItem( item );
				}
			}
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
		wxArrayString itemlist;
		wxString filter, selected;
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
			if (title == "Camera") {
				const struct lfCamera *const *cameras;
				cameras = lf_db_get_cameras (ldb);
				for (int i = 0; cameras [i]; i++)
				{
					items.Add(wxString((char *) cameras[i]->Model));
				}
			}
			else if (title == "Lens") {
				const struct lfLens *const *lenses;
				lenses = lf_db_get_lenses (ldb);
				for (int i = 0; lenses [i]; i++)
				{
					items.Add(wxString((char *) lenses[i]->Model));
				}
			}



			//list = new myLensList(this, wxID_ANY, lenses);
			list = new myListCtrl(this, wxID_ANY, "Foo", items, wxDefaultPosition, wxSize(400,300));
			sz->Add(list, 0, wxEXPAND | wxALL, 3);
			
			ct->Add(new wxButton(this, wxID_OK, "Ok"), 0, wxALL, 10);
			ct->Add(new wxStaticText(this, wxID_ANY, "Filter: "), 0, wxALL, 10);
			fil = new wxTextCtrl(this, FILTERID, "", wxDefaultPosition, wxSize(100,25),wxTE_PROCESS_ENTER);
			ct->Add(fil, 0, wxALL, 10);
			
			sz->Add(ct, 0, wxALL, 10);
			SetSizerAndFit(sz);
			
			Bind(wxEVT_TEXT_ENTER, &myLensDialog::FilterGrid, this);
			Bind(wxEVT_TEXT, &myLensDialog::FilterGrid, this);
			Bind(wxEVT_BUTTON, &myLensDialog::EndDialog, this);
			
			
			
		}
		
		~myLensDialog()
		{
			if (list) list->Destroy();
		}
		
		void EndDialog(wxCommandEvent& event)
		{
			EndModal(wxID_OK);
		}
		
		void FilterGrid(wxCommandEvent& event)
		{
			list->setFilter(fil->GetValue());
		}
		
		
		wxString GetSelection()
		{
			//return list->GetLens();
			return list->GetSelected();
		}
		
	private:
		//myLensList *list;
		myListCtrl *list;
		wxTextCtrl *fil;
		wxString lens;
	
	
	
};

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
			b->Add(new wxButton(this, CAMERAID, "Select camera"), flags);
			lens = new wxTextCtrl(this, wxID_ANY, "", wxDefaultPosition, wxSize(200,25),wxTE_PROCESS_ENTER);
			b->Add(lens, flags);
			b->Add(new wxButton(this, LENSID, "Select lens"), flags);
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
			Bind(wxEVT_BUTTON, &LensCorrectionPanel::lensDialog, this);
			Bind(wxEVT_RADIOBOX,&LensCorrectionPanel::paramChanged, this);
			Bind(wxEVT_CHECKBOX, &LensCorrectionPanel::paramChanged, this);
		}

		~LensCorrectionPanel()
		{
		}

		void lensDialog(wxCommandEvent& event)
		{
			switch (event.GetId()) {
				case LENSID: {
					myLensDialog dlg(this, wxID_ANY, "Lens", ((PicProcessorLensCorrection *)q)->getLensDatabase(), wxDefaultPosition, wxDefaultSize);
					if ( dlg.ShowModal() == wxID_OK )
						wxMessageBox(dlg.GetSelection());
					break;
				}
				case CAMERAID: {
					myLensDialog dlg(this, wxID_ANY, "Camera", ((PicProcessorLensCorrection *)q)->getLensDatabase(), wxDefaultPosition, wxDefaultSize);
					if ( dlg.ShowModal() == wxID_OK )
						wxMessageBox(dlg.GetSelection());
					break;
				}
			}
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

lfDatabase * PicProcessorLensCorrection::getLensDatabase()
{
	return ldb;
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
	std::map<std::string, std::string> info = dib->getInfo();
	pix * img = dib->getImageDataRaw();

	bool success = true;
	
	//gImage * olddib = new gImage(getPreviousPicProcessor()->getProcessedPic());
	//pix * img = olddib->getImageDataRaw();

	const lfCamera *cam = NULL;
	const lfCamera ** cameras = ldb->FindCamerasExt(NULL, info["Model"].c_str());
	if (cameras)
		cam = cameras[0];
	else {
		wxMessageBox(wxString::Format("Cannot find a camera matching %s in database\n", info["Model"].c_str()));
		success = false;
	}
	lf_free (cameras);


	const lfLens *lens = NULL;
	const lfLens **lenses = ldb->FindLenses (cam, NULL, info["Lens"].c_str());
	if (success) {
		// try to find a matching lens in the database
		lenses = ldb->FindLenses (cam, NULL, info["Lens"].c_str());
		if (lenses)
			lens = lenses [0];
		else {
			wxMessageBox(wxString::Format("Cannot find a lens matching %s in database\n", info["Lens"].c_str()));
			success = false;
		}
		lf_free (lenses);
	}

	if (success) {
	
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

		if (ModifyFlags & LF_MODIFY_VIGNETTING) {  //#2
			((wxFrame*) m_display->GetParent())->SetStatusText("vignetting...");
			pix * newimg = dib->getImageDataRaw();
			bool ok = true;
			for (unsigned y = 0; ok && y < h; y++) {
				unsigned p = y*w;
				ok = mod->ApplyColorModification (&newimg[p], 0.0, y, w, 1, LF_CR_3 (RED, GREEN, BLUE), w);
			}
		}

		if ((ModifyFlags & LF_MODIFY_DISTORTION) && (ModifyFlags & LF_MODIFY_TCA)) { //both #2 and #3
			((wxFrame*) m_display->GetParent())->SetStatusText("chromatic abberation and distortion...");
			gImage olddib(*dib);
			pix * newimg = dib->getImageDataRaw();
			bool ok = true;
			int lwidth = w * 2 * 3;
			for (unsigned y = 0; ok && y < h; y++) {
				float pos[lwidth];
				ok = mod->ApplySubpixelGeometryDistortion (0.0, y, w, 1, pos);
				if (ok)
				{
					unsigned s=0;
					for (unsigned x = 0; x < w; x++)
					{
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
				((wxFrame*) m_display->GetParent())->SetStatusText("distortion...");
				gImage olddib(*dib);
				pix * newimg = dib->getImageDataRaw();
				bool ok = true;
				int lwidth = w * 2;
				for (unsigned y = 0; ok && y < h; y++) {
					float pos[lwidth];
					ok = mod->ApplyGeometryDistortion (0.0, y, w, 1, pos);
					if (ok)
					{
						unsigned s=0;
						for (unsigned x = 0; x < w; x++)
						{
							unsigned p = x + y*w;
							newimg[p] = olddib.getRGB (pos [s], pos [s+1]);
							s += 2;
						}
					}
				}
			}
		
			if (ModifyFlags & LF_MODIFY_TCA) {  //#3
				((wxFrame*) m_display->GetParent())->SetStatusText("chromatic abberation...");
				gImage olddib(*dib);
				pix * newimg = dib->getImageDataRaw();
				bool ok = true;
				int lwidth = w * 2 * 3;
				for (unsigned y = 0; ok && y < h; y++) {
					float pos[lwidth];
					ok = mod->ApplySubpixelDistortion (0.0, y, w, 1, pos);
					if (ok)
					{
						unsigned s=0;
						for (unsigned x = 0; x < w; x++)
						{
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

		//get integer image array, lensfun it, and use it to create the new gImage dib.
		wxString d = duration();

		if ((myConfig::getConfig().getValueOrDefault("tool.all.log","0") == "1") || (myConfig::getConfig().getValueOrDefault("tool.lenscorrection.log","0") == "1"))
			log(wxString::Format("tool=lenscorrection,%s,imagesize=%dx%d,time=%s",getParams(), dib->getWidth(), dib->getHeight(),d));
	
		dirty = false;

		((wxFrame*) m_display->GetParent())->SetStatusText("");
		if (processnext) processNext();
	}
	
	return result;
}



