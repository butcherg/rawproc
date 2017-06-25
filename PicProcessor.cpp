
#include "PicProcessor.h"
#include "util.h"
#include <wx/event.h>
#include <exception>

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


PicProcessor::PicProcessor(wxString name, wxString command, wxTreeCtrl *tree, PicPanel *display, wxPanel *parameters, gImage * startpic) 
{
	m_parameters = parameters;
	m_display = display;
	m_tree = tree;
	c = command;
	n = name;
	dcList = "";

	dib = startpic;
	m_tree->DeleteAllItems();
	wxTreeItemId id = m_tree->AddRoot(name, -1, -1, this);
	m_tree->SetItemState(id,0);
	m_tree->SelectItem(id);

	m_tree->ExpandAll();
	dirty = true;
}

PicProcessor::PicProcessor(wxString name, wxString command, wxTreeCtrl *tree, PicPanel *display, wxPanel *parameters) 
{
	m_parameters = parameters;
	m_display = display;
	m_tree = tree;
	c = command;
	n = name;
	dcList = "";

	dib = new gImage(getSelectedPicProcessor(m_tree)->getProcessedPic());

	wxTreeItemId id;
	if (m_tree->IsSelected(m_tree->GetRootItem())) 
		id = m_tree->PrependItem(m_tree->GetRootItem(), name, -1, -1, this);
	else 
	 	id = m_tree->InsertItem(m_tree->GetRootItem(), m_tree->GetSelection(), name, -1, -1, this);

	m_tree->SetItemState(id,0);
	m_tree->SelectItem(id);

	dirty = true;
}



PicProcessor::~PicProcessor()
{
	if (dib) delete dib;
}

bool PicProcessor::processPic() 
{ 	
	if (GetId() != m_tree->GetRootItem()) {
		if (dib) delete dib;
		dib = new gImage(getPreviousPicProcessor()->getProcessedPic());
	}
	dirty = false;
	
	processNext();

	return true;
}

void PicProcessor::processNext()
{
	wxTreeItemId next = m_tree->GetNextSibling(GetId());
	if (m_tree->GetItemState(GetId()) == 1) {
		displayProcessedPic();
		//if (next.IsOk()) {
		//	PicProcessor * nextitem = (PicProcessor *) m_tree->GetItemData(next);
		//	nextitem->setDirty();
		//}
	}
	else { 
		if (next.IsOk()) {
			PicProcessor * nextitem = (PicProcessor *) m_tree->GetItemData(next);
			nextitem->processPic();
		}
	}
}

void PicProcessor::setDirty()
{
	dirty = true;
	wxTreeItemId next = m_tree->GetNextSibling(GetId());
	if (next.IsOk()) {
		PicProcessor * nextitem = (PicProcessor *) m_tree->GetItemData(next);
		nextitem->setDirty();
	}
}

bool PicProcessor::isDirty()
{
	return dirty;
}

wxString PicProcessor::getCommand()
{
	if (c.IsEmpty())
		return n;
	else
		return wxString::Format("%s:%s ",n,c);
}

wxString PicProcessor::getName()
{
	return n;
}

wxString PicProcessor::getParams()
{
	return c;
}

void PicProcessor::setParams(wxString params)
{
	c = params;
	dirty = true;
}

void PicProcessor::showParams()
{
	if (!m_parameters) return;
	m_parameters->DestroyChildren();
	r = new BlankPanel(m_parameters, this, "");
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

PicProcessor *PicProcessor::getSelectedPicProcessor(wxTreeCtrl *tree)
{
	wxTreeItemId sel = tree->GetSelection();
	if (sel.IsOk())
		return (PicProcessor *) tree->GetItemData(sel);
	else
		return NULL;
}

gImage& PicProcessor::getProcessedPic() 
{
	if (dirty || dib->getWidth()==0) processPic();
	return *dib;
}

gImage* PicProcessor::getProcessedPicPointer()
{
	if (dirty || dib->getWidth()==0) processPic();
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
		m_display->SetPic(dib);
		m_display->SetDrawList(dcList);
	}
}

void PicProcessor::displayDraw(wxDC &dc)
{
	
}




