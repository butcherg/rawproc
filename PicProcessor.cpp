
#include "PicProcessor.h"
#include "FreeImage16.h"
#include "util.h"
#include <wx/event.h>

class BlankPanel: public PicProcPanel 
{
	public:
		BlankPanel(wxPanel *parent, PicProcessor *proc, wxString params): PicProcPanel(parent, proc, params)
		{
			panel = new wxPanel(this);
			b->Add(panel, 1, wxALIGN_LEFT, 10);
			SetSizerAndFit(b);
		}

		~BlankPanel()
		{
			panel->~wxPanel();
		}

	private:
		wxPanel *panel;

};


PicProcessor::PicProcessor(wxString name, wxString command, wxTreeCtrl *tree, PicPanel *display, wxPanel *parameters, FIBITMAP *startpic) {
	m_parameters = parameters;
	m_display = display;
	m_tree = tree;
	c = command;
	n = name;
	if (startpic) { 
		dib = startpic;
		m_tree->DeleteAllItems();
		wxTreeItemId id = m_tree->AddRoot(name, -1, -1, this);
		m_tree->SetItemState(id,0);
		m_tree->SelectItem(id);
	}
	else {
		dib = NULL;
		wxTreeItemId id;
		if (m_tree->IsSelected(m_tree->GetRootItem()))
			id = m_tree->PrependItem(m_tree->GetRootItem(), name, -1, -1, this);
		else
		 	id = m_tree->InsertItem(m_tree->GetRootItem(), m_tree->GetSelection(), name, -1, -1, this);
		m_tree->SetItemState(id,0);
		m_tree->SelectItem(id);
	}
	m_tree->ExpandAll();
	dirty = true;
}


PicProcessor::~PicProcessor()
{
	if (dib) FreeImage_Unload(dib); 
}

bool PicProcessor::processPic() { 
	//get pointer to previous sibling's processed pic
	//apply the processing
	//if selected, put processed pic in display
	
	if (GetId() != m_tree->GetRootItem()) {
		FIBITMAP *s = getPreviousPicProcessor()->getProcessedPic();
		if (dib) FreeImage_Unload(dib);
		dib = FreeImage_Clone(s);
	}
	dirty = false;

	//put in every processPic()...
	if (m_tree->GetItemState(GetId()) == 1) displayProcessedPic();
	m_tree->SetItemBold(GetId(), false);
	wxTreeItemId next = m_tree->GetNextSibling(GetId());
	if (next.IsOk()) {
		PicProcessor * nextitem = (PicProcessor *) m_tree->GetItemData(next);
		nextitem->processPic();
	}
	return true;
}

wxString PicProcessor::getCommand()
{
	return wxString::Format("%s:%s ",n,c);
}

wxString PicProcessor::getName()
{
	return n;
}

wxString PicProcessor::getParams()
{
	return c;
//	if (!r) return;
//	c = r->getParams();
//	processPic();
}

void PicProcessor::setParams(wxString params)
{
	c = params;
	dirty = true;
//	processPic();
}

void PicProcessor::showParams()
{
	if (!m_parameters) return;
	m_parameters->DestroyChildren();
	r = new BlankPanel(m_parameters, this, "");
	//m_parameters->Refresh();
	//m_parameters->Update();
}

PicProcessor *PicProcessor::getPreviousPicProcessor()
{
	wxTreeItemId prev = m_tree->GetPrevSibling(GetId());
	if (prev.IsOk()) {
		//return its data
		return (PicProcessor *) m_tree->GetItemData(prev);
	}
	else {
		//return root data
		return (PicProcessor *) m_tree->GetItemData(m_tree->GetRootItem());
	}
	
}

FIBITMAP *PicProcessor::getProcessedPic() 
{
	if (dirty) processPic();
	return dib;
}

PicPanel *PicProcessor::getDisplay()
{
	return m_display;
}

wxTreeCtrl *PicProcessor::getCommandTree()
{
	return m_tree;
}

void PicProcessor::displayProcessedPic() 
{
	if (m_display) {
		if (dib) {
			m_display->SetPic(dib);
		}
	}
}



enum{
	WORKER_EVENT = wxID_HIGHEST+1
};

ConvolveThread::ConvolveThread(FIBITMAP *psrc, FIBITMAP *pdst, unsigned pstartrow, unsigned pincrement, double pkernel[3][3])
: wxThread(wxTHREAD_JOINABLE)
{
	src = psrc;
	dst = pdst;
	startrow = pstartrow;
	increment = pincrement;
	for (int x=0; x<3; x++)
		for (int y=0; y<3; y++)
			kernel[x][y] = pkernel[x][y];
}

