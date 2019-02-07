
#include "PicPanel.h"
#include "util.h"
#include <vector>
#include "PicProcessor.h"
#include "myConfig.h"
#include <wx/clipbrd.h>


PicPanel::PicPanel(wxFrame *parent, wxTreeCtrl *tree, myHistogramPane *hgram): wxPanel(parent, wxID_ANY, wxDefaultPosition, wxSize(1000,740)) 
{

	SetDoubleBuffered(true); 
	display_dib = NULL;
	image = NULL;
	display_dib = NULL;
	scale = 1.0;
	imgctrx = 0.5; imgctry = 0.5;
	imageposx=0; imageposy = 0;
	mousex = 0; mousey=0;
	thumbdragging = dragging = false;
	thumbvisible = true;
	histogram = hgram;
	skipmove=0;
	oob = 0;
	dcList.clear();

	displayProfile = NULL;
	displayTransform = NULL;

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
	Bind(wxEVT_KEY_DOWN, &PicPanel::OnKey,  this);
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

void PicPanel::SetPic(gImage * dib, GIMAGE_CHANNEL channel)
{
	if (dib) {
		//parm display.status: Write display... in status when setting the display image, 0|1.  Default=1
		if (myConfig::getConfig().getValueOrDefault("display.status","1") == "1")
			((wxFrame *) GetParent())->SetStatusText("display...");

		display_dib = dib;
		ch = channel;
		wxImage img;

		//parm display.outofbound: Enable/disable out-of-bound pixel marking, 0|1.  In display pane 'o' toggles between no oob, average of channels, and at least one channel.  Default=0
		if (myConfig::getConfig().getValueOrDefault("display.outofbound","0") == "1")
			img = gImage2wxImage(*dib, oob);
		else
			img = gImage2wxImage(*dib);

		//parm display.thumbsize: The largest dimension of the thumbnail. Default=150
		unsigned thumbsize = atoi(myConfig::getConfig().getValueOrDefault("display.thumbsize","150").c_str());
		thumbh = thumbw = thumbsize;
		
		//parm display.cms: Enable color tranform of the display image, 0|1.  Default=1
		if (myConfig::getConfig().getValueOrDefault("display.cms","1") == "1") {
/*
			colormgt = true;
		else
			colormgt = false;

		if (colormgt) {
*/
			cmsHPROFILE hImgProfile, hSoftProofProfile;
		
			wxFileName profilepath;
			//parm cms.profilepath: Directory path where ICC colorspace profiles can be found.  Default: (none, implies current working directory)
			profilepath.AssignDir(wxString(myConfig::getConfig().getValueOrDefault("cms.profilepath","")));
		
			//parm display.cms.displayprofile: Filename of display profile.  Default: none.
			profilepath.SetFullName(wxString(myConfig::getConfig().getValueOrDefault("display.cms.displayprofile",""))); 
	
			if (profilepath.FileExists()) 
				displayProfile = cmsOpenProfileFromFile(profilepath.GetFullPath().c_str(), "r");
			else 
				displayProfile = NULL;
		
			if (dib->getProfile() != NULL & dib->getProfileLength() > 0) 
				hImgProfile = cmsOpenProfileFromMem(dib->getProfile(), dib->getProfileLength());
		
			if (hImgProfile) {
				cmsUInt32Number dwflags = 0;
				if (displayTransform) cmsDeleteTransform(displayTransform);
			
				//parm display.cms.renderingintent: Specify the rendering intent for the display transform, perceptual|saturation|relative_colorimetric|absolute_colorimetric.  Default=perceptual
				wxString intentstr = wxString(myConfig::getConfig().getValueOrDefault("display.cms.renderingintent","perceptual"));
				cmsUInt32Number intent = INTENT_PERCEPTUAL;
				if (intentstr == "perceptual") intent = INTENT_PERCEPTUAL;
				if (intentstr == "saturation") intent = INTENT_SATURATION;
				if (intentstr == "relative_colorimetric") intent = INTENT_RELATIVE_COLORIMETRIC;
				if (intentstr == "absolute_colorimetric") intent = INTENT_ABSOLUTE_COLORIMETRIC;
			
				//parm display.cms.blackpointcompensation: Perform display color transform with black point compensation.  Default=1  
				if (myConfig::getConfig().getValueOrDefault("display.cms.blackpointcompensation","1") == "1") dwflags = dwflags | cmsFLAGS_BLACKPOINTCOMPENSATION;

				cmsUInt32Number proofintent;
				//parm display.cms.softproof: Perform softproofing color transform.  Default=0  
				if (myConfig::getConfig().getValueOrDefault("display.cms.softproof","0") == "1") { 
					dwflags = dwflags | cmsFLAGS_SOFTPROOFING;
					//parm display.cms.softproof.profile: Sets the ICC profile to be used for softproofing.  Default="", which disables soft proofing.
					if (myConfig::getConfig().getValueOrDefault("display.cms.softproof.profile","") != "") {			
						profilepath.SetFullName(wxString(myConfig::getConfig().getValueOrDefault("display.cms.softproof.profile","").c_str()));
						hSoftProofProfile = cmsOpenProfileFromFile(profilepath.GetFullPath().c_str(), "r");
					}
					else hSoftProofProfile = NULL;
					//parm display.cms.softproof.renderingintent: Specify the rendering intent for the display transform, perceptual|saturation|relative_colorimetric|absolute_colorimetric.  Default=perceptual
					wxString proofintentstr = wxString(myConfig::getConfig().getValueOrDefault("display.cms.softproof.renderingintent","perceptual"));
					proofintent = INTENT_PERCEPTUAL;
					if (proofintentstr == "perceptual") proofintent = INTENT_PERCEPTUAL;
					if (proofintentstr == "saturation") proofintent = INTENT_SATURATION;
					if (proofintentstr == "relative_colorimetric") proofintent = INTENT_RELATIVE_COLORIMETRIC;
					if (proofintentstr == "absolute_colorimetric") proofintent = INTENT_ABSOLUTE_COLORIMETRIC;
					//parm display.cms.softproof.gamutcheck: Perform softproofing color transform with gamut check, marking out-of-gamut colors.  Default=0  
					if (myConfig::getConfig().getValueOrDefault("display.cms.softproof.gamutcheck","0") == "1") dwflags = dwflags | cmsFLAGS_GAMUTCHECK;
				}
			
				if (hImgProfile) {
					if (displayProfile) {
						if (hSoftProofProfile) {
							displayTransform = cmsCreateProofingTransform(
								hImgProfile, TYPE_RGB_8,
								displayProfile, TYPE_RGB_8,
								hSoftProofProfile,
								intent,
								proofintent,
								dwflags);
						}
						else {
						displayTransform = cmsCreateTransform(
							hImgProfile, TYPE_RGB_8,
							displayProfile, TYPE_RGB_8,
							intent, dwflags);
						}
					}
				}
			}
		
		
			if (displayTransform) {
				//cmsDoTransform(hTransform, img.GetData(), img.GetData(), img.GetWidth()*img.GetHeight());
				unsigned char* im = img.GetData();
				unsigned w = img.GetWidth();
				unsigned h = img.GetHeight();
				#pragma omp parallel for
				for (unsigned y=0; y<h; y++) {
					unsigned pos = y*w*3;
					cmsDoTransform(displayTransform, &im[pos], &im[pos], w);
				}
				((wxFrame *) GetParent())->SetStatusText("CMS",1);
			}
			else ((wxFrame *) GetParent())->SetStatusText("CMS:err",1);
		}
		else ((wxFrame *) GetParent())->SetStatusText("",1);
		
		
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
		
		histogram->SetPic(*dib, scale);
		//parm histogram.singlechannel: 0|1, turns on/off the display of single-channel histogram plot for per-channel curves
		if (myConfig::getConfig().getValueOrDefault("histogram.singlechannel","1") == "1")
			histogram->SetChannel(channel);
		else
			histogram->SetChannel(CHANNEL_RGB);

		Refresh();
		((wxFrame *) GetParent())->SetStatusText("");
	}
}

void PicPanel::setStatusBar()
{
	if (display_dib)
		struct pix p = display_dib->getPixel(imagex, imagey);
	else
		return;

	if (imagex > 0 & imagex <= imagew & imagey > 0 & imagey <= imageh) {
		struct pix p = display_dib->getPixel(imagex, imagey);
		((wxFrame *) GetParent())->SetStatusText(wxString::Format("xy:%d,%d rgb:%f,%f,%f",imagex, imagey, p.r, p.g, p.b));
		//((wxFrame *) GetParent())->SetStatusText(wxString::Format("imagepos:%dx%d viewpos:%dx%d view:%dx%d xy:%d,%d",
		//	imageposx,imageposy,  viewposx, viewposy, vieww, viewh, imagex, imagey));
	}
	else
		((wxFrame *) GetParent())->SetStatusText("");

	if (fit)
		((wxFrame *) GetParent())->SetStatusText("scale: fit",2);
	else
		((wxFrame *) GetParent())->SetStatusText(wxString::Format("scale: %.0f%%", scale*100),2);
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
	wxMemoryDC mdc;
	mdc.SelectObject(*image);
	dc.StretchBlit(imageposx,imageposy, panelw, panelh, &mdc, viewposx, viewposy, vieww, viewh);
	mdc.SelectObject(wxNullBitmap);

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

	if (scale < 0.1) 
		scale = 0.1;
	else if (scale > 5) 
		scale = 5; 

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
	SetFocus();
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

	
void PicPanel::BlankPic()
{

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

bool PicPanel::GetColorManagement()
{
	return false;
}


void PicPanel::SetProfile(gImage * dib)
{

}

void PicPanel::SetImageProfile(cmsHPROFILE hImgProf)
{

}

cmsHTRANSFORM PicPanel::GetDisplayTransform()
{
	return NULL;
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

void PicPanel::SetScale(double s)
{
	scale = s;
}

void PicPanel::FitMode(bool f)
{
	fit = f;
}


void PicPanel::OnRightDown(wxMouseEvent& event)
{

}



void PicPanel::OnKey(wxKeyEvent& event)
{
	event.Skip();
	//if (blank) return;
	//((wxFrame *) GetParent())->SetStatusText(wxString::Format("PicPanel: keycode=%d", event.GetKeyCode()));
	switch (event.GetKeyCode()) {
		case 116: //t
		case 84: //T - toggle display thumbnail
			if (thumbvisible)
				thumbvisible = false;
			else
				thumbvisible = true;
			Refresh();
			break;

		case 67: //c - with Ctrl-, copy RGB at the x,y
			if (event.ControlDown()) 
				if (display_dib)
					if (wxTheClipboard->Open()) {
						struct pix p = display_dib->getPixel(imagex, imagey);
						wxTheClipboard->SetData( new wxTextDataObject(wxString::Format("%f,%f,%f", p.r, p.g, p.b)) );
						wxTheClipboard->Close();
					}
			break;

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
				SetPic(display_dib, ch);
				Refresh();
			}
			break;
	}
}


