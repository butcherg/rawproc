#include "PicProcessor.h"
#include "PicProcessorSpot.h"
#include "PicProcPanel.h"
#include "myConfig.h"
#include "myRowSizer.h"
#include "myIntegerCtrl.h"
#include "gimage_parse.h"
#include "gimage_process.h"
#include "undo.xpm"
#include "util.h"
#include "copy.xpm"
#include "paste.xpm"

#include <wx/grid.h>

#include <vector>

#define SPOTENABLE 8800
#define SPOTRADIAL  8801
#define SPOTCLONE   8802
#define SPOTFILE	8803
#define SPOTLIST	8804
#define SPOTCOPY	8805
#define SPOTPASTE	8806

struct spt {
	unsigned sx, sy, px, py, pr;
};

class pointList: public wxGrid
{
public:
	pointList(wxWindow *parent): wxGrid(parent, wxID_ANY)
	{
		CreateGrid( 0, 5 );
		SetSelectionMode(wxGridSelectRows);
		//HideRowLabels();
		SetRowLabelSize(20);
		SetColLabelValue (0, "sx");
		SetColLabelValue (1, "sy");
		SetColLabelValue (2, "px");
		SetColLabelValue (3, "py");
		SetColLabelValue (4, "r");
		SetColFormatNumber(0);
		SetColFormatNumber(1);
		SetColFormatNumber(2);
		SetColFormatNumber(3);
		SetColFormatNumber(4);
		AutoSizeColumns();
		//SetColSize(0,-1);
		//SetColSize(1,-1);
		//SetColSize(2,-1);
		//SetColSize(3,-1);
		//SetColSize(4,-1);
	}
	
	int size()
	{
		return GetNumberRows();
	}
	
	int push_back(spt p)
	{
		AppendRows(1);
		int row = GetNumberRows()-1;
		SetCellValue(row, 0, wxString::Format("%d",p.sx));
		SetCellValue(row, 1, wxString::Format("%d",p.sy));
		SetCellValue(row, 2, wxString::Format("%d",p.px));
		SetCellValue(row, 3, wxString::Format("%d",p.py));
		SetCellValue(row, 4, wxString::Format("%d",p.pr));	
		AutoSizeColumns();
		return row;
	}
	
	std::vector<spt> getPoints()
	{
		std::vector<spt> points;
		for (int i=0; i< size(); i++) {
			spt p;
			p.sx = atoi(GetCellValue(i, 0).ToStdString().c_str());
			p.sy = atoi(GetCellValue(i, 1).ToStdString().c_str());
			p.px = atoi(GetCellValue(i, 2).ToStdString().c_str());
			p.py = atoi(GetCellValue(i, 3).ToStdString().c_str());
			p.pr =  atoi(GetCellValue(i, 4).ToStdString().c_str());
			points.push_back(p);
		}
		return points;
	}
	
	std::string getPointString()
	{
		std::string pstr;
		for (int i=0; i< size(); i++) {
			pstr += GetCellValue(i, 0).ToStdString(); pstr += ",";
			pstr += GetCellValue(i, 1).ToStdString(); pstr += ",";
			pstr += GetCellValue(i, 2).ToStdString(); pstr += ",";
			pstr += GetCellValue(i, 3).ToStdString(); pstr += ",";
			pstr += GetCellValue(i, 4).ToStdString(); pstr += ";";
		}
		return pstr;
	}
	
	void setPointString(wxString ps)
	{
		DeleteRows(0,GetNumberRows());
		wxArrayString pts = split(ps, ";");
		for (int i=0; i<pts.size(); i++) {
			AppendRows(1);
			int row = GetNumberRows()-1;
			wxArrayString pv = split(pts[i], ",");
			SetCellValue(row, 0, pv[0]);
			SetCellValue(row, 1, pv[1]);
			SetCellValue(row, 2, pv[2]);
			SetCellValue(row, 3, pv[3]);
			SetCellValue(row, 4, pv[4]);
		}
		AutoSizeColumns();
	}
	
