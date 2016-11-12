
#include "PicProcessor.h"
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


PicProcessor::PicProcessor(wxString name, wxString command, wxTreeCtrl *tree, PicPanel *display, wxPanel *parameters, gImage& startpic) {
	m_parameters = parameters;
	m_display = display;
	m_tree = tree;
	c = command;
	n = name;
	if (startpic.getWidth() != 0) { 
		dib = startpic;
		m_tree->DeleteAllItems();
		wxTreeItemId id = m_tree->AddRoot(name, -1, -1, this);
		m_tree->SetItemState(id,0);
		m_tree->SelectItem(id);
	}
	else {
		dib = gImage();
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

}

bool PicProcessor::processPic() { 
	//get pointer to previous sibling's processed pic
	//apply the processing
	//if selected, put processed pic in display
	
	if (GetId() != m_tree->GetRootItem()) {
		dib = getPreviousPicProcessor()->getProcessedPic();
	}
	dirty = false;

	//put in every processPic()...
	if (m_tree->GetItemState(GetId()) == 1) displayProcessedPic();
	wxTreeItemId next = m_tree->GetNextSibling(GetId());
	if (next.IsOk()) {
		PicProcessor * nextitem = (PicProcessor *) m_tree->GetItemData(next);
		nextitem->processPic();
	}
	return true;
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

gImage& PicProcessor::getProcessedPic() 
{
	if (dirty) processPic();
	//if (!dib) processPic();
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
	}
}

