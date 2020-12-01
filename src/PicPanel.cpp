
#include "PicPanel.h"
#include "util.h"
#include <vector>
#include <cmath>
#include "gimage/gimage.h"
#include "PicProcessor.h"
#include "myConfig.h"
#include <wx/clipbrd.h>
#include <wx/minifram.h>
#include <wx/frame.h>
#include <wx/display.h>

class mySnapshotWindow: public wxFrame
{
public:
	mySnapshotWindow (wxWindow *parent, wxWindowID id, const wxString &title, wxBitmap snapshot, const wxPoint &pos=wxDefaultPosition, const wxSize &size=wxDefaultSize): wxFrame(parent, id, title, pos, size, wxCLOSE_BOX | wxCAPTION)
	{
		wxSize sz = snapshot.GetSize();
		snap = snapshot;
		Bind(wxEVT_PAINT, &mySnapshotWindow::OnPaint,  this);
		Bind(wxEVT_SIZE, &mySnapshotWindow::OnSize, this);
		SetClientSize(sz.GetWidth(),sz.GetHeight());
		Refresh();
	}

	void OnSize(wxSizeEvent& event) 
	{
		Refresh();
		event.Skip();
	}


	void OnPaint(wxPaintEvent & event)
	{
		wxPaintDC dc(this);
		dc.DrawBitmap(snap, 0,0); 
	}

private:
	wxBitmap snap;
};

class mySnapshotDialog: public wxDialog
{
public:
	mySnapshotDialog (wxWindow *parent, wxWindowID id, const wxString &title, wxBitmap snapshot, const wxPoint &pos=wxDefaultPosition, const wxSize &size=wxDefaultSize): wxDialog(parent, id, title, pos, size, wxCLOSE_BOX | wxCAPTION)
	{
		wxSize sz = snapshot.GetSize();
		snap = snapshot;
		Bind(wxEVT_PAINT, &mySnapshotDialog::OnPaint,  this);
		Bind(wxEVT_SIZE, &mySnapshotDialog::OnSize, this);
		SetClientSize(sz.GetWidth(),sz.GetHeight());
		Refresh();
	}

	void OnSize(wxSizeEvent& event) 
	{
		Refresh();
		event.Skip();
	}


	void OnPaint(wxPaintEvent & event)
	{
		wxPaintDC dc(this);
		dc.DrawBitmap(snap, 0,0); 
	}

private:
	wxBitmap snap;
};


PicPanel::PicPanel(wxFrame *parent, wxTreeCtrl *tree, myHistogramPane *hgram): wxPanel(parent, wxID_ANY, wxDefaultPosition, wxSize(1000,740), wxTAB_TRAVERSAL |wxBORDER_RAISED) 
{

	SetDoubleBuffered(true); 
	display_dib = NULL;
	image = NULL;
	display_dib = NULL;
	thumbnail = NULL;
	scale = 1.0;
	imgctrx = 0.5; imgctry = 0.5;
	imageposx=0; imageposy = 0;
	mousex = 0; mousey=0;
	softproof = thumbdragging = dragging = modified = pixelbox = snapshot = false;

	thumbvisible = true;
	histogram = hgram;
	commandtree = tree;
	skipmove=0;
	oob = 0;
	dcList.clear();

	displayProfile = NULL;
	displayTransform = NULL;

	//parm display.exposuredata: 0|1, enable/disable exposurebox display at startup.  Exposurebox display can still be toggled on/off with the 'e' key. Default=1
	if (myConfig::getConfig().getValueOrDefault("display.exposuredata","1") == "1")
		exposurebox = true;
	else
		exposurebox = false;

	//parm app.tooltip: 0|1, enable/disable tooltip display at startup. Tooltip display can still be toggled on/off with the 't' key.  Default=1
	if (myConfig::getConfig().getValueOrDefault("app.tooltip","1") == "1")
		SetToolTip("-: zoom out\n+: zoom in\nCtrl-c: Copy RGB values at the mouse x,y\ne: Exposure box toggle\nf,F: Fit image to window\nh: Toggle display thumbnail at the upper-left corner\nn: Take a snapshot of the display window.  This can be done repeatedly.\no: Out-of-bound toggle.  Rotates between off|RGB average|at least one channel\ns: Softproof toggle\nt,T: Tooltip toggle\nleft-arrow: Pan left, Shift = x10, Ctrl = x100\nright-arrow: Pan right, Shift = x10, Ctrl = x100\ndown-arrow: Pan down, Shift = x10, Ctrl = x100\nup-arrow: Pan up, Shift = x10, Ctrl = x100\n");



	Bind(wxEVT_SIZE, &PicPanel::OnSize, this);
	Bind(wxEVT_PAINT, &PicPanel::OnPaint,  this);
	Bind(wxEVT_LEFT_DOWN, &PicPanel::OnLeftDown,  this);
	Bind(wxEVT_RIGHT_DOWN, &PicPanel::OnRightDown,  this);
	Bind(wxEVT_LEFT_DCLICK, &PicPanel::OnLeftDoubleClicked,  this);
	Bind(wxEVT_LEFT_UP, &PicPanel::OnLeftUp,  this);
	Bind(wxEVT_MOTION, &PicPanel::OnMouseMove,  this);
	Bind(wxEVT_MOUSEWHEEL, &PicPanel::OnMouseWheel,  this);
	Bind(wxEVT_ENTER_WINDOW, &PicPanel::OnMouseEnter,  this);
	Bind(wxEVT_LEAVE_WINDOW, &PicPanel::OnMouseLeave,  this);
	//Bind(wxEVT_KEY_DOWN, &PicPanel::OnKey,  this);
	//Bind(wxEVT_CHAR, &PicPanel::OnKey,  this);
	Bind(wxEVT_CHAR_HOOK, &PicPanel::OnKey,  this);
	Bind(wxEVT_TIMER, &PicPanel::OnTimer,  this);
		
	//t = new wxTimer(this);
}

PicPanel::~PicPanel()
{
	if (image) image->~wxBitmap();
	if (thumbnail) thumbnail->~wxBitmap();
	//if (t) t->~wxTimer();
}

void PicPanel::OnSize(wxSizeEvent& event) 
{
	Refresh();
	event.Skip();
}

bool PicPanel::ToggleToolTip()
{
	if (GetToolTipText() == "") {
		SetToolTip("-: zoom out\n+: zoom in\nCtrl-c: Copy RGB values at the mouse x,y\ne: Exposure box toggle\nf,F: Fit image to window\nh: Toggle display thumbnail at the upper-left corner\nn: Take a snapshot of the display window.  This can be done repeatedly.\no: Out-of-bound toggle.  Rotates between off|RGB average|at least one channel\ns: Softproof toggle\nt,T: Tooltip toggle\nleft-arrow: Pan left, Shift = x10, Ctrl = x100\nright-arrow: Pan right, Shift = x10, Ctrl = x100\ndown-arrow: Pan down, Shift = x10, Ctrl = x100\nup-arrow: Pan up, Shift = x10, Ctrl = x100\n");
		return true;
	}
	else {
		UnsetToolTip();
		return false;
	}
}