	void setPoints(std::vector<spt> pts)
	{
		//ClearGrid();
		DeleteRows(0,GetNumberRows());
		for (int i=0; i< pts.size(); i++) {
			AppendRows(1);
			int row = GetNumberRows()-1;
			SetCellValue(row, 0, wxString::Format("%d",pts[i].sx));
			SetCellValue(row, 1, wxString::Format("%d",pts[i].sy));
			SetCellValue(row, 2, wxString::Format("%d",pts[i].px));
			SetCellValue(row, 3, wxString::Format("%d",pts[i].py));
			SetCellValue(row, 4, wxString::Format("%d",pts[i].pr));	
		}
		AutoSizeColumns();
	}
	
	void insert(int row, spt p)
	{
		InsertRows(row);
		SetCellValue(row, 0, wxString::Format("%d",p.sx));
		SetCellValue(row, 1, wxString::Format("%d",p.sy));
		SetCellValue(row, 2, wxString::Format("%d",p.px));
		SetCellValue(row, 3, wxString::Format("%d",p.py));
		SetCellValue(row, 4, wxString::Format("%d",p.pr));	
		AutoSizeColumns();
	}
	
	void changePatch(int row, coord p)
	{
		SetCellValue(row, 2, wxString::Format("%d", p.x));
		SetCellValue(row, 3, wxString::Format("%d", p.y));
		AutoSizeColumns();
	}
	
	void changeRadius(int row, unsigned r)
	{
		SetCellValue(row, 4, wxString::Format("%d", r));
		AutoSizeColumns();
	}
	
	bool erase(int pt)
	{
		bool t = DeleteRows(pt);
		AutoSizeColumns();
		return t;
	}
	
	void select(int pt)
	{
		SelectRow(pt);
		GoToCell(pt,0);
	}
	
	int getSelected()
	{
		if (size() <= 0) return -1;
		wxArrayInt i = GetSelectedRows();
		if (i.GetCount() > 0) return i[0];
		return -1;
	}
	
	spt operator[](int index) const
	{
		spt p;
		p.sx = atoi(GetCellValue(index, 0).ToStdString().c_str());
		p.sy = atoi(GetCellValue(index, 1).ToStdString().c_str());
		p.px = atoi(GetCellValue(index, 2).ToStdString().c_str());
		p.py = atoi(GetCellValue(index, 3).ToStdString().c_str());
		p.pr = atoi(GetCellValue(index, 4).ToStdString().c_str());
		return p;
	} 
	
	spt at(int index)
	{
		spt p;
		p.sx = atoi(GetCellValue(index, 0).ToStdString().c_str());
		p.sy = atoi(GetCellValue(index, 1).ToStdString().c_str());
		p.px = atoi(GetCellValue(index, 2).ToStdString().c_str());
		p.py = atoi(GetCellValue(index, 3).ToStdString().c_str());
		p.pr = atoi(GetCellValue(index, 4).ToStdString().c_str());
		return p;
	}
	
	
private:
	
	
};

