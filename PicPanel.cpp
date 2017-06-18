
#include "PicPanel.h"
#include "util.h"
#include <vector>
#include <wx/fileconf.h>
#include "PicProcessor.h"


BEGIN_EVENT_TABLE(PicPanel, wxPanel)
    EVT_PAINT(PicPanel::OnPaint)
    EVT_LEFT_DOWN(PicPanel::OnLeftDown)
    EVT_RIGHT_DOWN(PicPanel::OnRightDown)
    EVT_LEFT_DCLICK(PicPanel::OnLeftDoubleClicked)
    EVT_LEFT_UP(PicPanel::OnLeftUp)
    EVT_MOTION(PicPanel::OnMouseMove)
    EVT_MOUSEWHEEL(PicPanel::OnMouseWheel)
    //EVT_ERASE_BACKGROUND(PicPanel::OnEraseBackground)
    EVT_SIZE(PicPanel::OnSize)
    EVT_CHAR(PicPanel::OnKey)
    //EVT_DROP_FILES(PicPanel::DropFiles)
END_EVENT_TABLE()

	PicPanel::PicPanel(wxFrame *parent, wxTreeCtrl *tree, myHistogramPane *hgram): wxPanel(parent) {
		parentframe = parent;
		commandtree = tree;
		histogram = hgram;
		//wxWindow::SetBackgroundStyle(wxBG_STYLE_PAINT);
		SetBackgroundColour(wxColour(64,64,64));  //SetBackgroundColour(*wxBLACK);
		wxInitAllImageHandlers();
		SetDoubleBuffered(true);  //watch this one... tricksy...
		showDebug = true;
		scaleWindow = false;
		blank = true;
		dcList = "";

		colormgt = false;
		hTransform = NULL;
		hImgProfile = NULL;

		fitmode=true;
		keyCode = 0;
		moving=false; thumbmoving=false;
		histogramcolor = wxColour(50,50,50);
		picX = 0; picY = 0;
		scale = 1.0;

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
        
        void PicPanel::OnEraseBackground(wxEraseEvent& event) {};
        
        void PicPanel::OnSize(wxSizeEvent& event) 
        {
            Refresh();
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
		Update();
	}

	void PicPanel::ToggleThumb()
	{
		toggleThumb++;
		if (toggleThumb>3) toggleThumb = 1;
		if (toggleThumb==2) toggleThumb = 3;
		Refresh();
		Update();
	}
	
	void PicPanel::BlankPic()
	{
		blank = true;
		Refresh();
		Update();
	}

	void PicPanel::SetPic(gImage * dib)
	{
		cmsHPROFILE hDisplayProfile;
		//parentframe->SetStatusText("display...");
		//mark();
		d = dib;
		int w, h;
		GetSize(&w, &h);
		img.Destroy();

		int hist[256];
		std::vector<int> hgram;
		int hmax;

		if (thumbimg) thumbimg->~wxImage();
		if (scaledimg) scaledimg->~wxImage();
		if (pic) pic->~wxBitmap();
		if (thumb) thumb->~wxBitmap();
		if (scaledpic) scaledpic->~wxBitmap();
		//if (histogram) histogram->~wxBitmap();

		img = gImage2wxImage(*dib);
		
		int rotation = atoi(dib->getInfoValue("Orientation").c_str());
		if (rotation == 3) img.Rotate180();
		if (rotation == 5) img.Rotate90(false);
		if (rotation == 6) img.Rotate90(true);

		if (hImgProfile) {
			//parm display.cms.displayprofile: If color management is enabled, sets the ICC profile used for rendering the display image. Is either a path/filename, or one of the internal profiles.  This parameter is read every time the display is updated, so it can be changed in mid-edit.  Default=srgb
			wxString displayprof = wxConfigBase::Get()->Read("display.cms.displayprofile","srgb");
			//parm display.cms.gamma: If color management is enabled and an internal profile is used, sets the gamma used for generating it. Default=2.4
			float gamma = wxConfigBase::Get()->Read("display.cms.gamma",2.4);
			hDisplayProfile = gImage::makeLCMSProfile(std::string(displayprof.ToAscii()), gamma);
			if (!hDisplayProfile)
				//load the monitor profile:
				hDisplayProfile = cmsOpenProfileFromFile(displayprof.c_str(), "r");

			if (hTransform) cmsDeleteTransform(hTransform);
			
			//parm display.cms.renderingintent: Specify the rendering intent for the display transform, perceptual|saturation|relative_colorimetric|absolute_colorimetric.  Default=perceptual
			wxString intentstr = wxConfigBase::Get()->Read("display.cms.renderingintent","perceptual");
			cmsUInt32Number intent = INTENT_PERCEPTUAL;
			if (intentstr == "perceptual") intent = INTENT_PERCEPTUAL;
			if (intentstr == "saturation") intent = INTENT_SATURATION;
			if (intentstr == "relative_colorimetric") intent = INTENT_RELATIVE_COLORIMETRIC;
			if (intentstr == "absolute_colorimetric") intent = INTENT_ABSOLUTE_COLORIMETRIC;

			if (hImgProfile)
				if (hDisplayProfile)
					hTransform = cmsCreateTransform(
						hImgProfile, TYPE_RGB_8,
						hDisplayProfile, TYPE_RGB_8,
						intent, 0);
				else printf("bad display profile...\n");
			else printf("bad image profile...\n");

			//cmsCloseProfile(hImgProfile);  //Now done from rawprocFrm with a method call...
			cmsCloseProfile(hDisplayProfile);
		}

		aspectW = (float) img.GetWidth() / (float) img.GetHeight();
		aspectH = (float) img.GetHeight() / (float) img.GetWidth();

		//if (hImgProfile) 
		//	if (hTransform) 
		//		cmsDoTransform(hTransform, img.GetData(), img.GetData(), img.GetWidth()*img.GetHeight());
              
		//generate and store a thumbnail bitmap:
		thumbW = 100*aspectW;
		thumbH = 100;
		wxImage thumbimg = img.Scale(thumbW,thumbH, wxIMAGE_QUALITY_HIGH);
		
		if (hImgProfile) 
			if (hTransform) 
				cmsDoTransform(hTransform, thumbimg.GetData(), thumbimg.GetData(), thumbW*thumbH);

		//parm histogram.scale: The number of buckets to display in the histogram. Default=256
		unsigned scale = wxConfigBase::Get()->Read("histogram.scale",256);
		histogram->SetPic(*dib, scale);
			
		thumb = new wxBitmap(thumbimg);

		hsgram = wxBitmap();
		blank =  false;

		//parentframe->SetStatusText("");
		//parentframe->SetStatusText(wxString::Format("disp: %s",duration().c_str()));
		Refresh();
		Update();

		
	}

	void PicPanel::render(wxDC &dc)
	{
		int w, h;
		int tw, th;
		int iw, ih;
		wxImage spic, sspic;
		
		if (blank) {
			dc.Clear();
			return;
		}
            
		if (fitmode) SetScaleToWidth();
		GetSize(&w, &h);
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
		else if (scale <= .5)
			spic = img.Scale(iw, ih, wxIMAGE_QUALITY_HIGH);
		else
			spic = img.Scale(iw, ih); //, wxIMAGE_QUALITY_HIGH);

		if (hImgProfile) 
			if (hTransform)
				cmsDoTransform(hTransform, spic.GetData(), spic.GetData(), iw*ih);
    
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
		Update();
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
		if (img.IsOk() && thumb != NULL) {
			wxPaintDC dc(this);
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
	}

	void PicPanel::OnLeftDown(wxMouseEvent& event)
	{
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
		event.Skip();
	}
        
        void PicPanel::OnRightDown(wxMouseEvent& event)
        {
            picX = 0; picY = 0;
            PaintNow();
			event.Skip();
        }

        void PicPanel::OnLeftUp(wxMouseEvent& event)
        {
		if (moving | thumbmoving) {
			moving=false;
			thumbmoving=false;
		}
		event.Skip();
	}

void PicPanel::OnMouseMove(wxMouseEvent& event)
{
	bool anchorx;
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
		if (abs(dx) > abs(dy))
			anchorx = true;  //x
		else 
			anchorx = false;  //y


		if (moving) {
			picX -= MouseX-x; 
			picY -= MouseY-y;
			MouseX = x; MouseY = y;
			Refresh();
			//Update();
			//PaintNow();
		}

		if (thumbmoving) {
			picX += (MouseX-x) * ((float) iw / (float) thumbW);
			picY += (MouseY-y) * ((float) ih / (float) thumbH);
			MouseX = x; MouseY = y;
			Refresh();
			//Update();
			//PaintNow();
		}

		if (scale == 1.0) {
			imgX = x-picX;
			imgY = y-picY;
			if (d) {
				std::vector<PIXTYPE> p = d->getPixelArray(imgX, imgY);
				if (imgX > 1) {
					if (imgY > 1) {
						if (imgX < scaledpic->GetWidth()) {
							if (imgY < scaledpic->GetHeight()) { 
								//parm display.rgb.scale - multiplier for rgb display in status line.  Default=1
								int pscale = wxConfigBase::Get()->Read("display.rgb.scale",1);
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
	event.Skip();
}
        
void PicPanel::OnMouseWheel(wxMouseEvent& event)
{
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
	Update();
}

void PicPanel::OnLeftDoubleClicked(wxMouseEvent& event)
{
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
	Update();
}

void PicPanel::OnKey(wxKeyEvent& event)
{
	//parentframe->SetStatusText(wxString::Format("PicPanel: keycode=%d", event.GetKeyCode()));
	switch (event.GetKeyCode()) {
		case 116: //t
		case 84: //T - toggle display thumbnail
			ToggleThumb();
			break;
	}
	event.Skip();
}
        
        