void PicPanel::SetModified(bool m)
{
	modified = m;
	if (modified)
		((wxFrame *) GetParent())->SetStatusText("Modified",STATUS_MODIFIED);
	else
		((wxFrame *) GetParent())->SetStatusText("",STATUS_MODIFIED);
}

bool PicPanel::Modified()
{
	return modified;
}


void PicPanel::SetPic(gImage * dib, GIMAGE_CHANNEL channel)
{
	cmsHTRANSFORM dispTransform = NULL;

	if (dib) {
		//parm display.status: Write display... in status when setting the display image, 0|1.  Default=1
		if (myConfig::getConfig().getValueOrDefault("display.status","1") == "1")
			((wxFrame *) GetParent())->SetStatusText("display...");
		mark();

		display_dib = dib;
		ch = channel;
		wxImage img;
		float sspeed, aperture, iso;
		cmsUInt32Number dwflags = 0;
		
		int localoob = oob;
		//parm display.outofbound: Enable/disable out-of-bound pixel marking, 0|1.  In display pane 'o' toggles between no oob, average of channels, and at least one channel.  Default=0
		if (myConfig::getConfig().getValueOrDefault("display.outofbound","0") == "0") localoob = 0;

		exposurestring.Clear();
		std::string infoitem;
		infoitem = dib->getInfoValue("ExposureTime");
		if (infoitem != "") {
			sspeed = atof(infoitem.c_str());
			if (sspeed < 1.0)
				exposurestring += wxString::Format("1/%dsec  ", int(round(1.0/sspeed)));
			else
				exposurestring += wxString::Format("%dsec  ", int(sspeed));
		}
		infoitem = dib->getInfoValue("FNumber");
		aperture = atof(infoitem.c_str());
		if (infoitem != "") exposurestring += wxString::Format("f%s  ",wxString(infoitem));
		infoitem = dib->getInfoValue("ISOSpeedRatings");
		iso = atof(infoitem.c_str());
		if (infoitem != "") exposurestring += wxString::Format("ISO%s  ",wxString(infoitem));
		infoitem = dib->getInfoValue("FocalLength");
		if (infoitem != "") exposurestring += wxString::Format("%smm  ",wxString(infoitem));

		float ev = log2(pow(aperture,2)/sspeed);
		float lv = 2 * log2(aperture) - log2(sspeed) - log2(iso/100);
		//parm display.info.evlv: Add either/both EV (Exposure Value, Wikipedia definition) or LV (Light Value, exiftool definition) to the information string. "EV" and/or "LV" need to be in the property value, you can separate them with blanks, a comma, or just run them together.  Default: blank. 
		wxString evlv = wxString(myConfig::getConfig().getValueOrDefault("display.info.evlv",""));
		if (evlv.Find("EV") != wxNOT_FOUND & !std::isnan(ev)) exposurestring.Append(wxString::Format("EV%0.1f ",ev));
		if (evlv.Find("LV") != wxNOT_FOUND & !std::isnan(lv)) exposurestring.Append(wxString::Format("LV%0.1f ",lv));

		//parm display.thumbsize: The largest dimension of the thumbnail. Default=150
		unsigned thumbsize = atoi(myConfig::getConfig().getValueOrDefault("display.thumbsize","150").c_str());
		thumbh = thumbw = thumbsize;
		
		//parm display.cms: Enable color tranform of the display image, 0|1.  Default=1
		if (myConfig::getConfig().getValueOrDefault("display.cms","1") == "1") {

			cmsUInt32Number informat;
			if (sizeof(PIXTYPE) == 2) informat = TYPE_RGB_HALF_FLT; 
			if (sizeof(PIXTYPE) == 4) informat = TYPE_RGB_FLT;
			if (sizeof(PIXTYPE) == 8) informat = TYPE_RGB_DBL;

			wxString resultstr = "";
			cmsHPROFILE hImgProfile=NULL, hSoftProofProfile=NULL;
		
			wxFileName profilepath;
			profilepath.AssignDir(wxString(myConfig::getConfig().getValueOrDefault("cms.profilepath","")));
		
			//parm display.cms.displayprofile: Filename of display profile. One of the srgb|wide|adobe|prophoto built-ins can be used in a pinch.   Default: none.
			//template display.cms.displayprofile=iccfile
			wxString iccfile = wxString(myConfig::getConfig().getValueOrDefault("display.cms.displayprofile",""));
			profilepath.SetFullName(iccfile); 
			
			//parm display.cms.displayprofile.<number>: Filename of the display profile for the enumerated display.
			std::map<std::string, std::string> p = myConfig::getConfig().getSubset("display.cms.displayprofile.");
			int disp = wxDisplay::GetFromWindow(this);
			for (std::map<std::string, std::string>::iterator it=p.begin(); it!=p.end(); ++it) {
				int name = atoi(it->first.c_str());
				std::string val =  it->second.c_str();
				if (name == disp) profilepath.SetFullName(wxString(val)); 
			}
			
			printf("display profile: %s\n",profilepath.GetFullName().ToStdString().c_str()); fflush(stdout);

			//parm display.cms.displaygamma: Float number representing the gamma TRC to use if the displayprofile is one of the built-ins.  Default: 2.2
			float displaygamma = atof(myConfig::getConfig().getValueOrDefault("display.cms.displaygamma","2.2").c_str());
	
			if (iccfile == "srgb" | iccfile == "wide" | iccfile == "adobe" | iccfile == "prophoto" | iccfile == "identity") {
				displayProfile = gImage::makeLCMSProfile(iccfile.ToStdString(), displaygamma);
			}
			else if (iccfile == "srgb-output") {
				displayProfile = gImage::makeLCMSStoredProfile(iccfile.ToStdString());
			}
			else {
				if (profilepath.FileExists()) 
					displayProfile = cmsOpenProfileFromFile(profilepath.GetFullPath().c_str(), "r");
				else 
					displayProfile = NULL;
			}
		
			if (dib->getProfile() != NULL & dib->getProfileLength() > 0) 
				hImgProfile = cmsOpenProfileFromMem(dib->getProfile(), dib->getProfileLength());
		
			if (hImgProfile) {
				
			
				//parm display.cms.renderingintent: Specify the rendering intent for the display transform, perceptual|saturation|relative_colorimetric|absolute_colorimetric.  Default=perceptual
				wxString intentstr = wxString(myConfig::getConfig().getValueOrDefault("display.cms.renderingintent","perceptual"));
				cmsUInt32Number intent = INTENT_PERCEPTUAL;
				if (intentstr == "perceptual") intent = INTENT_PERCEPTUAL;
				if (intentstr == "saturation") intent = INTENT_SATURATION;
				if (intentstr == "relative_colorimetric") intent = INTENT_RELATIVE_COLORIMETRIC;
				if (intentstr == "absolute_colorimetric") intent = INTENT_ABSOLUTE_COLORIMETRIC;
			
				//parm display.cms.blackpointcompensation: Perform display color transform with black point compensation.  Default=1  
				if (myConfig::getConfig().getValueOrDefault("display.cms.blackpointcompensation","1") == "1") dwflags = dwflags | cmsFLAGS_BLACKPOINTCOMPENSATION;

				if (softproof) {
					cmsUInt32Number proofintent;
					hSoftProofProfile = NULL;
					dwflags = dwflags | cmsFLAGS_SOFTPROOFING;

					//parm display.cms.softproof.profile: Sets the ICC profile to be used for softproofing.  Default="", which disables soft proofing.
					//template display.cms.softproof.profile=iccfile	
					profilepath.SetFullName(wxString(myConfig::getConfig().getValueOrDefault("display.cms.softproof.profile","").c_str()));
					
					if (profilepath.FileExists()) {
						hSoftProofProfile = cmsOpenProfileFromFile(profilepath.GetFullPath().c_str(), "r");

						//parm display.cms.softproof.renderingintent: Specify the rendering intent for the display transform, perceptual|saturation|relative_colorimetric|absolute_colorimetric.  Default=perceptual
						wxString proofintentstr = wxString(myConfig::getConfig().getValueOrDefault("display.cms.softproof.renderingintent","perceptual"));
						proofintent = INTENT_PERCEPTUAL;
						if (proofintentstr == "perceptual") proofintent = INTENT_PERCEPTUAL;
						if (proofintentstr == "saturation") proofintent = INTENT_SATURATION;
						if (proofintentstr == "relative_colorimetric") proofintent = INTENT_RELATIVE_COLORIMETRIC;
						if (proofintentstr == "absolute_colorimetric") proofintent = INTENT_ABSOLUTE_COLORIMETRIC;

						//parm display.cms.softproof.gamutcheck: Perform softproofing color transform with gamut check, marking out-of-gamut colors.  Default=0  
						if (myConfig::getConfig().getValueOrDefault("display.cms.softproof.gamutcheck","0") == "1") dwflags = dwflags | cmsFLAGS_GAMUTCHECK;
			
						if (displayProfile) {
							if (hSoftProofProfile) {
								img = gImageFloat2wxImage(*dib, displayProfile, hSoftProofProfile, localoob, dwflags);
								if (dib->getLastError() != GIMAGE_OK) {
									resultstr.Append(":xform_error");
									img = gImageFloat2wxImage(*dib, NULL, NULL, localoob, dwflags);
								}
								else {
									resultstr.Append(":softproof");
								}
							}
							else {
								resultstr.Append(":soft_error");
								img = gImageFloat2wxImage(*dib, NULL, NULL, localoob, dwflags);
							}
						}
						else {
							resultstr.Append(":disp_error");
							img = gImageFloat2wxImage(*dib, NULL, NULL, localoob, dwflags);
						}
					}
					else  {
						resultstr.Append(":file_error");
						img = gImageFloat2wxImage(*dib, NULL, NULL, localoob, dwflags);
					}
				}
				else {
					if (displayProfile) {
						img = gImageFloat2wxImage(*dib, displayProfile, NULL, localoob, 0);
						if (dib->getLastError() != GIMAGE_OK) {
							resultstr.Append(":xform_error");
							img = gImageFloat2wxImage(*dib, NULL, NULL, localoob, dwflags);
						}
						else {
							resultstr.Append(":display");
						}
					}
					else {
						resultstr.Append(":disp_error");
						img = gImageFloat2wxImage(*dib, NULL, NULL, localoob, dwflags);
					}
				}
				
				displayTransform = cmsCreateTransform(  //for crop and rotate tools...
							hImgProfile, TYPE_RGB_8,
							displayProfile, TYPE_RGB_8,
							intent, dwflags);
			}
			else {
				resultstr.Append(":prof_error");
				img = gImageFloat2wxImage(*dib, NULL, NULL, localoob, dwflags);
			}

			((wxFrame *) GetParent())->SetStatusText(wxString::Format("CMS%s",resultstr),STATUS_CMS);

		}
		else { // no display CMS, just display the "raw" image:
			img = gImageFloat2wxImage(*dib, NULL, NULL, localoob, dwflags);
			((wxFrame *) GetParent())->SetStatusText("",STATUS_CMS);
		}

		
		if (image) image->~wxBitmap();
		image = new wxBitmap(img);
		imagew = image->GetWidth();
		imageh = image->GetHeight();
		
		if (imagew > imageh)
			thumbh = thumbw * ((float) imageh / (float) imagew);
		else
			thumbw = thumbh * ((float) imagew / (float) imageh);
		thumbhscale = (float) thumbh / (float) imageh;
		thumbwscale = (float) thumbw / (float) imagew;
		thumbnail = new wxBitmap(img.Scale(thumbw,thumbh,wxIMAGE_QUALITY_HIGH));

	
		//parm histogram.scale: The number of buckets to display in the histogram. Default=256
		unsigned scale = atoi(myConfig::getConfig().getValueOrDefault("histogram.scale","256").c_str());
		
		histogram->SetPic(dib, scale);
		//parm histogram.singlechannel: 0|1, turns on/off the display of single-channel histogram plot for per-channel curves
		if (myConfig::getConfig().getValueOrDefault("histogram.singlechannel","1") == "1")
			histogram->SetChannel(channel);
		else
			histogram->SetChannel(CHANNEL_RGB);

		Refresh();
		((wxFrame *) GetParent())->SetStatusText("");

		wxString d = duration();

		//parm display.render.log: 0|1, turn on/off logging of the duration of the display render operation. Default=0
		if ((myConfig::getConfig().getValueOrDefault("display.all.log","0") == "1") || 	(myConfig::getConfig().getValueOrDefault("display.render.log","0") == "1"))
			log(wxString::Format(_("display,time=%s"),d));


	}
}