class SpotPanel: public PicProcPanel
{
	public:
		SpotPanel(wxWindow *parent, PicProcessor *proc, wxString params): PicProcPanel(parent, proc, params)
		{
			Freeze();
			SetSize(parent->GetSize());
			wxSizerFlags flags = wxSizerFlags().Center().Border(wxLEFT|wxRIGHT|wxTOP|wxBOTTOM);

			p.x = p.y = s.x = s.y = 0;

			enablebox = new wxCheckBox(this, SPOTENABLE, _("spot:"));
			enablebox->SetValue(true);

			radialb = new wxRadioButton(this, SPOTRADIAL, _("radial"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
			cloneb  = new wxRadioButton(this, SPOTCLONE, _("clone"));
			fileb  = new wxRadioButton(this, SPOTFILE, _("file"));
			clonelistb  = new wxRadioButton(this, SPOTLIST, _("clone list"));
			
			spot = new wxStaticText(this, wxID_ANY, _("--,--"));
			patch = new wxStaticText(this, wxID_ANY, _("--,--"));

			//parm tool.spot.radius: Default value for spot/patch radius.  Default=20
			radius = new myIntegerCtrl(this, wxID_ANY, "Radius:", atoi(myConfig::getConfig().getValueOrDefault("tool.spot.radius","20").c_str()), 0, 100);
			clist = new pointList(this);

			clonelistb->SetValue(true);
			spotmode = SPOTLIST;
			
			setParamString(params);

			myRowSizer *m = new myRowSizer(wxSizerFlags().Expand());
			m->AddRowItem(enablebox, wxSizerFlags(1).Left().Border(wxLEFT|wxTOP));
			m->AddRowItem(new wxBitmapButton(this, SPOTCOPY, wxBitmap(copy_xpm), wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT), flags);
			m->AddRowItem(new wxBitmapButton(this, SPOTPASTE, wxBitmap(paste_xpm), wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT), flags);
			m->NextRow(wxSizerFlags().Expand());
			m->AddRowItem(new wxStaticLine(this, wxID_ANY), wxSizerFlags(1).Left().Border(wxLEFT|wxRIGHT|wxTOP|wxBOTTOM));

			m->NextRow();
			m->AddRowItem(new wxStaticText(this, wxID_ANY, _("Spot:")), flags);
			m->AddRowItem(spot, flags);
			m->NextRow();
			m->AddRowItem(radius, flags);
			m->NextRow(wxSizerFlags().Expand());
			m->AddRowItem(new wxStaticLine(this, wxID_ANY), wxSizerFlags(1).Left().Border(wxLEFT|wxRIGHT|wxTOP|wxBOTTOM));

			m->NextRow();
			m->AddRowItem(cloneb,flags);
			m->AddRowItem(new wxStaticText(this, wxID_ANY, _("Patch:")), flags);
			m->AddRowItem(patch, flags);

			m->NextRow();
			m->AddRowItem(radialb,flags);

			m->NextRow();
			m->AddRowItem(fileb,flags);
			
			m->NextRow();
			m->AddRowItem(clonelistb,flags);
			m->NextRow();
			m->Add(clist, wxSizerFlags(1).Expand());

			m->End();
			SetSizerAndFit(m);
			SetFocus();
			t.SetOwner(this);
			Bind(wxEVT_BUTTON, &SpotPanel::OnButton, this);
			Bind(myINTEGERCTRL_CHANGE,&SpotPanel::OnChanged, this);
			Bind(myINTEGERCTRL_UPDATE,&SpotPanel::paramChanged, this);
			Bind(wxEVT_RADIOBUTTON, &SpotPanel::OnRadioButton, this);
			Bind(wxEVT_CHECKBOX, &SpotPanel::onEnable, this, SPOTENABLE);
			Bind(wxEVT_TIMER, &SpotPanel::OnTimer,  this);
			Bind(wxEVT_CHAR_HOOK, &SpotPanel::OnKey,  this);
			Bind(wxEVT_BUTTON, &SpotPanel::OnCopy, this, SPOTCOPY);
			Bind(wxEVT_BUTTON, &SpotPanel::OnPaste, this, SPOTPASTE);
			clist->Bind(wxEVT_GRID_CELL_CHANGED, &SpotPanel::OnGrid, this);
			Thaw();
		}
		
		void setParamString(wxString params)
		{
			wxArrayString pm = split(params,",");
			
			if (pm.size() >= 1) {
				if (pm[0] == "radial") {
					radialb->SetValue(true);
					spotmode = SPOTRADIAL;
					if (pm.size() >= 2) s.x = atoi(pm[1].c_str());
					if (pm.size() >= 3) s.y = atoi(pm[2].c_str());
					if (pm.size() >= 4) radius->SetIntegerValue(atoi(pm[3].c_str()));
				}
				else if (pm[0] == "clone") {
					cloneb->SetValue(true);
					spotmode = SPOTCLONE;
					if (pm.size() >= 2) s.x = atoi(pm[1].c_str());
					if (pm.size() >= 3) s.y = atoi(pm[2].c_str());
					if (pm.size() >= 4) p.x = atoi(pm[3].c_str());
					if (pm.size() >= 5) p.y = atoi(pm[4].c_str());
					if (pm.size() >= 6) radius->SetIntegerValue(atoi(pm[5].c_str()));
				}
				else if (pm[0] == "file") {
					fileb->SetValue(true);
					spotmode = SPOTFILE;
				}
				else if (true) { //isInt(pm[0])) {
					clonelistb->SetValue(true);
					spotmode = SPOTLIST;
					clist->setPointString(params);
				}
				else {
					clonelistb->SetValue(true);
					spotmode = SPOTLIST;
				}
			}
		}

		void onEnable(wxCommandEvent& event)
		{
			if (enablebox->GetValue()) {
				q->enableProcessing(true);
				q->processPic();
			}
			else {
				q->enableProcessing(false);
				q->processPic();
			}
		}
		
		void OnCopy(wxCommandEvent& event)
		{
			q->copyParamsToClipboard();
			((wxFrame *) GetGrandParent())->SetStatusText(wxString::Format(_("Copied command to clipboard: %s"),q->getCommand()));
		}
		
		void OnPaste(wxCommandEvent& event)
		{
			if (q->pasteParamsFromClipboard()) {
				setParamString(q->getParams());
				q->processPic();
				((wxFrame *) GetGrandParent())->SetStatusText(wxString::Format(_("Pasted command from clipboard: %s"),q->getCommand()));
				Refresh();
			}
			else wxMessageBox(wxString::Format("Invalid Paste"));
		}
		
		void OnGrid(wxGridEvent &event)
		{
			q->processPic();
			Refresh();
		}

		void OnChanged(wxCommandEvent& event)
		{
			t.Start(500,wxTIMER_ONE_SHOT);
		}

		void OnTimer(wxTimerEvent& event)
		{
			if (spotmode == SPOTLIST) {
				int row = clist->getSelected();
				if (row < 0) return;
				clist->changeRadius(row, radius->GetIntegerValue());
			}
			processSP();
		}

		void paramChanged(wxCommandEvent& event)
		{
			processSP();
			event.Skip();
		}

		void SetSpot(coord spot)
		{
			s=spot;
			this->spot->SetLabel(wxString::Format("%d,%d", s.x, s.y));
			Refresh();
			processSP();
		}

		void SetPatch(coord patch)
		{
			p=patch;
			this->patch->SetLabel(wxString::Format("%d,%d", p.x, p.y));
			Refresh();
			processSP();
		}
		
		void AddSpot(coord spot)
		{
			int lr = 15;
			spt sp;
			for (int i = 0; i < clist->size(); i++) {
				spt e = clist->at(i);
				if ((spot.x > e.sx-lr) & (spot.x < e.sx+lr)) {
					if ((spot.y > e.sy-lr) & (spot.y < e.sy+lr)) {
						clist->erase(i);
						if (clist->size() > 0) clist->SelectRow(i-1);
						processSP();
						return;
					}
				}
			}
			sp.sx = spot.x; sp.sy = spot.y; sp.px = 0; sp.py = 0; sp.pr = radius->GetIntegerValue();
			int row = clist->push_back(sp);
			clist->SelectRow(row);
			processSP();
		}
		
		void AddPatch(coord patch)
		{
			if (clist->size() <= 0) return;
			int row = clist->getSelected();
			if (row < 0) return;
			clist->changePatch(row, patch);
			processSP();
		}

		int GetRadius()
		{
			return radius->GetIntegerValue();
		}

		void OnThumbTrack(wxCommandEvent& event)
		{

		}

		void OnButton(wxCommandEvent& event)
		{
			processSP();
			event.Skip();
		}

		wxString DCList()
		{
			wxString dclist;
			if (spotmode == SPOTCLONE) {
				if (p.x != 0) dclist.Append(wxString::Format("circle,%d,%d,%d;", p.x, p.y, radius->GetIntegerValue()/2));
				if (s.x != 0) dclist.Append(wxString::Format("cross,%d,%d;", s.x, s.y));
			}
			else if (spotmode == SPOTLIST) {
				for (int i = 0; i < clist->size(); i++) {
					spt e = clist->at(i);
					if (e.sx > 0 & e.sy > 0) dclist.Append(wxString::Format("cross,%d,%d;", e.sx, e.sy));
					if (e.px > 0 & e.py > 0) dclist.Append(wxString::Format("circle,%d,%d,%d;", e.px, e.py, e.pr/2));
				}
			}
			return dclist;
		}

		void processSP()
		{
			if (spotmode == SPOTRADIAL) {
				if (s.x != 0) {
					q->setParams(wxString::Format("radial,%d,%d,%d",s.x, s.y, radius->GetIntegerValue()));
					q->processPic();
				}
			}
			else if (spotmode == SPOTCLONE) {
				if (s.x != 0 & p.x != 0) {
					q->setParams(wxString::Format("clone,%d,%d,%d,%d,%d",s.x, s.y, p.x, p.y, radius->GetIntegerValue()));
					q->processPic();
				}
			}
			else if (spotmode == SPOTFILE) {
				//to-do...
				q->setParams(wxString::Format("file,%s","foo.txt"));
				q->processPic();
			}
			else if (spotmode == SPOTLIST) {
				//if (clist->size() > 0) {
					q->setParams(wxString(clist->getPointString()));
					q->processPic();
				//}
			}
		}

		void OnRadioButton(wxCommandEvent& event)
		{
			spotmode = event.GetId();
			processSP();
		}

	private:
		wxBitmapButton *btn;
		wxCheckBox *enablebox;
		wxRadioButton *radialb, *cloneb, *fileb, *clonelistb;
		wxStaticText *spot, *patch;
		coord s, p;
		myIntegerCtrl * radius;
		std::vector<spt> splist;
		pointList * clist;
		int spotmode;
		wxTimer t;
};


PicProcessorSpot::PicProcessorSpot(wxString name, wxString command, wxTreeCtrl *tree, PicPanel *display): PicProcessor(name, command,  tree, display) 
{
	m_display->Bind(wxEVT_LEFT_DOWN, &PicProcessorSpot::OnLeftDown, this);
}

PicProcessorSpot::~PicProcessorSpot()
{
	m_display->Unbind(wxEVT_LEFT_DOWN, &PicProcessorSpot::OnLeftDown, this);
}

void PicProcessorSpot::createPanel(wxSimplebook* parent)
{
	toolpanel = new SpotPanel(parent, this, c);
	dcList = ((SpotPanel *) toolpanel)->DCList();
	parent->ShowNewPage(toolpanel);
	toolpanel->Refresh();
	toolpanel->Update();
}

bool PicProcessorSpot::processPicture(gImage *processdib) 
{
	if (!processingenabled) return true;

	((wxFrame*) m_display->GetParent())->SetStatusText(_("spot..."));
	bool ret = true;
	std::map<std::string,std::string> result;

	std::map<std::string,std::string> params;
	std::string pstr = getParams().ToStdString();

	if (!pstr.empty())
		params = parse_spot(std::string(pstr));

	if (params.find("error") != params.end()) {
		wxMessageBox(params["error"]);
		ret = false; 
	}
	else if (params.find("mode") == params.end()) {  //all variants need a mode, now...
		ret = false;
	}
	else { 
		dcList = ((SpotPanel *) toolpanel)->DCList();
		result = process_spot(*dib, params);

		if (result.find("error") != result.end()) {
			wxMessageBox(wxString(result["error"]));
			ret = false;
		}
		else {
			if (paramexists(result,"treelabel")) m_tree->SetItemText(id, wxString(result["treelabel"]));
			m_display->SetModified(true);
			if ((myConfig::getConfig().getValueOrDefault("tool.all.log","0") == "1") || 
				(myConfig::getConfig().getValueOrDefault("tool.spot.log","0") == "1"))
					log(wxString::Format(_("tool=spot,%s,imagesize=%dx%d,threads=%s,time=%s"),
						params["mode"].c_str(),
						dib->getWidth(), 
						dib->getHeight(),
						result["threadcount"].c_str(),
						result["duration"].c_str())
					);
		}
	}

	dirty=false;
	((wxFrame*) m_display->GetParent())->SetStatusText("");
	return ret;
}

void PicProcessorSpot::OnLeftDown(wxMouseEvent& event)
{
	coord spot, patch;
	if (m_tree->GetSelection() == GetId()) {
		if (event.ShiftDown()) {
			patch = m_display->GetImgCoords();
			//((SpotPanel *) toolpanel)->SetPatch(patch);
			((SpotPanel *) toolpanel)->AddPatch(patch);
			dcList = ((SpotPanel *) toolpanel)->DCList();
		}
		else if (event.ControlDown()) {
			spot = m_display->GetImgCoords();
			//((SpotPanel *) toolpanel)->SetSpot(spot);
			((SpotPanel *) toolpanel)->AddSpot(spot);
			dcList = ((SpotPanel *) toolpanel)->DCList();
		}
	}
	event.Skip();
}
