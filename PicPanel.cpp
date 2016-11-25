
#include "PicPanel.h"
#include "util.h"
#include <vector>
#include <wx/fileconf.h>


BEGIN_EVENT_TABLE(PicPanel, wxPanel)
    EVT_PAINT(PicPanel::OnPaint)
    EVT_LEFT_DOWN(PicPanel::OnLeftDown)
    EVT_RIGHT_DOWN(PicPanel::OnRightDown)
    EVT_LEFT_DCLICK(PicPanel::OnLeftDoubleClicked)
    EVT_LEFT_UP(PicPanel::OnLeftUp)
    EVT_MOTION(PicPanel::OnMouseMove)
    EVT_MOUSEWHEEL(PicPanel::OnMouseWheel)
    EVT_ERASE_BACKGROUND(PicPanel::OnEraseBackground)
    EVT_SIZE(PicPanel::OnSize)
    EVT_CHAR(PicPanel::OnKey)
    //EVT_DROP_FILES(PicPanel::DropFiles)
END_EVENT_TABLE()

        PicPanel::PicPanel(wxFrame *parent): wxPanel(parent) {
		parentframe = parent;
		wxWindow::SetBackgroundStyle(wxBG_STYLE_PAINT);
		SetBackgroundColour(wxColour(64,64,64));  //SetBackgroundColour(*wxBLACK);
		wxInitAllImageHandlers();
		SetDoubleBuffered(true);  //watch this one... tricksy...
		showDebug = true;
		scaleWindow = false;
		cropmode = false;
		fitmode=true;
		keyCode = 0;
		//tick = 0;
		moving=false; thumbmoving=false;
		histogramcolor = wxColour(50,50,50);
		picX = 0; picY = 0;
		scale = 1.0;
		cropX = 0;
		cropY = 0;
		cropW = 0;
		cropH = 0;
		cropnode = 0; //1=topleft, 3=bottomright, 5=in cropbox
		cropratio = 1; //0=none; 1=maintain aspect

		//wxImages:
		thumbimg=NULL;
		scaledimg=NULL;

		//wxBitmaps:
        	pic=NULL;
		thumb=NULL;
		scaledpic=NULL;
		histogram=NULL;

		histstr="histogram: ";
        }

	PicPanel::~PicPanel()
	{
		if (thumbimg) thumbimg->~wxImage();
		if (scaledimg) scaledimg->~wxImage();
		if (pic) pic->~wxBitmap();
		if (thumb) thumb->~wxBitmap();
		if (scaledpic) scaledpic->~wxBitmap();
		if (histogram) histogram->~wxBitmap();
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
		if (!cropmode) {
			toggleThumb++;
			if (toggleThumb>3) toggleThumb = 1;
			Refresh();
			Update();
		}
		//parentframe->SetStatusText(wxString::Format("thumbmode: %d", toggleThumb));
	}

	void PicPanel::SetPic(gImage * dib)
	{
		//parentframe->SetStatusText("display...");
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
		if (histogram) histogram->~wxBitmap();

		img = gImage2wxImage(*dib);

		cropX = 0;
		cropY = 0;
		cropW = img.GetWidth();
		cropH = img.GetHeight();
		cropnode = 0;

                aspectW = (float) img.GetWidth() / (float) img.GetHeight();
                aspectH = (float) img.GetHeight() / (float) img.GetWidth();
                
                //generate and store a thumbnail bitmap:
                thumbW = 100*aspectW;
                thumbH = 100;
                wxImage thumbimg = img.Scale(thumbW,thumbH, wxIMAGE_QUALITY_HIGH);
                //if (thumb) thumb->~wxBitmap();
                thumb = new wxBitmap(thumbimg);

		//hsgram = HistogramFrom(img, thumbW, thumbH);
		////hsgram = HistogramFromVec(histdata, hmax, thumbW, thumbH);
		hsgram = wxBitmap();

		//parentframe->SetStatusText("");
                Refresh();
		Update();
		
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
		parentframe->SetStatusText(wxString::Format("scale: %0.0f\%",scale*100.0),1);
		Refresh();
		Update();
	}

	void PicPanel::ToggleCropMode()
	{
		cropmode = !cropmode;
		if (cropmode) {
			toggleThumb = 3;
			cropnode = 0;
		}
		Refresh();
		Update();
		if (cropmode) 
			parentframe->SetStatusText("cropmode on");
		else
			parentframe->SetStatusText("cropmode off");
	}

	void PicPanel::CropMode(bool c)
	{
		cropmode = c;
		if (cropmode) {
			toggleThumb = 3;
			cropnode = 0;
		}
		Refresh();
		Update();
	}

	//void PicPanel::CropProcessor(PicProcessorCrop *c)
	//{
	//	cropproc = c;
	//}

	void PicPanel::SetCropRatio(int r)
	{
		cropratio = r;
	}

	wxString PicPanel::GetCropParams()
	{
		return wxString::Format("%d,%d,%d,%d", cropX, cropY, cropW, cropH);
	}

	void PicPanel::SetCropParams(wxString params)
	{
		wxArrayString cp = split(params,",");
		cropX = atoi(cp[0]);
		cropY = atoi(cp[1]);
		cropW = atoi(cp[2]);
		cropH = atoi(cp[3]);
	}

	void PicPanel::FitMode(bool f)
	{
		fitmode = f;
	}


	double PicPanel::GetScale()
	{
		return scale;
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

      
	void PicPanel::render(wxDC &dc)
	{
		int w, h;
		int tw, th;
		int iw, ih;
		wxImage spic;
            
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
		//else if (scale <= .5)
		//	spic = img->Scale(iw, ih, wxIMAGE_QUALITY_HIGH);
		else
			spic = img.Scale(iw, ih); //, wxIMAGE_QUALITY_HIGH);
    
		if (scaledpic) scaledpic->~wxBitmap();
		scaledpic = new wxBitmap(spic);
		dc.SetPen(wxPen(wxColour(255,255,255),1));
		dc.SetBrush(wxBrush(wxColour(50,50,50)));
		//dc.DrawRectangle(0,h-20,160,h);
		dc.DrawBitmap(*scaledpic, picX, picY, false);
		if (toggleThumb != 3) {
			dc.SetPen(wxPen(wxColour(0,0,0),1));
			dc.DrawRectangle(0,0,thumb->GetWidth()+4, thumb->GetHeight()+4);			
			dc.SetPen(wxPen(wxColour(255,255,255),1));
			dc.DrawRectangle(1,1,thumb->GetWidth()+2, thumb->GetHeight()+2);
		}
		if (toggleThumb == 1) dc.DrawBitmap(*thumb,2,2,false);
		if (toggleThumb == 2) {
			if (!hsgram.IsOk()) {
				hsgram = ThreadedHistogramFrom(img, thumb->GetWidth(), thumb->GetHeight());
			}
			dc.DrawBitmap(hsgram,2,2,false);
		}

		//draw crop rectangle:
		if (cropmode) {
			dc.SetPen(*wxYELLOW_PEN);
			dc.SetBrush(*wxYELLOW_BRUSH);
			drawBox(dc,picX+cropX*scale, picY+cropY*scale, cropW*scale, cropH*scale);
			dc.DrawCircle(picX+cropX*scale, picY+cropY*scale, 10);
			dc.DrawCircle(picX+cropX*scale+cropW*scale, picY+cropY*scale+cropH*scale, 10);
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
        
        
        
	void PicPanel::OnLeftDown(wxMouseEvent& event)
	{
		SetFocus();
		int radius = 20;
		MouseX = event.m_x;
		MouseY = event.m_y;
		if (cropmode) {
			// x/y:
			if ((MouseX > picX-radius+cropX*scale) & (MouseX < picX+radius+cropX*scale)) {
				if ((MouseY > picY-radius+cropY*scale) & (MouseY < picY+radius+cropY*scale)) {
					cropnode = 1;
				}
			} 
			// w/h:
			else if ((MouseX > picX-radius+cropX*scale+cropW*scale) & (MouseX < picX+radius+cropX*scale+cropW*scale)) {
				if ((MouseY > picY-radius+cropY*scale+cropH*scale) & (MouseY < picY+radius+cropY*scale+cropH*scale)) {
					cropnode = 3;
				}
			}
			// move:
			else if ((MouseX > picX+cropX*scale) & (MouseX < picX+cropX*scale+cropW*scale)) {
				if ((MouseY > picY+cropY*scale) & (MouseY < picY+cropY*scale+cropH*scale)) {
					cropnode = 5;
				}
			}
		}
		if (toggleThumb != 2)
			if (MouseX < thumbW & MouseY < thumbH)
				thumbmoving = true;
			else
				moving=true;
		else
			moving=true;
		//event.Skip();
	}
        
        void PicPanel::OnRightDown(wxMouseEvent& event)
        {
            picX = 0; picY = 0;
            PaintNow();
	    //event.Skip();
        }

        void PicPanel::OnLeftUp(wxMouseEvent& event)
        {
		if (moving | thumbmoving) {
			moving=false;
			thumbmoving=false;
		}
		if (cropnode != 0) {
			cropnode = 0;
			wxCommandEvent *e = new wxCommandEvent(wxEVT_SCROLL_THUMBRELEASE);
			e->SetString(wxString::Format("%d,%d,%d,%d", cropX, cropY, cropW, cropH));
			wxQueueEvent(this,e);
		}
		//event.Skip();
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

		//todo: cropratio behavior (1=maintain aspect)
		if (cropmode) {
			switch (cropnode) {
				case 1: //x/y:
					cropX -= (MouseX-x)/scale;
					cropY -= (MouseY-y)/scale;
					if (cropX < 0) 
						cropX = 0;
					else
						cropW += (MouseX-x)/scale;
					if (cropY < 0) 
						cropY = 0;
					else
						cropH += (MouseY-y)/scale;
					if (cropratio==1) {
						if (anchorx) {
							cropH = cropW * aspectH;
						}
						else {
							cropW = cropH * aspectW;
						}
						if (cropX+cropW > img.GetWidth())  cropX = img.GetWidth()-cropW;
						if (cropY+cropH > img.GetHeight()) cropY = img.GetHeight()-cropH;
					}
					MouseX = x; MouseY = y;
					Refresh();
					Update();
					//PaintNow();
					break;
				case 3: //w/h:
					cropW -= (MouseX-x)/scale;
					cropH -= (MouseY-y)/scale;
					if (cropW > img.GetWidth()) cropW = img.GetWidth();
					if (cropH > img.GetHeight()) cropH = img.GetHeight(); 
					if (cropratio==1) {
						if (anchorx) 
							cropH = cropW * aspectH;
						else
							cropW = cropH * aspectW;
					}
					if (cropX+cropW > img.GetWidth())  cropX = img.GetWidth()-cropW;
					if (cropY+cropH > img.GetHeight()) cropY = img.GetHeight()-cropH;
					MouseX = x; MouseY = y;
					Refresh();
					Update();
					//PaintNow();
					break;
				case 5:  //move:
					cropX -= (MouseX-x)/scale;
					cropY -= (MouseY-y)/scale;
					if (cropX < 0) cropX = 0;
					if (cropY < 0) cropY = 0;
					if (cropX+cropW > img.GetWidth())  cropX = img.GetWidth()-cropW;
					if (cropY+cropH > img.GetHeight()) cropY = img.GetHeight()-cropH;
					MouseX = x; MouseY = y;
					Refresh();
					Update();
					//PaintNow();
					break;
			}
			parentframe->SetStatusText("");
			//parentframe->SetStatusText(wxString::Format("%d,%d,%d,%d    %d,%d", cropX, cropY, cropW, cropH, iw, ih));
		}
		else {
                	if (moving) {
				picX -= MouseX-x; 
				picY -= MouseY-y;
				MouseX = x; MouseY = y;
				Refresh();
				Update();
				//PaintNow();
                	}
                	if (thumbmoving) {
				picX += (MouseX-x) * ((float) iw / (float) thumbW);
				picY += (MouseY-y) * ((float) ih / (float) thumbH);
				MouseX = x; MouseY = y;
				Refresh();
				Update();
				//PaintNow();
                	}
		}
		if (scale == 1.0) {
			unsigned px = x-picX;
			unsigned py = y-picY;
			if (px > 1) {
				if (py > 1) {
					if (px < scaledpic->GetWidth()) {
						if (py < scaledpic->GetHeight()) {
							parentframe->SetStatusText(wxString::Format("xy: %d,%d\trgb: %d,%d,%d", px, py, (unsigned) img.GetRed(px,py), (unsigned) img.GetGreen(px,py), (unsigned) img.GetBlue(px,py))); 
						} else parentframe->SetStatusText("");
					} else parentframe->SetStatusText("");
				} else parentframe->SetStatusText("");
			} else parentframe->SetStatusText("");
		}
            }
	    //event.Skip();
        }
        
        void PicPanel::OnMouseWheel(wxMouseEvent& event)
        {
		double increment = 0.05;
 
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
		parentframe->SetStatusText(wxString::Format("scale: %.0f%", scale*100),1);
		parentframe->SetStatusText("");
		Refresh();
		Update();
        }

void PicPanel::OnLeftDoubleClicked(wxMouseEvent& event)
{
	MouseX = event.m_x;
	MouseY = event.m_y;
	if (MouseX < thumbW & MouseY < thumbH) {
		ToggleThumb();
	}
	else {
		if (scale != 1.0) {
			scale = 1.0;
			FitMode(false);
			parentframe->SetStatusText("scale: 100%",1);
		}
		else {
			SetScaleToWidth();
			FitMode(true);
			parentframe->SetStatusText("scale: fit",1);
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
        
        