/*
void PicPanel::SetPic(gImage * dib, GIMAGE_CHANNEL channel)
{
	cmsHTRANSFORM dispTransform = NULL;

	if (dib) {
		//parm display.status: Write display... in status when setting the display image, 0|1.  Default=1
		if (myConfig::getConfig().getValueOrDefault("display.status","1") == "1")
			((wxFrame *) GetParent())->SetStatusText("display...");
		mark();

		display_dib = dib;
		ch = channel;
		wxImage img;
		float sspeed, aperture, iso;

		exposurestring.Clear();
		std::string infoitem;
		infoitem = dib->getInfoValue("ExposureTime");
		if (infoitem != "") {
			sspeed = atof(infoitem.c_str());
			if (sspeed < 1.0)
				exposurestring += wxString::Format("1/%dsec  ", int(round(1.0/sspeed)));
			else
				exposurestring += wxString::Format("%dsec  ", int(sspeed));
		}
		infoitem = dib->getInfoValue("FNumber");
		aperture = atof(infoitem.c_str());
		if (infoitem != "") exposurestring += wxString::Format("f%s  ",wxString(infoitem));
		infoitem = dib->getInfoValue("ISOSpeedRatings");
		iso = atof(infoitem.c_str());
		if (infoitem != "") exposurestring += wxString::Format("ISO%s  ",wxString(infoitem));
		infoitem = dib->getInfoValue("FocalLength");
		if (infoitem != "") exposurestring += wxString::Format("%smm  ",wxString(infoitem));

		float ev = log2(pow(aperture,2)/sspeed);
		float lv = 2 * log2(aperture) - log2(sspeed) - log2(iso/100);
		//parm display.info.evlv: Add either/both EV (Exposure Value, Wikipedia definition) or LV (Light Value, exiftool definition) to the information string. "EV" and/or "LV" need to be in the property value, you can separate them with blanks, a comma, or just run them together.  Default: blank. 
		wxString evlv = wxString(myConfig::getConfig().getValueOrDefault("display.info.evlv",""));
		if (evlv.Find("EV") != wxNOT_FOUND & !std::isnan(ev)) exposurestring.Append(wxString::Format("EV%0.1f ",ev));
		if (evlv.Find("LV") != wxNOT_FOUND & !std::isnan(lv)) exposurestring.Append(wxString::Format("LV%0.1f ",lv));

		//parm display.thumbsize: The largest dimension of the thumbnail. Default=150
		unsigned thumbsize = atoi(myConfig::getConfig().getValueOrDefault("display.thumbsize","150").c_str());
		thumbh = thumbw = thumbsize;
		
		//parm display.cms: Enable color tranform of the display image, 0|1.  Default=1
		if (myConfig::getConfig().getValueOrDefault("display.cms","1") == "1") {

			cmsUInt32Number informat;
			if (sizeof(PIXTYPE) == 2) informat = TYPE_RGB_HALF_FLT; 
			if (sizeof(PIXTYPE) == 4) informat = TYPE_RGB_FLT;
			if (sizeof(PIXTYPE) == 8) informat = TYPE_RGB_DBL;

			wxString resultstr = "";
			cmsHPROFILE hImgProfile=NULL, hSoftProofProfile=NULL;
		
			wxFileName profilepath;
			profilepath.AssignDir(wxString(myConfig::getConfig().getValueOrDefault("cms.profilepath","")));
		
			//parm display.cms.displayprofile: Filename of display profile. One of the srgb|wide|adobe|prophoto built-ins can be used in a pinch.   Default: none.
			//template display.cms.displayprofile=iccfile
			wxString iccfile = wxString(myConfig::getConfig().getValueOrDefault("display.cms.displayprofile",""));
			profilepath.SetFullName(iccfile); 

			//parm display.cms.displaygamma: Float number representing the gamma TRC to use if the displayprofile is one of the built-ins.  Default: 2.2
			float displaygamma = atof(myConfig::getConfig().getValueOrDefault("display.cms.displaygamma","2.2").c_str());
	
			if (iccfile == "srgb" | iccfile == "wide" | iccfile == "adobe" | iccfile == "prophoto" | iccfile == "identity") {
				displayProfile = gImage::makeLCMSProfile(iccfile.ToStdString(), displaygamma);
			}
			else if (iccfile == "srgb-output") {
				displayProfile = gImage::makeLCMSStoredProfile(iccfile.ToStdString());
			}
			else {
				if (profilepath.FileExists()) 
					displayProfile = cmsOpenProfileFromFile(profilepath.GetFullPath().c_str(), "r");
				else 
					displayProfile = NULL;
			}
		
			if (dib->getProfile() != NULL & dib->getProfileLength() > 0) 
				hImgProfile = cmsOpenProfileFromMem(dib->getProfile(), dib->getProfileLength());
		
			if (hImgProfile) {
				cmsUInt32Number dwflags = 0;
			
				//parm display.cms.renderingintent: Specify the rendering intent for the display transform, perceptual|saturation|relative_colorimetric|absolute_colorimetric.  Default=perceptual
				wxString intentstr = wxString(myConfig::getConfig().getValueOrDefault("display.cms.renderingintent","perceptual"));
				cmsUInt32Number intent = INTENT_PERCEPTUAL;
				if (intentstr == "perceptual") intent = INTENT_PERCEPTUAL;
				if (intentstr == "saturation") intent = INTENT_SATURATION;
				if (intentstr == "relative_colorimetric") intent = INTENT_RELATIVE_COLORIMETRIC;
				if (intentstr == "absolute_colorimetric") intent = INTENT_ABSOLUTE_COLORIMETRIC;
			
				//parm display.cms.blackpointcompensation: Perform display color transform with black point compensation.  Default=1  
				if (myConfig::getConfig().getValueOrDefault("display.cms.blackpointcompensation","1") == "1") dwflags = dwflags | cmsFLAGS_BLACKPOINTCOMPENSATION;

				if (softproof) {
					cmsUInt32Number proofintent;
					hSoftProofProfile = NULL;
					dwflags = dwflags | cmsFLAGS_SOFTPROOFING;

					//parm display.cms.softproof.profile: Sets the ICC profile to be used for softproofing.  Default="", which disables soft proofing.
					//template display.cms.softproof.profile=iccfile	
					profilepath.SetFullName(wxString(myConfig::getConfig().getValueOrDefault("display.cms.softproof.profile","").c_str()));
					
					if (profilepath.FileExists()) {
						hSoftProofProfile = cmsOpenProfileFromFile(profilepath.GetFullPath().c_str(), "r");

						//parm display.cms.softproof.renderingintent: Specify the rendering intent for the display transform, perceptual|saturation|relative_colorimetric|absolute_colorimetric.  Default=perceptual
						wxString proofintentstr = wxString(myConfig::getConfig().getValueOrDefault("display.cms.softproof.renderingintent","perceptual"));
						proofintent = INTENT_PERCEPTUAL;
						if (proofintentstr == "perceptual") proofintent = INTENT_PERCEPTUAL;
						if (proofintentstr == "saturation") proofintent = INTENT_SATURATION;
						if (proofintentstr == "relative_colorimetric") proofintent = INTENT_RELATIVE_COLORIMETRIC;
						if (proofintentstr == "absolute_colorimetric") proofintent = INTENT_ABSOLUTE_COLORIMETRIC;

						//parm display.cms.softproof.gamutcheck: Perform softproofing color transform with gamut check, marking out-of-gamut colors.  Default=0  
						if (myConfig::getConfig().getValueOrDefault("display.cms.softproof.gamutcheck","0") == "1") dwflags = dwflags | cmsFLAGS_GAMUTCHECK;
			
						if (displayProfile) {
							if (hSoftProofProfile) {
								if (hImgProfile) {
									dispTransform = cmsCreateProofingTransform(
										hImgProfile, informat,
										displayProfile, TYPE_RGB_8,
										hSoftProofProfile,
										intent,
										proofintent,
										dwflags);
									if (dispTransform)
										resultstr.Append(":softproof");
									else
										resultstr.Append(":xform_error");
								}
								else resultstr.Append(":prof_error");
							}
							else resultstr.Append(":soft_error");
						}
						else resultstr.Append(":disp_error");
					}
					else  resultstr.Append(":file_error");
				}
				else {
					if (displayProfile) {
						dispTransform = cmsCreateTransform(
							hImgProfile, informat,
							displayProfile, TYPE_RGB_8,
							intent, dwflags);
						resultstr.Append(":display");
					}
					else resultstr.Append(":xform_error");
				}
				displayTransform = cmsCreateTransform(  //for crop and rotate tools...
							hImgProfile, TYPE_RGB_8,
							displayProfile, TYPE_RGB_8,
							intent, dwflags);
			}

			((wxFrame *) GetParent())->SetStatusText(wxString::Format("CMS%s",resultstr),STATUS_CMS);
	
			//if (dispTransform) 
			//	((wxFrame *) GetParent())->SetStatusText(wxString::Format("CMS%s",resultstr),STATUS_CMS);
			//else ((wxFrame *) GetParent())->SetStatusText("CMS:xform_error",STATUS_CMS);

		}
		else ((wxFrame *) GetParent())->SetStatusText("",STATUS_CMS);


		int localoob = oob;
		//parm display.outofbound: Enable/disable out-of-bound pixel marking, 0|1.  In display pane 'o' toggles between no oob, average of channels, and at least one channel.  Default=0
		if (myConfig::getConfig().getValueOrDefault("display.outofbound","0") == "0")
			localoob = 0;
		if (dispTransform) 
			img = gImage2wxImage(*dib, dispTransform, localoob);
		else 
			img = gImage2wxImage(*dib, localoob);
		
		
		if (image) image->~wxBitmap();
		image = new wxBitmap(img);
		imagew = image->GetWidth();
		imageh = image->GetHeight();
		
		if (imagew > imageh)
			thumbh = thumbw * ((float) imageh / (float) imagew);
		else
			thumbw = thumbh * ((float) imagew / (float) imageh);
		thumbhscale = (float) thumbh / (float) imageh;
		thumbwscale = (float) thumbw / (float) imagew;
		thumbnail = new wxBitmap(img.Scale(thumbw,thumbh,wxIMAGE_QUALITY_HIGH));

	
		//parm histogram.scale: The number of buckets to display in the histogram. Default=256
		unsigned scale = atoi(myConfig::getConfig().getValueOrDefault("histogram.scale","256").c_str());
		
		histogram->SetPic(dib, scale);
		//parm histogram.singlechannel: 0|1, turns on/off the display of single-channel histogram plot for per-channel curves
		if (myConfig::getConfig().getValueOrDefault("histogram.singlechannel","1") == "1")
			histogram->SetChannel(channel);
		else
			histogram->SetChannel(CHANNEL_RGB);

		Refresh();
		((wxFrame *) GetParent())->SetStatusText("");

		wxString d = duration();

		//parm display.render.log: 0|1, turn on/off logging of the duration of the display render operation. Default=0
		if ((myConfig::getConfig().getValueOrDefault("display.all.log","0") == "1") || 	(myConfig::getConfig().getValueOrDefault("display.render.log","0") == "1"))
			log(wxString::Format(_("display,time=%s"),d));


	}
}
*/


