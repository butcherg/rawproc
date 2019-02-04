
#include "PicPanel.h"
#include "util.h"
#include <vector>
#include "PicProcessor.h"
#include "myConfig.h"
#include <wx/clipbrd.h>

//move to a property:
int border = 5;


PicPanel::PicPanel(wxFrame *parent, wxTreeCtrl *tree, myHistogramPane *hgram): wxPanel(parent, wxID_ANY, wxDefaultPosition, wxSize(1000,740)) 
{

	SetDoubleBuffered(true); 
	display_dib = NULL;
	image = NULL;
	scale = 1.0;
	imgctrx = 0.5; imgctry = 0.5;
	imageposx=0; imageposy = 0;
	mousex = 0; mousey=0;
	dragging = false;
	histogram = hgram;

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
		
	//t = new wxTimer(this);
}

PicPanel::~PicPanel()
{
	if (image) image->~wxBitmap();
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
		display_dib = dib;
		wxImage img = gImage2wxImage(*dib);
		if (image) image->~wxBitmap();
		image = new wxBitmap(img);
		imagew = image->GetWidth();
		imageh = image->GetHeight();
		//ToDo: cmsTransform()...
	
		//parm histogram.scale: The number of buckets to display in the histogram. Default=256
		unsigned scale = atoi(myConfig::getConfig().getValueOrDefault("histogram.scale","256").c_str());
		
		histogram->SetPic(*dib, scale);
		//parm histogram.singlechannel: 0|1, turns on/off the display of single-channel histogram plot for per-channel curves
		if (myConfig::getConfig().getValueOrDefault("histogram.singlechannel","1") == "1")
			histogram->SetChannel(channel);
		else
			histogram->SetChannel(CHANNEL_RGB);

		Refresh();
	}
}

void PicPanel::setStatusBar()
{
	struct pix p = display_dib->getPixel(imagex, imagey);

	if (imagex > 0 & imagex <= imagew & imagey > 0 & imagey <= imageh)
		//((wxFrame *) GetParent())->SetStatusText(wxString::Format("xy:%d,%d rgb:%f,%f,%f",imagex, imagey, p.r, p.g, p.b));
		((wxFrame *) GetParent())->SetStatusText(wxString::Format("imagepos:%dx%d viewpos:%dx%d view:%dx%d xy:%d,%d",
			imageposx,imageposy,  viewposx, viewposy, vieww, viewh, imagex, imagey));
	else
		((wxFrame *) GetParent())->SetStatusText("");

	if (fit)
		((wxFrame *) GetParent())->SetStatusText("scale: fit",2);
	else
		((wxFrame *) GetParent())->SetStatusText(wxString::Format("scale: %.0f%%", scale*100),2);
}


void PicPanel::render(wxDC &dc)
{
	if (!image) return;
	int panelw, panelh,  scaledimagew, scaledimageh;

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
	if (scaledimagew <= panelw | scaledimageh <= panelh) {
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

	wxMemoryDC mdc;
	mdc.SelectObject(*image);
	// definition: StretchBlit (xdest, ydest, dstWidth, dstHeight, wxDC *source, xsrc, ysrc, srcWidth, srcHeight)
	dc.StretchBlit(imageposx,imageposy, panelw, panelh, &mdc, viewposx, viewposy, vieww, viewh);
	mdc.SelectObject(wxNullBitmap);
}

void PicPanel::OnMouseWheel(wxMouseEvent& event)
{
	fit=false;
	int mx = event.m_x;
	int my = event.m_y;

	double increment = 0.05;

	if (event.GetWheelRotation() > 0)
		scale += increment;
	else
		scale -= increment;

	if (scale < 0.1) 
		scale = 0.1;
	else if (scale > 3) 
		scale = 3; 


	if (event.GetWheelRotation() > 0) {
		viewposx -= (float) viewposx * increment;
		viewposy -= (float) viewposy * increment;
	}
	else {
		viewposx += (float) viewposx * increment;
		viewposy += (float) viewposy * increment;
	}

	imagex = (((mx-border) - imageposx) / scale) + (viewposx);
	imagey = (((my-border) - imageposy) / scale) + (viewposy);

	setStatusBar();

	mousex = mx;
	mousey = my;

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

	if (scale != 1.0) {
		scale = 1.0;
		fit=false;
	}
	else {
		fit=true;
	}

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
	((wxFrame *) GetParent())->SetStatusText("");
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

	if (!fit & dragging) { 
		viewposx -= (float) (mx - mousex) / scale;
		viewposy -= (float) (my - mousey) / scale;
		Refresh();
	}

	imagex = (((mx-border) - imageposx) / scale) + (viewposx);
	imagey = (((my-border) - imageposy) / scale) + (viewposy);

	setStatusBar();

	mousex = mx;
	mousey = my;

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