ConvolveThread::~ConvolveThread() 
{ 

}

wxThread::ExitCode ConvolveThread::Entry()
{
	unsigned x, y;
	BYTE *bits = NULL;

	//if(!FreeImage_HasPixels(src))
	//	return false;
	
	int bpp = FreeImage_GetBPP(src);
	//int bytespp = FreeImage_GetLine(src) / FreeImage_GetWidth(src);
	int bytespp = bpp/8;

	unsigned pitch = FreeImage_GetPitch(src);
	BYTE *dibbits = (BYTE*)FreeImage_GetBits(src);
	//bits = dibbits +(pitch*(y))+(x*(bytespp));

	double R, G, B;
	FIRGB16 value;
	//RGBQUAD value;
	//FIBITMAP *dst = FreeImage_Clone(src);

	switch(bpp) {
		case 48:
			for(y = startrow+1; y < FreeImage_GetHeight(src)-1; y+=increment) {
				//bits =  FreeImage_GetScanLine(src, y);
				FIRGB16 *srcbits = (FIRGB16 *)FreeImage_GetScanLine(src, y);
				FIRGB16 *dstbits = (FIRGB16 *)FreeImage_GetScanLine(dst, y);
				//BYTE * maskbits = (BYTE *)FreeImage_GetScanLine(mask,y);
				for(x = 1; x < FreeImage_GetWidth(src)-1; x++) {
					//if (mask && (maskbits[0] < threshold)) {
					//	dstbits[x].red   = srcbits[x].red;
					//	dstbits[x].green = srcbits[x].green;
					//	dstbits[x].blue  = srcbits[x].blue;
					//}
					//else {
						R=0.0; G=0.0; B=0.0;
						for (int kx=0; kx<3; kx++) {
							for (int ky=0; ky<3; ky++) {
								int ix = x-1+kx;
								int iy = y-1+ky;

								FreeImage_GetPixelColor16(src, ix, iy, &value);
								//FIRGB16 *pixel = (FIRGB16 *) dibbits +(pitch*(iy))+(ix*(bytespp));

								R += value.red   * kernel[kx][ky];
								G += value.green * kernel[kx][ky];
								B += value.blue  * kernel[kx][ky];

							}
						}
						dstbits[x].red   = MIN(MAX(int(R), 0), 65535);
						dstbits[x].green = MIN(MAX(int(G), 0), 65535);
						dstbits[x].blue  = MIN(MAX(int(B), 0), 65535);
					//}

				}
			}
			break;
            
		case 24 :

			for(y = startrow+1; y < FreeImage_GetHeight(src)-1; y+=increment) {
				//bits =  FreeImage_GetScanLine(src, y);
				BYTE *srcbits = FreeImage_GetScanLine(src, y);
				BYTE *dstbits = FreeImage_GetScanLine(dst, y);
				//BYTE * maskbits = (BYTE *)FreeImage_GetScanLine(mask,y);
				for(x = 1; x < FreeImage_GetWidth(src)-1; x++) {
					//if (mask && (maskbits[0] < threshold)) {
					//	dstbits[FI_RGBA_RED]   = srcbits[FI_RGBA_RED];
					//	dstbits[FI_RGBA_GREEN] = srcbits[FI_RGBA_GREEN];
					//	dstbits[FI_RGBA_BLUE]  = srcbits[FI_RGBA_BLUE];
					//}
					//else {
						R=0.0; G=0.0; B=0.0;
						for (int kx=0; kx<3; kx++) {
							for (int ky=0; ky<3; ky++) {
								int ix = x-1+kx;
								int iy = y-1+ky;
	
								FreeImage_GetPixelColor16(src, ix, iy, &value);
								//BYTE *pixel =  dibbits +(pitch*(iy))+(ix*(bytespp));
	
								R += value.red   * kernel[kx][ky];
								G += value.green * kernel[kx][ky];
								B += value.blue  * kernel[kx][ky];

							}
						}
						dstbits[FI_RGBA_RED]   = MIN(MAX(int(R), 0), 255);
						dstbits[FI_RGBA_GREEN] = MIN(MAX(int(G), 0), 255);
						dstbits[FI_RGBA_BLUE]  = MIN(MAX(int(B), 0), 255);
					//}
					srcbits += 3;
					dstbits += 3;
					//maskbits++;

				}
			}
			break;
	}

	return (wxThread::ExitCode)0;
}