wxBitmap * PicPanel::getBitmap()
{
	return image;
}
		
		

void PicPanel::setStatusBar()
{
	if (!display_dib) return;

	if (imagex > 0 & imagex <= imagew & imagey > 0 & imagey <= imageh) {
		struct pix p = display_dib->getPixel(imagex, imagey);
		//parm display.statusbar.luminance: redmult,greenmult,bluemult - Multipliers used to calculate pixel luminance. Default:0.21,0.72,0.07
		wxArrayString lumstr = split(wxString(myConfig::getConfig().getValueOrDefault("display.statusbar.luminance","0.21,0.72,0.07")), ",");
		float lum = p.r * atof(lumstr[0].ToStdString().c_str()) + p.g * atof(lumstr[1].ToStdString().c_str()) + p.b * atof(lumstr[2].ToStdString().c_str());
		wxString stext = wxString::Format("xy:%d,%d rgb:%f,%f,%f lum:%f",imagex, imagey, p.r, p.g, p.b, lum);
		if (pixelbox) {
			struct pix sp = display_dib->getPixel(selectedx, selectedy);
			stext.Append(wxString::Format("   selected: xy%d,%d rgb:%f,%f,%f, ",selectedx, selectedy, sp.r, sp.g, sp.b));
		}
		((wxFrame *) GetParent())->SetStatusText(stext);
		//((wxFrame *) GetParent())->SetStatusText(wxString::Format("imagepos:%dx%d viewpos:%dx%d view:%dx%d xy:%d,%d",
		//	imageposx,imageposy,  viewposx, viewposy, vieww, viewh, imagex, imagey));
	}
	else
		((wxFrame *) GetParent())->SetStatusText("");

	wxString states;
	if (fit)
		states.Append("scale: fit");
	else
		states.Append(wxString::Format("scale: %.0f%%", scale*100));

	if (oob == 1)
		states.Append("  oob:avg");
	else if (oob == 2) 
		states.Append("  oob:1");

	if (softproof) states.Append("  soft");

	((wxFrame *) GetParent())->SetStatusText(states,STATUS_SCALE);
}

