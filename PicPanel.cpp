
#include "PicPanel.h"
#include "util.h"
#include <vector>
#include "PicProcessor.h"
#include "myConfig.h"
#include <wx/clipbrd.h>


BEGIN_EVENT_TABLE(PicPanel, wxPanel)
    EVT_PAINT(PicPanel::OnPaint)
    EVT_LEFT_DOWN(PicPanel::OnLeftDown)
    EVT_RIGHT_DOWN(PicPanel::OnRightDown)
    EVT_LEFT_DCLICK(PicPanel::OnLeftDoubleClicked)
    EVT_LEFT_UP(PicPanel::OnLeftUp)
    EVT_MOTION(PicPanel::OnMouseMove)
    EVT_MOUSEWHEEL(PicPanel::OnMouseWheel)
	EVT_LEAVE_WINDOW(PicPanel::OnMouseLeave)
    //EVT_ERASE_BACKGROUND(PicPanel::OnEraseBackground)
    EVT_SIZE(PicPanel::OnSize)
    EVT_KEY_DOWN(PicPanel::OnKey)
    //EVT_DROP_FILES(PicPanel::DropFiles)
END_EVENT_TABLE()

	PicPanel::PicPanel(wxFrame *parent, wxTreeCtrl *tree, myHistogramPane *hgram): wxPanel(parent, wxID_ANY, wxDefaultPosition, wxSize(1000,740)) 
	{
		parentframe = parent;
		commandtree = tree;
		histogram = hgram;
		//wxWindow::SetBackgroundStyle(wxBG_STYLE_PAINT);
		SetBackgroundColour(wxColour(64,64,64));  //SetBackgroundColour(*wxBLACK);
		SetDoubleBuffered(true);  //watch this one... tricksy...
		showDebug = true;
		scaleWindow = false;
		blank = true;
		settingpic = false;
		dcList = "";
		oob = 0;

		colormgt = false;
		hTransform = NULL;
		hImgProfile = NULL;

		fitmode=true;
		keyCode = 0;
		moving=false; thumbmoving=false;
		histogramcolor = wxColour(50,50,50);
		picX = 0; picY = 0;
		scale = 1.0;

		pr = pb = pg = -1.0;

		//wxImages:
		thumbimg=NULL;
		scaledimg=NULL;

		//wxBitmaps:
        	pic=NULL;
		thumb=NULL;
		scaledpic=NULL;
		//histogram=NULL;

		histstr="histogram: ";
        }

	PicPanel::~PicPanel()
	{
		if (thumbimg) thumbimg->~wxImage();
		if (scaledimg) scaledimg->~wxImage();
		if (pic) pic->~wxBitmap();
		if (thumb) thumb->~wxBitmap();
		if (scaledpic) scaledpic->~wxBitmap();
		//if (histogram) histogram->~wxBitmap();
	}
        
