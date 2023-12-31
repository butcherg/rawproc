#include "wx/wx.h"
#include <wx/process.h>
#include <wx/event.h>
#include <wx/txtstrm.h>

#define WXCMD_DISMISS 2000
#define WXCMD_KILL 2001

class cmdProcess;

class wxCmdApp : public wxApp
{
public:
	virtual bool OnInit(); //wxOVERRIDE;
};

class wxCmdFrame : public wxFrame
{
public:
	wxCmdFrame(const wxString& title, wxString cmd, bool autodismiss);
	void OnQuit(wxCommandEvent& event);
	void OnIdle(wxIdleEvent& event);
	void OnTimer(wxTimerEvent& event);
	void OnTerm(wxProcessEvent& event);
	void OnDismiss(wxCommandEvent& event);
	void OnKill(wxCommandEvent& event);
	void Terminate();
	void Append(wxString line);

private:
	wxButton *dismiss, *kill;
	wxTextCtrl *t;
	cmdProcess *p;
	int pid;
	wxTimer timer;
	bool adismiss;
};

wxIMPLEMENT_APP(wxCmdApp);



class cmdProcess: public wxProcess
{
public:
	cmdProcess(wxCmdFrame *parent, const wxString& cmd): wxProcess(wxPROCESS_REDIRECT)
	{
		m_parent = parent;
		m_cmd = cmd;
	}

	virtual void OnTerminate(int pid, int status) //wxOVERRIDE
	{
		while ( HasInput() )
			;
		m_parent->Append("WXCMD: PROCESS TERMINATED.");
		m_parent->Terminate();
	}

	void Kill()
	{
		long pid = GetPid();
		wxProcess::Kill(pid);
	}
	
	virtual bool HasInput()
	{
		bool hasInput = false;
		if ( IsInputAvailable() )
		{
			wxTextInputStream tis(*GetInputStream());
			wxString msg;
			msg << tis.ReadLine();
			m_parent->Append(msg);
			hasInput = true;
		}

		if ( IsErrorAvailable() )
		{
			wxTextInputStream tis(*GetErrorStream());
			wxString msg;
			msg << tis.ReadLine();
			m_parent->Append(msg);
			hasInput = true;
		}
		return hasInput;
	}

protected:
	wxCmdFrame *m_parent;
	wxString m_cmd;
};



bool wxCmdApp::OnInit()
{
	bool autodismiss = false;
	wxString cmdstring;
	for (int i=1; i < wxAppConsole::argc; i++)
		if (wxAppConsole::argv[i] == "-x") autodismiss = true;
	else
		cmdstring.Append(wxString::Format("%s ",wxAppConsole::argv[i]));
	wxCmdFrame *frame = new wxCmdFrame("wxCmd", cmdstring, autodismiss);
	frame->Show(true);
	return true;
}

wxCmdFrame::wxCmdFrame(const wxString& title, wxString cmdstring, bool autodismiss): wxFrame(NULL, wxID_ANY, title)
{
	adismiss = autodismiss;
	wxBoxSizer *s = new wxBoxSizer(wxVERTICAL);
	t = new wxTextCtrl(this, wxID_ANY, "", wxDefaultPosition, wxSize(800,400), wxTE_MULTILINE);
	s->Add(t, wxSizerFlags(1).Expand().Border(wxALL, 5));
	wxBoxSizer *b = new wxBoxSizer(wxHORIZONTAL);
	dismiss = new wxButton(this, WXCMD_DISMISS, "Kill and Dismiss");
	b->Add(dismiss, wxSizerFlags(1).Expand().Border(wxALL, 5));
	kill = new wxButton(this, WXCMD_KILL, "Kill");
	b->Add(kill, wxSizerFlags(1).Expand().Border(wxALL, 5));
	s->Add(b);
	SetSizerAndFit(s);
	Center();

	p = new cmdProcess(this, cmdstring);
	pid = wxExecute(cmdstring, wxEXEC_ASYNC, p);
	timer.SetOwner(this);

	Bind(wxEVT_BUTTON,&wxCmdFrame::OnDismiss, this, WXCMD_DISMISS);
	Bind(wxEVT_BUTTON,&wxCmdFrame::OnKill, this, WXCMD_KILL);
	Bind(wxEVT_TIMER, &wxCmdFrame::OnTimer, this);
	timer.Start(100,wxTIMER_CONTINUOUS);
}

void wxCmdFrame::Append(wxString line)
{
	*t << line << "\n";
	t->Update();
}

void wxCmdFrame::OnTerm(wxProcessEvent& event)
{
	Unbind(wxEVT_IDLE, &wxCmdFrame::OnIdle, this);
}

void wxCmdFrame::Terminate()
{
	Unbind(wxEVT_IDLE, &wxCmdFrame::OnIdle, this);
	timer.Stop();
	delete p;
	if (adismiss) Close(true);
}

void wxCmdFrame::OnIdle(wxIdleEvent& event)
{
	p->HasInput();
}

void wxCmdFrame::OnTimer(wxTimerEvent& event)
{
	if (p) p->HasInput();
}

void wxCmdFrame::OnQuit(wxCommandEvent& WXUNUSED(event))
{
	Close(true);
}

void wxCmdFrame::OnDismiss(wxCommandEvent& event)
{
	if (wxProcess::Exists(pid)) wxProcess::Kill(pid, wxSIGKILL);
	Close(true);
}

void wxCmdFrame::OnKill(wxCommandEvent& event)
{
	if (wxProcess::Exists(pid)) {
		wxProcess::Kill(pid, wxSIGKILL);
		Append("WXCMD: KILL SIGNAL SENT TO PROCESS.");
	}
	else Append("WXCMD: PROCESS ALREADY DEAD.");
}