void PicPanel::drawBox(wxDC &dc, int x, int y, int w,int h)
{
	dc.DrawLine(x, y, x+w, y);
	dc.DrawLine(x+w, y, x+w, y+h);
	dc.DrawLine(x+w, y+h, x, y+h);
	dc.DrawLine(x, y+h, x,y);
}

void PicPanel::render(wxDC &dc)
{
	if (!image) return;
	int panelw, panelh,  scaledimagew, scaledimageh;

	//parm display.panelborder: Border around the display image.  Default: 5
	int border = atoi(myConfig::getConfig().getValueOrDefault("display.panelborder","5").c_str());

	dc.SetDeviceOrigin(border, border);
	GetSize(&panelw, &panelh);
	panelw -= border*2;
	panelh -= border*2;

	if (fit) {
		if (imagew > imageh) {
			scale = (double) panelw/ (double) imagew;
		}
		else {
			scale = (double) panelh/ (double) imageh;
		}
		viewposx = 0;
		viewposy = 0;
	}

	scaledimagew = imagew * scale;
	scaledimageh = imageh * scale;

	vieww = (float) panelw / scale;
	viewh = (float) panelh / scale;

	//lock pan position if scaled image is smaller than the display panel:
	if (scaledimagew <= panelw & scaledimageh <= panelh) {
		imageposx = panelw/2 - scaledimagew/2;
		imageposy = panelh/2 - scaledimageh/2;
		viewposx = 0;
		viewposy = 0;
	}
	else {
		imageposx = 0;
		imageposy = 0;
	}

	//bound lower-right pan to image:
	if (viewposx+vieww > imagew) viewposx = (imagew - vieww);
	if (viewposy+viewh > imageh) viewposy = (imageh - viewh);

	//bound upper-left pan to image:
	if (viewposx < 0) viewposx = 0;
	if (viewposy < 0) viewposy = 0;

	//setStatusBar();

	//write the display image:
	{
		wxMemoryDC mdc;
		mdc.SelectObject(*image);
		dc.StretchBlit(imageposx,imageposy, panelw, panelh, &mdc, viewposx, viewposy, vieww, viewh);
		mdc.SelectObject(wxNullBitmap);
	}

	//parm display.snapshot: 1|0, enables|disables the 'n' key and snapshooting of the display.  Default: 0
	if (snapshot & myConfig::getConfig().getValueOrDefault("display.snapshot","0") == "1") {
		int snapw, snaph;
		if (scaledimagew <= panelw & scaledimageh <= panelh) {
			snapw = scaledimagew;
			snaph = scaledimageh;
		}
		else {
			snapw = panelw;
			snaph = panelh;
		}
		wxBitmap snap(snapw, snaph);
		wxMemoryDC mydc, paneldc;
		paneldc.SelectObject(*image);
		mydc.SelectObject(snap);
		mydc.StretchBlit(0,0, panelw, panelh, &paneldc, viewposx, viewposy, vieww, viewh);
		mydc.SelectObject(wxNullBitmap);
		paneldc.SelectObject(wxNullBitmap);
		mySnapshotDialog *d = new mySnapshotDialog (this, wxID_ANY, "Snapshot", snap);
		d->Show();
	}
	snapshot = false;

//
/*
	//write the display image, slowly.  Legacy (circa v8) code for reference, doesn't work quite right here:
	int iw = image->GetWidth()*scale;
	int ih = image->GetHeight()*scale;
	int picX=imageposx, picY=imageposy;
	if (iw < panelw) {
		picX = (float) panelw/2 - (float) iw/2;
	}
	else {
		if (picX < -(iw-panelw))
			picX = panelw-iw;
		else if (picX > 0)
			picX = 0;
	}

	if (ih < panelh) {
		picY = (float) panelh/2 - (float) ih/2;
	}
	else {
		if (picY < -(ih-panelh))
			picY = panelh-ih;
		else if (picY > 0)
			picY = 0;
	}
	wxBitmap dimage(image->ConvertToImage().Scale(iw, ih, wxIMAGE_QUALITY_HIGH));
	//dc.DrawBitmap(dimage,picX,picY, false);
	dc.DrawBitmap(dimage,imageposx,imageposy, false);
*/

	PicProcessor *selected = PicProcessor::getSelectedPicProcessor(commandtree);
	if (selected) dcList = selected->getDrawList();
	if (!pixbox.IsEmpty()) dcList.Append(pixbox);

	//write the tool-supplied plots:
	if (dcList != "") {
		dc.SetPen(*wxYELLOW_PEN);
		wxArrayString l = split(dcList, ";");
		for (unsigned i=0; i<l.GetCount(); i++) {
			wxArrayString c = split(l[i],",");
			if (c[0] == "cross") {
				if (c.GetCount() < 3) continue;
				//convert image coordinates to panel:
				int px = (atoi(c[1].c_str())-viewposx) * scale + imageposx;
				int py = (atoi(c[2].c_str())-viewposy) * scale + imageposy;
				dc.DrawLine(px-10, py, px+10, py);
				dc.DrawLine(px, py-10, px, py+10);
			}
			if (c[0] == "box") {
				if (c.GetCount() < 3) continue;
				int px = (atoi(c[1].c_str())-viewposx) * scale + imageposx;
				int py = (atoi(c[2].c_str())-viewposy) * scale + imageposy;
				dc.DrawLine(px-10, py, px+10, py);
				dc.DrawLine(px, py-10, px, py+10);

				dc.DrawLine(px, py, px+scale, py);
				dc.DrawLine(px+scale, py, px+scale, py+scale);
				dc.DrawLine(px+scale, py+scale, px, py+scale);
				dc.DrawLine(px, py+scale, px, py);
			}
		}
	}

	//if the thumbnail is visible, write it and its viewport:
	if (thumbvisible) {
		dc.SetPen(wxPen(wxColour(0,0,0),1));
		dc.DrawRectangle(0,0,thumbw+4, thumbh+4);			
		dc.SetPen(wxPen(wxColour(255,255,255),1));
		dc.DrawRectangle(1,1,thumbw+2, thumbh+2);
		dc.DrawBitmap(*thumbnail,2,2);

		dc.SetClippingRegion(2,2,thumbw,thumbh);
		dc.SetPen(wxPen(wxColour(255,255,255),1));
		if (vieww < imagew | viewh < imageh)
			drawBox(dc, 
				(int) ((viewposx+2) * thumbwscale), 
				(int) ((viewposy+2) * thumbhscale),
				(int) (vieww * thumbwscale),
				(int) (viewh * thumbhscale));
	}

	if (exposurebox  & !exposurestring.IsEmpty()) {
		dc.DestroyClippingRegion();
		wxPoint p(10,panelh-30);
		wxSize exp = dc.GetTextExtent(exposurestring);
		//parm display.exposuredata.backgroundcolor: Sets background color of the exposure box.  Specify an integer RGB triplet or single of 0-255 (e.g., 128,128,128 or just 128 for gray).  Default=192 (light gray)
		dc.SetBrush(wxBrush(wxString2wxColour(wxString(myConfig::getConfig().getValueOrDefault("display.exposuredata.backgroundcolor","192")))));
		//parm display.exposuredata.outlinecolor: Sets border color of the exposure box.  Specify an integer RGB triplet or single of 0-255 (e.g., 128,128,128 or just 128 for gray).  Default=0 (black)
		dc.SetPen(wxPen(wxString2wxColour(wxString(myConfig::getConfig().getValueOrDefault("display.exposuredata.outlinecolor","0"))),1));
		dc.DrawRectangle(p.x-3,p.y-3,exp.GetWidth()+6,exp.GetHeight()+6);
		//parm display.exposuredata.outlinecolor: Sets text color of the exposure box.  Specify an integer RGB triplet or single of 0-255 (e.g., 128,128,128 or just 128 for gray).  Default=0 (black)
		dc.SetTextForeground(wxString2wxColour(wxString(myConfig::getConfig().getValueOrDefault("display.exposuredata.textcolor","0"))));
		dc.DrawText(exposurestring, p);
	}

}

