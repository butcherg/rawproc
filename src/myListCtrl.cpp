#include "myListCtrl.h"
#include "myConfig.h"
#include "util.h"


myListCtrl::myListCtrl(wxWindow *parent, wxWindowID id, wxString listname, wxArrayString listitems, const wxPoint &pos, const wxSize &size):
	wxListCtrl(parent, id, pos, size, wxLC_REPORT | wxLC_NO_HEADER | wxLC_HRULES, wxDefaultValidator, listname)
{
	//SetDoubleBuffered(true);
	name = listname;
	width = size.x;
	wxListItem col0;
	col0.SetId(0);
	col0.SetText(name );
	col0.SetWidth(width);
	InsertColumn(0, col0);

	itemlist = listitems;

	for (int i=0; i<itemlist.GetCount(); i++) {
		wxListItem item;
		item.SetId(i);
		item.SetText( itemlist[i] );
		InsertItem( item );
	}

	filter.clear();
	selected.clear();

	Bind(wxEVT_LIST_ITEM_SELECTED, &myListCtrl::Selected, this);
}

//Filters the list to include only entries that contain the specified string:
void myListCtrl::setFilter(wxString f)
{
	wxArrayString filteredlist;
	filter = f;
	//DeleteAllItems();
		
	ClearAll();
	wxListItem col0;
	col0.SetId(0);
	col0.SetText(name);
	col0.SetWidth(width);
	InsertColumn(0, col0);
			
	for (int i=0; i<itemlist.GetCount(); i++) {
		if (itemlist[i].Lower().Find(filter.Lower()) != wxNOT_FOUND) {
		//if (itemlist[i].Find(filter) != wxNOT_FOUND) {
			//wxListItem item;
			//item.SetId(j);
			//item.SetText( itemlist[i] );
			//InsertItem( item );
			//j++;
			filteredlist.Add(itemlist[i]);
		}
	}
	for (int i=0; i<filteredlist.GetCount(); i++) {
		wxListItem item;
		item.SetId(i);
		item.SetText( filteredlist[i] );
		InsertItem( item );
	}
	Refresh();
}
		

		//Captures the entry selected, at selection:
void myListCtrl::Selected(wxListEvent& event)
{
	selected = event.GetText();
	event.Skip();
}

//Returns the selected entry, as populated by the wxListEvent method:
wxString myListCtrl::GetSelected()
{
	return selected;
}