//        void PicPanel::OnEraseBackground(wxEraseEvent& event) {};
        
        void PicPanel::OnSize(wxSizeEvent& event) 
        {
		Refresh();
		event.Skip();
        }
        
        void PicPanel::drawBox(wxDC &dc, int x, int y, int w,int h)
        {
            dc.DrawLine(x, y, x+w, y);
            dc.DrawLine(x+w, y, x+w, y+h);
            dc.DrawLine(x+w, y+h, x, y+h);
            dc.DrawLine(x, y+h, x,y);
        }

	void PicPanel::SetThumbMode(int mode)
	{
		toggleThumb = mode;
		Refresh();
		
	}

	void PicPanel::ToggleThumb()
	{
		toggleThumb++;
		if (toggleThumb>3) toggleThumb = 1;
		if (toggleThumb==2) toggleThumb = 3;
		Refresh();
		
	}
	
	void PicPanel::BlankPic()
	{
		blank = true;
		Refresh();
		
	}

	void PicPanel::RefreshPic()
	{
		SetPic(d, ch);
	}

	void PicPanel::SetPic(gImage * dib, GIMAGE_CHANNEL channel)
	{
		cmsHPROFILE hDisplayProfile;

		settingpic = true;
		
		//parm display.status: Write display... in status when setting the display image, 0|1.  Default=1
		if (myConfig::getConfig().getValueOrDefault("display.status","1") == "1")
			parentframe->SetStatusText("display...");
		//mark();
		
		d = dib;
		ch = channel;
		int w, h;
		GetSize(&w, &h);
		img.Destroy();

		int hist[256];
		std::vector<int> hgram;
		int hmax;

		if (thumbimg) thumbimg->~wxImage();
		if (scaledimg) scaledimg->~wxImage();
		if (pic) pic->~wxBitmap();

		//parm display.outofbound: Enable/disable out-of-bound pixel marking, 0|1.  In display pane 'o' toggles between no oob, average of channels, and at least one channel.  Default=0
		if (myConfig::getConfig().getValueOrDefault("display.outofbound","0") == "1")
			img = gImage2wxImage(*dib, oob);
		else
			img = gImage2wxImage(*dib);
		
		int rotation = atoi(dib->getInfoValue("Orientation").c_str());
		if (rotation == 3) img = img.Rotate180();
		if (rotation == 5) img = img.Rotate90(false);
		if (rotation == 6) img = img.Rotate90(true);
		if (rotation == 8) img = img.Rotate90(false);

		if (colormgt) {

			profilepath.AssignDir(wxString(myConfig::getConfig().getValueOrDefault("cms.profilepath","").c_str()));

			if (dib->getProfile() != NULL & dib->getProfileLength() > 0) {
				cmsHPROFILE hImgProf = cmsOpenProfileFromMem(dib->getProfile(), dib->getProfileLength());
				if (hImgProf) {
					if (hImgProfile) cmsCloseProfile(hImgProfile);
					hImgProfile = hImgProf;
				}
				else {
					wxMessageBox(wxString::Format("Image profile not found, disabling color management"));
					SetColorManagement(false);
					hImgProfile = NULL;
				}
			}

			//Get display profile:
			//parm display.cms.displayprofile: If color management is enabled, sets the ICC profile used for rendering the display image. Is either a path/filename, or one of the internal profiles.  This parameter is read every time the display is updated, so it can be changed in mid-edit.  Default=srgb
			profilepath.SetFullName(wxString(myConfig::getConfig().getValueOrDefault("display.cms.displayprofile","").c_str()));
			if (myConfig::getConfig().getValueOrDefault("display.cms.displayprofile","") == "") {
				if (myConfig::getConfig().getValueOrDefault("display.cms.requireprofile","1") == "1") {
					wxMessageBox("No display profile specified, and is required. Disabling color management");
					SetColorManagement(false);
				}
				hDisplayProfile = NULL;
			}
			else {
				if (profilepath.IsOk() & profilepath.FileExists()) {
					hDisplayProfile = cmsOpenProfileFromFile(profilepath.GetFullPath().c_str(), "r");
				}
				else {
					wxMessageBox(wxString::Format("Display profile %s not found, disabling color management", profilepath.GetFullName()));
					SetColorManagement(false);
					hDisplayProfile = NULL;
				}
			}

			if (hTransform) cmsDeleteTransform(hTransform);
			
			//parm display.cms.renderingintent: Specify the rendering intent for the display transform, perceptual|saturation|relative_colorimetric|absolute_colorimetric.  Default=perceptual
			wxString intentstr = wxString(myConfig::getConfig().getValueOrDefault("display.cms.renderingintent","perceptual"));
			cmsUInt32Number intent = INTENT_PERCEPTUAL;
			if (intentstr == "perceptual") intent = INTENT_PERCEPTUAL;
			if (intentstr == "saturation") intent = INTENT_SATURATION;
			if (intentstr == "relative_colorimetric") intent = INTENT_RELATIVE_COLORIMETRIC;
			if (intentstr == "absolute_colorimetric") intent = INTENT_ABSOLUTE_COLORIMETRIC;

			cmsUInt32Number dwflags = 0;
			//parm display.cms.blackpointcompensation: Perform display color transform with black point compensation.  Default=1  
			if (myConfig::getConfig().getValueOrDefault("display.cms.blackpointcompensation","1") == "1") dwflags = dwflags | cmsFLAGS_BLACKPOINTCOMPENSATION;
			if (hImgProfile)
				if (hDisplayProfile)
					hTransform = cmsCreateTransform(
						hImgProfile, TYPE_RGB_8,
						hDisplayProfile, TYPE_RGB_8,
						intent, dwflags);
						
			if (!hTransform) {
				wxMessageBox(wxString::Format("Display transform creation failed, disabling color management"));
				SetColorManagement(false);
				hDisplayProfile = NULL;
			}

			//cmsCloseProfile(hImgProfile);  //Now done from rawprocFrm with a method call...
			cmsCloseProfile(hDisplayProfile);
		}
//		else {
//			wxMessageBox("bad image profile, disabling color management");
//			SetColorManagement(false);
//		}

		aspectW = (float) img.GetWidth() / (float) img.GetHeight();
		aspectH = (float) img.GetHeight() / (float) img.GetWidth();

		//parm display.cms.transform=set|render: Do display color profile transform at image set, or at render.  Trade is load time vs image pan smoothness.  Default=set
		wxString cmstransform = wxString(myConfig::getConfig().getValueOrDefault("display.cms.transform","set"));

		if (colormgt)
			if (cmstransform == "set")
				if (hImgProfile) 
					if (hTransform) {
						//cmsDoTransform(hTransform, img.GetData(), img.GetData(), img.GetWidth()*img.GetHeight());
						unsigned char* im = img.GetData();
						unsigned w = img.GetWidth();
						unsigned h = img.GetHeight();
						#pragma omp parallel for
						for (unsigned y=0; y<h; y++) {
							unsigned pos = y*w*3;
							cmsDoTransform(hTransform, &im[pos], &im[pos], w);
						}
					}
			
		
		//generate and store a thumbnail bitmap:
		thumbW = 100*aspectW;
		thumbH = 100;
		wxImage thumbimg = img.Scale(thumbW,thumbH, wxIMAGE_QUALITY_HIGH);
		
		if (colormgt)
			if (cmstransform != "set")   //meaning, don't do it twice, if 'set'...
				if (hImgProfile) 
					if (hTransform) 
						cmsDoTransform(hTransform, thumbimg.GetData(), thumbimg.GetData(), thumbW*thumbH);

		//parm histogram.scale: The number of buckets to display in the histogram. Default=256
		unsigned scale = atoi(myConfig::getConfig().getValueOrDefault("histogram.scale","256").c_str());
		
		histogram->SetPic(*dib, scale);
		histogram->SetChannel(channel);

		if (thumb) thumb->~wxBitmap();
		thumb = new wxBitmap(thumbimg);

		hsgram = wxBitmap();
		settingpic =  false;
		blank = false;

		parentframe->SetStatusText("");
		//parentframe->SetStatusText(wxString::Format("disp: %s",duration().c_str()));
		Refresh();
		

		
	}

	void PicPanel::render(wxDC &dc)
	{
		//if (blank) return;
		int w, h;
		int tw, th;
		int iw, ih;
		wxImage spic, sspic;
		
		if (blank) {
			dc.Clear();
			return;
		}

		GetSize(&w, &h);
            
		if (fitmode) {
			if (img.GetWidth() > img.GetHeight())
				scale = (double) w/ (double) img.GetWidth();
			else
				scale = (double) h/ (double) img.GetHeight();
		}

		dc.Clear();
		 if (!img.IsOk()) return;
		dc.SetClippingRegion(0,0,w,h);
                
                
		iw = img.GetWidth()*scale;
		ih = img.GetHeight()*scale;
   
		if (iw < w) {
			picX = (float) w/2 - (float) iw/2;
		}
		else {
			if (picX < -(iw-w))
				picX = w-iw;
			else if (picX > 0)
				picX = 0;
		}

		if (ih < h) {
			picY = (float) h/2 - (float) ih/2;
		}
		else {
			if (picY < -(ih-h))
				picY = h-ih;\
			else if (picY > 0)
				picY = 0;
		}
                
		if (scale == 1.0)
			spic = img.Copy();
		//else if (scale <= .5)
		//	spic = img.Scale(iw, ih, wxIMAGE_QUALITY_HIGH);
		else
			spic = img.Scale(iw, ih); //, wxIMAGE_QUALITY_HIGH);

		wxString cmstransform = wxString(myConfig::getConfig().getValueOrDefault("display.cms.transform","set"));

		if (colormgt)
			if (cmstransform == "render") {
						//cmsDoTransform(hTransform, spic.GetData(), spic.GetData(), iw*ih);
						unsigned char* im = spic.GetData();
						unsigned sw = spic.GetWidth();
						unsigned sh = spic.GetHeight();
						#pragma omp parallel for
						for (unsigned y=0; y<sh; y++) {
							unsigned pos = y*sw*3;
							cmsDoTransform(hTransform, &im[pos], &im[pos], sw);
						}
			}
    
		if (scaledpic) scaledpic->~wxBitmap();
		scaledpic = new wxBitmap(spic);
		dc.SetPen(wxPen(wxColour(255,255,255),1));
		dc.SetBrush(wxBrush(wxColour(50,50,50)));
		dc.DrawBitmap(*scaledpic, picX, picY, false);

		if (dcList != "" & scale == 1.0) {
			dc.SetPen(*wxYELLOW_PEN);
			wxArrayString l = split(dcList, ";");
			for (unsigned i=0; i<l.GetCount(); i++) {
				wxArrayString c = split(l[i],",");
				if (c[0] == "cross") {
					if (c.GetCount() < 3) continue;
					int px = atoi(c[1].c_str())+picX;
					int py = atoi(c[2].c_str())+picY;
					dc.DrawLine(px-10, py, px+10, py);
					dc.DrawLine(px, py-10, px, py+10);
				}
			}
		}

		if (thumb) {
			if (toggleThumb != 3) {
				dc.SetPen(wxPen(wxColour(0,0,0),1));
				dc.DrawRectangle(0,0,thumb->GetWidth()+4, thumb->GetHeight()+4);			
				dc.SetPen(wxPen(wxColour(255,255,255),1));
				dc.DrawRectangle(1,1,thumb->GetWidth()+2, thumb->GetHeight()+2);
			}
			if (toggleThumb == 1) dc.DrawBitmap(*thumb,2,2,false);
		
			//wxSize hs = histogram->GetSize();
			//histogram->SetBitmap(ThreadedHistogramFrom(img, hs.GetWidth(), hs.GetHeight()));
		
			//keep only to debug myHistogramPane...
			//if (toggleThumb == 2) {
			//	if (!hsgram.IsOk()) {
			//		hsgram = ThreadedHistogramFrom(img, thumb->GetWidth(), thumb->GetHeight());
			//		//hsgram = ThreadedHistogramFrom(spic, thumb->GetWidth(), thumb->GetHeight());
			//	}
			//	dc.DrawBitmap(hsgram,2,2,false);	
			//}
		}

		//draw crop rectangle in thumbnail:
		tw = thumbW * ((float) w/ (float) iw);
		th = thumbH * ((float) h/(float) ih);
		dc.SetClippingRegion(0,0,thumbW,thumbH);
		if (iw>w | ih>h) {
			dc.SetPen(wxPen(wxColour(255,255,255),1));
			if (toggleThumb == 1) drawBox(dc, (-picX * ((float) thumbW/ (float) iw)), (-picY * ((float) thumbH/(float) ih)),tw,th);
		}
		
	}
	
	void PicPanel::SetDrawList(wxString list)
	{
		dcList = list;
		Refresh();
		
	}
   

	void PicPanel::SetColorManagement(bool b)
	{
		colormgt = b;
		if (colormgt)
			parentframe->SetStatusText("CMS",1);
		else
			parentframe->SetStatusText("",1);
	}

	bool PicPanel::GetColorManagement()
	{
		return colormgt;
	}


	void PicPanel::SetProfile(gImage * dib)
	{
		cmsHPROFILE hImgProf;
		if (dib->getProfile() != NULL & dib->getProfileLength() > 0) {
			hImgProf = cmsOpenProfileFromMem(dib->getProfile(), dib->getProfileLength());
			SetImageProfile(hImgProf);
		}
	}

	void PicPanel::SetImageProfile(cmsHPROFILE hImgProf)
	{
		if (hImgProfile) cmsCloseProfile(hImgProfile);
		hImgProfile = hImgProf;
	}

	cmsHTRANSFORM PicPanel::GetDisplayTransform()
	{
		return hTransform;
	}

	wxString PicPanel::getHistogramString()
	{
		return histstr;
	}
        

	void PicPanel::SetScaleToWidth()
	{
		int w, h;
		GetSize(&w, &h);
		if (img.IsOk()) {
			scale = (double) w/ (double) img.GetWidth();
			Refresh();
			
		}
	}
	
	void PicPanel::SetScaleToHeight()
	{
		int w, h;
		GetSize(&w, &h);
		if (img.IsOk()) {
			scale = (double) h/ (double) img.GetHeight();
			Refresh();
			
		}
	}

	void PicPanel::SetScaleToWidth(double percentofwidth)
	{
		int w, h;
		GetSize(&w, &h);
		if (img.IsOk()) {
			scale = ((double) w/ ((double) img.GetWidth()) * percentofwidth);
			Refresh();
			
		}
	}

	void PicPanel::SetScale(double s)
	{
		scale = s;
		FitMode(false);
		parentframe->SetStatusText(wxString::Format("scale: %0.0f\%",scale*100.0),2);
		Refresh();
	}

	void PicPanel::FitMode(bool f)
	{
		fitmode = f;
	}


	double PicPanel::GetScale()
	{
		return scale;
	}
	
	coord PicPanel::GetImgCoords()
	{
		coord c; 
		c.x = imgX; c.y = imgY;
		return c;
	}
	
	void PicPanel::PaintNow()
	{
		wxClientDC dc(this);
		render(dc);
		/*
		wxTreeItemId item = commandtree->GetSelection();
		if (item.IsOk()) {
			PicProcessor * i = ((PicProcessor *) commandtree->GetItemData(item));
			if (i) { 
				parentframe->SetStatusText("going to displayDraw()...");
				i->displayDraw(dc);
			}
		}
		*/
	}

	void PicPanel::OnPaint(wxPaintEvent & event)
	{
		event.Skip();
		if (blank | settingpic ) return;
		if (img.IsOk() && thumb != NULL) {
			wxPaintDC dc(this);
			render(dc);
		}
	}

	void PicPanel::OnLeftDown(wxMouseEvent& event)
	{
		event.Skip();
		if (blank) return;
		SetFocus();
		int radius = 20;
		MouseX = event.m_x;
		MouseY = event.m_y;

		if (toggleThumb != 2)
			if (MouseX < thumbW & MouseY < thumbH)
				thumbmoving = true;
			else
				moving=true;
		else
			moving=true;
	}
        
	void PicPanel::OnRightDown(wxMouseEvent& event)
	{
		event.Skip();
		if (blank) return;
		picX = 0; picY = 0;
		Refresh();
		
	}

	void PicPanel::OnLeftUp(wxMouseEvent& event)
	{
		event.Skip();
		if (blank) return;
		if (moving | thumbmoving) {
			moving=false;
			thumbmoving=false;
		}
	}