void PicPanel::OnMouseWheel(wxMouseEvent& event)
{
	fit=false;
	int mx = event.m_x;
	int my = event.m_y;

	double increment = 0.05;
	if (event.ShiftDown()) increment = 0.2;
	if (event.ControlDown()) increment = 1.0;

	int border = atoi(myConfig::getConfig().getValueOrDefault("display.panelborder","5").c_str());

	if (event.GetWheelRotation() > 0)
		scale += increment;
	else
		scale -= increment;

	//parm display.minscale: Smallest panel image size.  Default=0.1, or 10%
	float minscale = atof(myConfig::getConfig().getValueOrDefault("display.minscale","0.1").c_str());
	//parm display.maxscale: Smallest panel image size.  Default=5.0, or 500%
	float maxscale = atof(myConfig::getConfig().getValueOrDefault("display.maxscale","5.0").c_str());
	if (scale < minscale) 
		scale = minscale;
	else if (scale > maxscale) 
		scale = maxscale; 

	//keep center of panel in the center...
	int dimagex = imagex - ((((mx-border) - imageposx) / scale) + (viewposx));
	int dimagey = imagey - ((((my-border) - imageposy) / scale) + (viewposy));
	viewposx += dimagex;
	viewposy += dimagey;

	imagex = (((mx-border) - imageposx) / scale) + (viewposx);
	imagey = (((my-border) - imageposy) / scale) + (viewposy);

	mousex = mx;
	mousey = my;

	setStatusBar();

	event.Skip();
	Refresh();
}

