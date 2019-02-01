
#include "PicPanel.h"
#include "util.h"
#include <vector>
#include "PicProcessor.h"
#include "myConfig.h"
#include <wx/clipbrd.h>


PicPanel::PicPanel(wxFrame *parent, wxTreeCtrl *tree, myHistogramPane *hgram): wxPanel(parent, wxID_ANY, wxDefaultPosition, wxSize(1000,740)) 
{

	SetDoubleBuffered(true);  //watch this one... tricksy...
	image = NULL;
	scale = 1.0;
	imgctrx = 0.5; imgctry = 0.5;
	mousex = 0; mousey=0;
	dragging = false;

	Bind(wxEVT_SIZE, &PicPanel::OnSize, this);
	Bind(wxEVT_PAINT, &PicPanel::OnPaint,  this);
	Bind(wxEVT_LEFT_DOWN, &PicPanel::OnLeftDown,  this);
	Bind(wxEVT_RIGHT_DOWN, &PicPanel::OnRightDown,  this);
	Bind(wxEVT_LEFT_DCLICK, &PicPanel::OnLeftDoubleClicked,  this);
	Bind(wxEVT_LEFT_UP, &PicPanel::OnLeftUp,  this);
	Bind(wxEVT_MOTION, &PicPanel::OnMouseMove,  this);
	Bind(wxEVT_MOUSEWHEEL, &PicPanel::OnMouseWheel,  this);
	Bind(wxEVT_LEAVE_WINDOW, &PicPanel::OnMouseLeave,  this);
	Bind(wxEVT_KEY_DOWN, &PicPanel::OnKey,  this);
	Bind(wxEVT_TIMER, &PicPanel::OnTimer,  this);
		
	t = new wxTimer(this);
}

PicPanel::~PicPanel()
{
	if (image) image->~wxBitmap();
	if (viewimage) viewimage->~wxBitmap();
	t->~wxTimer();
}
        
void PicPanel::OnSize(wxSizeEvent& event) 
{
	Refresh();
	event.Skip();
}

void PicPanel::SetPic(gImage * dib, GIMAGE_CHANNEL channel)
{
	wxImage img = gImage2wxImage(*dib);
	if (image) image->~wxBitmap();
	image = new wxBitmap(img);
	//ToDo: cmsTransform()...

	Refresh();
}


void PicPanel::render(wxDC &dc)
{
	if (!image) return;
	int panelw, panelh, viewx=0, viewy=0, vieww, viewh, imageposx=0, imageposy=0, imagew, imageh;
	GetSize(&panelw, &panelh);

	if (fit) {
		if (image->GetWidth() > image->GetHeight())
			scale = (double) panelw/ (double) image->GetWidth();
		else
			scale = (double) panelh/ (double) image->GetHeight();
	}

	imagew = (float) image->GetWidth() * scale;
	imageh = (float) image->GetHeight() * scale;


	vieww = (float) panelw / scale;
	viewh = (float) panelh / scale;

	if (imagew < panelw) {
		imageposx = panelw/2 - imagew/2;
		imageposy = panelh/2 - imageh/2;
	}

	if (viewposx < 0) viewposx = 0;
	//if (viewposx > imagew - panelw) viewposx = imagew - panelw;
	//if (viewposx *scale+viewx*scale > imagew) viewposx = imagew - vieww*scale;
	if (viewposy < 0) viewposy = 0;
	//if (viewposy > imageh - panelh) viewposy = imageh - panelh;

	((wxFrame *) GetParent())->SetStatusText(wxString::Format("image:%dx%d  panel:%dx%d imagepos:%dx%d viewpos:%dx%d",imagew, imageh, panelw, panelh, imageposx, imageposy, viewposx, viewposy));
	wxMemoryDC mdc;
	mdc.SelectObject(*image);
	//StretchBlit (xdest, ydest, dstWidth, dstHeight, wxDC *source, xsrc, ysrc, srcWidth, srcHeight)
	dc.StretchBlit(imageposx,imageposy, panelw, panelh, &mdc, viewposx, viewposy, vieww, viewh);
	mdc.SelectObject(wxNullBitmap);
}

void PicPanel::OnMouseWheel(wxMouseEvent& event)
{
	fit=false;
	mousex = event.m_x;
	mousey = event.m_y;

	double increment = 0.05;

	if (event.GetWheelRotation() > 0)
		scale += increment;
	else
		scale -= increment;

	if (scale < 0.1) 
		scale = 0.1;
	else if (scale > 3) 
		scale = 3; 
	//else {
	//	viewposx += (float) viewposx * increment;
	//	viewposy += (float) viewposy * increment;
	//}


	((wxFrame *) GetParent())->SetStatusText(wxString::Format("scale: %.0f%%", scale*100),2);
	event.Skip();
	Refresh();
}

void PicPanel::OnTimer(wxTimerEvent& event)
{
	Refresh();
}

void PicPanel::OnLeftDoubleClicked(wxMouseEvent& event)
{
	mousex = event.m_x;
	mousey = event.m_y;

	if (scale != 1.0) {
		scale = 1.0;
		fit=false;
		((wxFrame *) GetParent())->SetStatusText(wxString::Format("scale: %.0f%%", scale*100),2);
	
	}
	else {
		fit=true;
		((wxFrame *) GetParent())->SetStatusText("scale: fit",2);
		((wxFrame *) GetParent())->SetStatusText("");
	}

	event.Skip();
	Refresh();
}

double PicPanel::GetScale()
{
	return scale;
}
	
coord PicPanel::GetImgCoords()
{
	return coord {0,0};
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

void PicPanel::OnMouseLeave(wxMouseEvent& event)
{
	dragging = false;
}

void PicPanel::OnLeftDown(wxMouseEvent& event)
{
	mousex = event.m_x;
	mousey = event.m_y;
	dragging = true;
	event.Skip();

}

void PicPanel::OnMouseMove(wxMouseEvent& event)
{
	int mx = event.m_x;
	int my = event.m_y;

	if (fit) return;

	int imagew, imageh, panelw, panelh;
	imagew = (float) image->GetWidth() * scale;
	imageh = (float) image->GetHeight() * scale;
	GetSize(&panelw, &panelh);
	if (imagew < panelw) return;


	if (dragging) {
		viewposx -= (float) (mx - mousex) / scale;
		viewposy -= (float) (my - mousey) / scale;
	}

	mousex = mx;
	mousey = my;
	Refresh();
}

void PicPanel::OnLeftUp(wxMouseEvent& event)
{
	dragging = false;
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

}

void PicPanel::ToggleThumb()
{

}
	
void PicPanel::BlankPic()
{

}

void PicPanel::RefreshPic()
{

}


	
void PicPanel::SetDrawList(wxString list)
{

}
   

void PicPanel::SetColorManagement(bool b)
{

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

}