void PicPanel::OnMouseMove(wxMouseEvent& event)
{
	event.Skip();
	if (blank) return;
	int x, y, posx, posy;
	int iw, ih;
	int dx, dy;
	
	if (img.IsOk()){
		iw = img.GetWidth()*scale;
		ih = img.GetHeight()*scale;
            
		GetPosition(&posx, &posy);
		x=event.m_x; y=event.m_y;
		dx = MouseX-x;
		dy = MouseY-y;


		if (moving) {
			picX -= MouseX-x; 
			picY -= MouseY-y;
			MouseX = x; MouseY = y;
			Refresh();
			
		}

		if (thumbmoving) {
			picX += (MouseX-x) * round((float) iw / (float) thumbW);
			picY += (MouseY-y) * round((float) ih / (float) thumbH);
			MouseX = x; MouseY = y;
			Refresh();
			
		}

		pr = pb = pg = -1.0;

		if (scale == 1.0 & scaledpic != NULL) {
			imgX = x-picX;
			imgY = y-picY;
			if (d) {
				std::vector<float> p = d->getPixelArray(imgX, imgY);
				if (imgX > 1) {
					if (imgY > 1) {
						if (imgX < scaledpic->GetWidth()) {
							if (imgY < scaledpic->GetHeight()) { 
								pr = p[0]; pg = p[1]; pb = p[2];
								//parm display.rgb.scale: Multiplier for rgb display in status line.  Default=1
								int pscale = atoi(myConfig::getConfig().getValueOrDefault("display.rgb.scale","1").c_str());
								if (pscale > 1)
									parentframe->SetStatusText(wxString::Format("xy: %d,%d\trgb: %0.0f,%0.0f,%0.0f", imgX, imgY,  p[0]*pscale, p[1]*pscale, p[2]*pscale)); 
								else
									parentframe->SetStatusText(wxString::Format("xy: %d,%d\trgb: %f,%f,%f", imgX, imgY,  p[0]*pscale, p[1]*pscale, p[2]*pscale)); 									
							} else parentframe->SetStatusText("");
						} else parentframe->SetStatusText("");
					} else parentframe->SetStatusText("");
				} else parentframe->SetStatusText("");
			}
		}
	}
}

