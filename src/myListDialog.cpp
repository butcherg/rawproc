#include "myListDialog.h"
//#include "myConfig.h"
#include "util.h"


myListDialog::myListDialog(wxWindow *parent, wxWindowID id, const wxString &title, wxArrayString items, const wxPoint &pos, const wxSize &size):
	wxDialog(parent, id, title, pos, size)
{
	wxBoxSizer *sz = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer *ct = new wxBoxSizer(wxHORIZONTAL);

	list = new myListCtrl(this, wxID_ANY, title, items, wxDefaultPosition, wxSize(400,300));
	sz->Add(list, 0, wxEXPAND | wxALL, 3);
			
	ct->Add(new wxButton(this, wxID_OK, _("Ok")), 0, wxALL, 10);
	ct->Add(new wxButton(this, wxID_CANCEL, _("Cancel")), 0, wxALL, 10);
	ct->Add(new wxStaticText(this, wxID_ANY, _("Filter: ")), 0, wxALL, 10);
	fil = new wxTextCtrl(this, wxID_ANY, "", wxDefaultPosition, wxSize(100,TEXTCTRLHEIGHT),wxTE_PROCESS_ENTER);
	ct->Add(fil, 0, wxALL, 10);
	count = new wxStaticText(this, wxID_ANY, wxString::Format(_("%d items"),list->GetItemCount()));
	ct->Add(count, 0, wxALL, 10);
	sz->Add(ct, 0, wxALL, 10);
	SetSizerAndFit(sz);
	fil->SetFocus();
			
	Bind(wxEVT_TEXT_ENTER, &myListDialog::filterGrid, this);
	Bind(wxEVT_TEXT, &myListDialog::filterGrid, this);
	Bind(wxEVT_BUTTON, &myListDialog::endDialog, this);
	list->Bind(wxEVT_LEFT_DCLICK, &myListDialog::doubleClicked, this);
		
}
		
void myListDialog::endDialog(wxCommandEvent& event)
{
	if (event.GetId() == wxID_OK)
		EndModal(wxID_OK);
	else
		EndModal(wxID_CANCEL);
}

void myListDialog::doubleClicked(wxMouseEvent& event)
{
	EndModal(wxID_OK);
	event.Skip();
}
		
void myListDialog::filterGrid(wxCommandEvent& event)
{
	list->setFilter(fil->GetValue());
	count->SetLabelText(wxString::Format(_("%d items"),list->GetItemCount()));
}
		
wxString myListDialog::getSelection()
{
	return list->GetSelected();
}

