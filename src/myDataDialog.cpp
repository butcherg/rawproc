#include "myDataDialog.h"
#include "util.h"

#define MYDATADIALOGRESET 4100
#define MYDATADIALOGTEXTENTRY 4101


myDataDialog::myDataDialog(wxWindow *parent, wxWindowID id, const wxString &title, wxArrayString items, const wxPoint &pos, const wxSize &size):
	wxDialog(parent, id, title, pos, size)
{
	wxBoxSizer *sz = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer *ct = new wxBoxSizer(wxHORIZONTAL);

	lines = items;

	list = new wxTextCtrl(this, wxID_ANY, title, wxDefaultPosition, wxSize(500,400),wxTE_MULTILINE|wxTE_DONTWRAP);
	sz->Add(list, 0, wxEXPAND | wxALL, 3);
			
	ct->Add(new wxButton(this, wxID_OK, _("Dismiss")), 0, wxALL, 10);
	ct->Add(new wxStaticText(this, wxID_ANY, _("Filter: ")), 0, wxTOP|wxBOTTOM|wxLEFT, 10);
	fil = new wxTextCtrl(this, MYDATADIALOGTEXTENTRY, "", wxDefaultPosition, wxSize(100,TEXTCTRLHEIGHT),wxTE_PROCESS_ENTER);
	ct->Add(fil, 0, wxTOP|wxBOTTOM, 10);
	ct->Add(new wxButton(this, MYDATADIALOGRESET, _("X"), wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT), 0, wxTOP|wxBOTTOM|wxRIGHT, 10);
	count = new wxStaticText(this, wxID_ANY, wxString::Format(_("%d items"),lines.GetCount()));
	ct->Add(count, 0, wxALL, 10);
	sz->Add(ct, 0, wxALL, 10);
	SetSizerAndFit(sz);
	fil->SetFocus();

	int i;
	for (i=0; i<lines.GetCount(); i++) {
		list->AppendText(lines[i]+"\n");
	}
	count->SetLabelText(wxString::Format(_("%d lines"),i));
			
	Bind(wxEVT_TEXT_ENTER, &myDataDialog::filterGrid, this, MYDATADIALOGTEXTENTRY);
	Bind(wxEVT_TEXT, &myDataDialog::filterGrid, this, MYDATADIALOGTEXTENTRY);
	Bind(wxEVT_BUTTON, &myDataDialog::resetDialog, this, MYDATADIALOGRESET);
	list->Bind(wxEVT_LEFT_DCLICK, &myDataDialog::doubleClicked, this);
		
}
		
void myDataDialog::endDialog(wxCommandEvent& event)
{
	if (event.GetId() == wxID_OK)
		EndModal(wxID_OK);
	else
		EndModal(wxID_CANCEL);
}

void myDataDialog::doubleClicked(wxMouseEvent& event)
{
	EndModal(wxID_OK);
	event.Skip();
}

void myDataDialog::resetDialog(wxCommandEvent& event)
{
	fil->SetValue("");
	int i;
	for (i=0; i<lines.GetCount(); i++) {
		list->AppendText(lines[i]+"\n");
	}
	count->SetLabelText(wxString::Format(_("%d lines"),i));
	fil->SetFocus();
}
		
void myDataDialog::filterGrid(wxCommandEvent& event)
{
	int i, c=0;;
	wxString filteredlist;
	wxString filter = fil->GetValue();
	for (i=0; i<lines.GetCount(); i++) {
		if (lines[i].Lower().Find(filter.Lower()) != wxNOT_FOUND) { 
			filteredlist.Append(lines[i]+"\n");
			c++;
		}
	}
	list->Clear();
	list->SetValue(filteredlist);
	count->SetLabelText(wxString::Format(_("%d lines"),c));
}
		