void PicPanel::OnMouseLeave(wxMouseEvent& event)
{
	event.Skip();
	parentframe->SetStatusText("");
}
        
void PicPanel::OnMouseWheel(wxMouseEvent& event)
{
	event.Skip();
	if (blank) return;
	double increment = 0.05;
	
	int iw = img.GetWidth();
	int ih = img.GetHeight();
	
	fitmode=false;
	wxImage scaledimg;
	if (event.GetWheelRotation() > 0)
		scale += increment;
	else
		scale -= increment;
	if (scale < 0.1) 
		scale = 0.1;
	else if (scale > 2) 
		scale = 2; 
	else {
		if (event.GetWheelRotation() > 0) { 
			picX += picX * 0.1;
			picY += picY * 0.1;
		}
		else {
			picX -= picX * 0.1;
			picY -= picY * 0.1;
		}
	}
	parentframe->SetStatusText(wxString::Format("scale: %.0f%", scale*100),2);
	parentframe->SetStatusText("");
	Refresh();
	
}

void PicPanel::OnLeftDoubleClicked(wxMouseEvent& event)
{
	event.Skip();
	if (blank) return;
	MouseX = event.m_x;
	MouseY = event.m_y;
	
	int iw = img.GetWidth();
	int ih = img.GetHeight();

	if (MouseX < thumbW & MouseY < thumbH) {
		ToggleThumb();
	}
	else {
		if (scale != 1.0) {
			picX = -(iw * ((MouseX-picX)/(iw*scale)) - ((iw*scale)/2));
			picY = -(ih * ((MouseY-picY)/(ih*scale)) - ((ih*scale)/2));
			scale = 1.0;
			FitMode(false);
			parentframe->SetStatusText("scale: 100%",2);
		}
		else {
			SetScaleToWidth();
			FitMode(true);
			parentframe->SetStatusText("scale: fit",2);
			parentframe->SetStatusText("");
		}
	}
	Refresh();
	
}


void PicPanel::OnKey(wxKeyEvent& event)
{
	event.Skip();
	if (blank) return;
	//parentframe->SetStatusText(wxString::Format("PicPanel: keycode=%d", event.GetKeyCode()));
	switch (event.GetKeyCode()) {
		case 116: //t
		case 84: //T - toggle display thumbnail
			ToggleThumb();
			break;
		case 67: //c - with Ctrl-, copy RGB at the x,y
			if (event.ControlDown()) 
				if (pr > -1.0)
					if (wxTheClipboard->Open()) {
						// This data objects are held by the clipboard,
						// so do not delete them in the app.
						wxTheClipboard->SetData( new wxTextDataObject(wxString::Format("%f,%f,%f", pr, pg, pb)) );
						wxTheClipboard->Close();
					}
		case 79: //o oob toggle
			if (myConfig::getConfig().getValueOrDefault("display.outofbound","0") == "1") {
				oob++;
				if (oob > 2) oob = 0;
				SetPic(d, ch);
				Refresh();
			}
			break;
	}
}