void PicPanel::OnTimer(wxTimerEvent& event)
{
	Refresh();
}

void PicPanel::OnLeftDoubleClicked(wxMouseEvent& event)
{
	int mx = event.m_x;
	int my = event.m_y;
	
	if (mx < thumbw & my < thumbh) {
		if (thumbvisible) thumbvisible = false; else thumbvisible = true;
		Refresh();
		event.Skip();
		return;
	}

	int border = atoi(myConfig::getConfig().getValueOrDefault("display.panelborder","5").c_str());

	int panelw, panelh;
	GetSize(&panelw,&panelh);

	if (scale != 1.0) {
		scale = 1.0;
		fit=false;
	}
	else {
		fit=true;
	}

	//center the view on the pixel that was double-clicked:
	viewposx = imagex - (panelw/2);
	viewposy = imagey - (panelh/2);

	imagex = (((mx-border) - imageposx) / scale) + (viewposx);
	imagey = (((my-border) - imageposy) / scale) + (viewposy);

	mousex = mx;
	mousey = my;

	setStatusBar();

	event.Skip();
	Refresh();
}

double PicPanel::GetScale()
{
	return scale;
}
	
coord PicPanel::GetImgCoords()
{
	return coord {imagex, imagey};
}
	
void PicPanel::PaintNow()
{
	wxClientDC dc(this);
	render(dc);
}

void PicPanel::OnPaint(wxPaintEvent & event)
{
	wxPaintDC dc(this);
	render(dc);
}

void PicPanel::OnMouseEnter(wxMouseEvent& event)
{
	thumbdragging = dragging = false;
	((wxFrame *) GetParent())->SetStatusText("");
}

void PicPanel::OnMouseLeave(wxMouseEvent& event)
{
	thumbdragging = dragging = false;
	((wxFrame *) GetParent())->SetStatusText("");
}

void PicPanel::OnLeftDown(wxMouseEvent& event)
{
	SetFocus();
	int mx = event.m_x;
	int my = event.m_y;

	int border = atoi(myConfig::getConfig().getValueOrDefault("display.panelborder","5").c_str());

	if (thumbvisible && mx < thumbw & my < thumbh) 
		thumbdragging = true;
	else
		dragging = true;

	imagex = (((mx-border) - imageposx) / scale) + (viewposx);
	imagey = (((my-border) - imageposy) / scale) + (viewposy);

	mousex = mx;
	mousey = my;

	if (event.ShiftDown() && pixelbox) {
		selectedx = imagex;
		selectedy = imagey;
		pixbox = wxString::Format("box,%d,%d;",selectedx, selectedy);
	}

	setStatusBar();
	event.Skip();
}

void PicPanel::OnMouseMove(wxMouseEvent& event)
{
	int mx = event.m_x;
	int my = event.m_y;

	int border = atoi(myConfig::getConfig().getValueOrDefault("display.panelborder","5").c_str());

	if (thumbvisible && thumbdragging && mx < thumbw & my < thumbh) {
		if (skipmove < 3) {  //kinda smooths out panning with the thumb viewport...
			skipmove++;
			return;
		}
		skipmove=0;
		viewposx -= (mousex - mx) * ((float) imagew / (float) thumbw);
		viewposy -= (mousey - my) * ((float) imageh / (float) thumbh);
		Refresh();
	}
	else if (!fit & dragging) { 
		viewposx -= (float) (mx - mousex) / scale;
		viewposy -= (float) (my - mousey) / scale;
		Refresh();
	}

	imagex = (((mx-border) - imageposx) / scale) + (viewposx);
	imagey = (((my-border) - imageposy) / scale) + (viewposy);

	mousex = mx;
	mousey = my;

	setStatusBar();
	event.Skip();
}

void PicPanel::OnLeftUp(wxMouseEvent& event)
{
	thumbdragging = dragging = false;
	Refresh();
}

void PicPanel::SetThumbMode(int mode)
{
	if (mode == 0) 
		thumbvisible = false;
	else
		thumbvisible = true;
	Refresh();
}


void PicPanel::RefreshPic()
{
	if (display_dib) SetPic(display_dib, ch);
}


void PicPanel::SetDrawList(wxString list)
{
	dcList = list;
	Refresh();
}
   

void PicPanel::SetColorManagement(bool b)
{
	colormgt = b;
	RefreshPic();
}

void PicPanel::OnKey(wxKeyEvent& event)
{
	float dimagex, dimagey, increment, maxscale, minscale;

	int panincrement = 1;
	if (event.ShiftDown()) panincrement = 10;
	if (event.ControlDown()) panincrement = 100;

	int border = atoi(myConfig::getConfig().getValueOrDefault("display.panelborder","5").c_str());

	int mx, my;
	GetSize(&mx,&my);
	
	wxChar uc = event.GetUnicodeKey();
	if ( uc != WXK_NONE )
	{
		// It's a "normal" character. Notice that this includes
		// control characters in 1..31 range, e.g. WXK_RETURN or
		// WXK_BACK, so check for them explicitly.
		if ( uc >= 32 )
		{
			switch (uc) {
				//key -: zoom out
				case 45: //- zoom out
					fit=false;
					GetSize(&mx,&my);
					mx /= 2; my /=2;

					increment = 0.5;
					if (event.ShiftDown()) increment = 1.0;

					border = atoi(myConfig::getConfig().getValueOrDefault("display.panelborder","5").c_str());

					scale -= increment;

					minscale = atof(myConfig::getConfig().getValueOrDefault("display.minscale","0.1").c_str());
					maxscale = atof(myConfig::getConfig().getValueOrDefault("display.maxscale","5.0").c_str());
					if (scale < minscale) 
						scale = minscale;
					else if (scale > maxscale) 
						scale = maxscale; 
		
					//keep center of panel in the center...
					dimagex = imagex - ((((mx-border) - imageposx) / scale) + (viewposx));
					dimagey = imagey - ((((my-border) - imageposy) / scale) + (viewposy));
					viewposx += dimagex;
					viewposy += dimagey;

					imagex = (((mx-border) - imageposx) / scale) + (viewposx);
					imagey = (((my-border) - imageposy) / scale) + (viewposy);

					mousex = mx;
					mousey = my;

					setStatusBar();
					Refresh();
					break;

				//key +: zoom in
				case 61: //+ zoom in
					fit=false;

					GetSize(&mx,&my);
					mx /= 2; my /=2;

					increment = 0.05;
					if (event.ShiftDown()) increment = 0.2;

					border = atoi(myConfig::getConfig().getValueOrDefault("display.panelborder","5").c_str());

					scale += increment;

					minscale = atof(myConfig::getConfig().getValueOrDefault("display.minscale","0.1").c_str());
					maxscale = atof(myConfig::getConfig().getValueOrDefault("display.maxscale","5.0").c_str());
					if (scale < minscale) 
						scale = minscale;
					else if (scale > maxscale) 
						scale = maxscale; 

					//keep center of panel in the center...
					dimagex = imagex - ((((mx-border) - imageposx) / scale) + (viewposx));
					dimagey = imagey - ((((my-border) - imageposy) / scale) + (viewposy));
					viewposx += dimagex;
					viewposy += dimagey;

					imagex = (((mx-border) - imageposx) / scale) + (viewposx);
					imagey = (((my-border) - imageposy) / scale) + (viewposy);

					mousex = mx;
					mousey = my;

					setStatusBar();
					Refresh();
					break;

				//key Ctrl-c: Copy RGB values at the mouse x,y
				case 67: //c - with Ctrl-, copy RGB at the x,y
					if (event.ControlDown()) 
						if (display_dib)
							if (wxTheClipboard->Open()) {
								struct pix p = display_dib->getPixel(imagex, imagey);
								wxTheClipboard->SetData( new wxTextDataObject(wxString::Format("%f,%f,%f", p.r, p.g, p.b)) );
								wxTheClipboard->Close();
								((wxFrame *) GetParent())->SetStatusText(wxString::Format("RGB at %d,%d (%f,%f,%f) copied to clipboard",imagex, imagey, p.r, p.g, p.b));
							}
					break;

				//key e: Exposure box toggle
				case 69: //e exposure box toggle
					if (!exposurestring.IsEmpty()) {
						if (exposurebox) {
							exposurebox = false;
						}
						else {
							exposurebox = true;
						}
						Refresh();
					}
					break;

				//key f,F: Fit image to window
				case 102: //f
				case 70:  //F - fit image to window
					SetScaleToWidth();
					FitMode(true);
					setStatusBar();
					Refresh();
					break;

				//key h: Toggle display thumbnail at the upper-left corner
				case 72:  //h - toggle display thumbnail 
					if (thumbvisible)
						thumbvisible = false;
					else
						thumbvisible = true;
					Refresh();
					break;

				//key n: Take a snapshot of the display window.  This can be done repeatedly.
				case 78: //n snapshot
					snapshot = true;
					Refresh();
					break;

				//key o: Out-of-bound toggle.  Rotates between off|RGB average|at least one channel
				case 79: //o oob toggle
					if (myConfig::getConfig().getValueOrDefault("display.outofbound","0") == "1") {
						oob++;
						if (oob > 2) oob = 0;
						if (oob == 0)
							((wxFrame *) GetParent())->SetStatusText("out-of-bound: off");
						else if (oob == 1)
							((wxFrame *) GetParent())->SetStatusText("out-of-bound: average");
						else if (oob == 2)
							((wxFrame *) GetParent())->SetStatusText("out-of-bound: at least one channel");
						RefreshPic();
						setStatusBar();
					}
					break;

				case 80: //p - pixel navigator toggle
					if (pixelbox) {
						pixbox.Empty();
						pixelbox = false;
						((wxFrame *) GetParent())->SetStatusText("pixelbox: off");
					}
					else {
						pixelbox = true;
						((wxFrame *) GetParent())->SetStatusText("pixelbox: on");
					}
					RefreshPic();
					setStatusBar();
					break;

				//key s: Softproof toggle
				case 83: //s softproof toggle
					if (softproof) {
						softproof = false;
						((wxFrame *) GetParent())->SetStatusText("softproof: off");
					}
					else {
						softproof = true;
						((wxFrame *) GetParent())->SetStatusText("softproof: on");
					}
					RefreshPic();
					break;

				//key t,T: Tooltip toggle
				case 116: //t
				case 84: //T - toggle tooltip
						if (ToggleToolTip())
							((wxFrame *) GetParent())->SetStatusText("PicPanel tooltip display: on");
						else
							((wxFrame *) GetParent())->SetStatusText("PicPanel tooltip display: off");
					break;
			}
		}
		else
		{
			// It's a control character, < WXK_START
			switch (uc)
			{
				case WXK_TAB:
					event.Skip();
					break;
			}
		}
	}
	else // No Unicode equivalent.
	{
		// It's a special key, > WXK_START, deal with all the known ones:
		switch ( event.GetKeyCode() )
		{
			//key left-arrow: Pan left, Shift = x10, Ctrl = x100
			case WXK_LEFT:
				fit=false;

				mx /= 2; my /=2;

				viewposx += -panincrement;
				imagex = (((mx-border) - imageposx) / scale) + (viewposx);
				imagey = (((my-border) - imageposy) / scale) + (viewposy);

				mousex = mx;
				mousey = my;

				setStatusBar();
				Refresh();
				break;

			//key right-arrow: Pan right, Shift = x10, Ctrl = x100
			case WXK_RIGHT:
				fit=false;

				mx /= 2; my /=2;

				viewposx += panincrement;
				imagex = (((mx-border) - imageposx) / scale) + (viewposx);
				imagey = (((my-border) - imageposy) / scale) + (viewposy);

				mousex = mx;
				mousey = my;

				setStatusBar();
				Refresh();
				break;

			//key down-arrow: Pan down, Shift = x10, Ctrl = x100
			case WXK_DOWN:
				fit=false;

				mx /= 2; my /=2;

				viewposy += panincrement;
				imagex = (((mx-border) - imageposx) / scale) + (viewposx);
				imagey = (((my-border) - imageposy) / scale) + (viewposy);

				mousex = mx;
				mousey = my;

				setStatusBar();
				Refresh();
				break;

			//key up-arrow: Pan up, Shift = x10, Ctrl = x100
			case WXK_UP:
				fit=false;

				mx /= 2; my /=2;

				viewposy += -panincrement;
				imagex = (((mx-border) - imageposx) / scale) + (viewposx);
				imagey = (((my-border) - imageposy) / scale) + (viewposy);

				mousex = mx;
				mousey = my;

				setStatusBar();
				Refresh();
				break;


		}
	}
}


void PicPanel::SetScale(double s)
{
	scale = s;
}

void PicPanel::FitMode(bool f)
{
	fit = f;
}




//below methods of questionable utility...

bool PicPanel::GetColorManagement()
{
	return false;
}

void PicPanel::BlankPic()
{
	if (image) { 
		wxBitmap *dimg = new wxBitmap(image->ConvertToImage().ConvertToDisabled(128));
		image->~wxBitmap();
		image = dimg;
		display_dib = NULL;
	}
	PaintNow();
}

void PicPanel::SetProfile(gImage * dib)
{

}

void PicPanel::SetImageProfile(cmsHPROFILE hImgProf)
{

}

cmsHTRANSFORM PicPanel::GetDisplayTransform()
{
	return displayTransform;
}

wxString PicPanel::getHistogramString()
{
	return "";
}
        

void PicPanel::SetScaleToWidth()
{

}
	
void PicPanel::SetScaleToHeight()
{

}

void PicPanel::SetScaleToWidth(double percentofwidth)
{
	int w, h;
}


void PicPanel::OnRightDown(wxMouseEvent& event)
{

}





