//
// $Source$
// $Revision$
// $Date$
//
// DESCRIPTION:
// Interface to pure-strategy equilibrium enumeration algorithms
//

#include "wx/wxprec.h"
#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif  // WX_PRECOMP
#include "wx/notebook.h"
#include "guishare/valinteger.h"
#include "guishare/wxstatus.h"
#include "gnullstatus.h"
#include "algenumpure.h"
#include "efgpure.h"  // for extensive-form algorithm
#include "psnesub.h"  // for efg via nfg
#include "nfgpure.h"  // for normal-form algorithm


//========================================================================
//                dialogEnumPure: Parameter-setting dialog
//========================================================================

const int idSTOPAFTER = 9000;

class panelEnumPure : public wxPanel {
private:
  wxRadioBox *m_stopAfter;
  wxStaticText *m_stopAfterText;
  wxTextCtrl *m_stopAfterCount;
  wxString m_stopAfterValue;

  // Event handlers
  void OnStopAfter(wxCommandEvent &);

public:
  panelEnumPure(wxWindow *p_parent);

  virtual bool Validate(void);

  int StopAfter(void) const;

  DECLARE_EVENT_TABLE()
};

BEGIN_EVENT_TABLE(panelEnumPure, wxPanel)
  EVT_RADIOBOX(idSTOPAFTER, panelEnumPure::OnStopAfter)
END_EVENT_TABLE()

panelEnumPure::panelEnumPure(wxWindow *p_parent)
  : wxPanel(p_parent, -1), m_stopAfterValue("1")
{
  SetAutoLayout(true);

  wxBoxSizer *topSizer = new wxBoxSizer(wxHORIZONTAL);

  wxString stopAfterChoices[] = { "Find one", "Find two",
				  "Find n", "Find all" };
  m_stopAfter = new wxRadioBox(this, idSTOPAFTER, "Number of equilibria",
			       wxDefaultPosition, wxDefaultSize,
			       4, stopAfterChoices, 1, wxRA_SPECIFY_COLS);
  m_stopAfter->SetSelection(0);
  topSizer->Add(m_stopAfter, 0, wxALL, 5);

  wxBoxSizer *stopAfterSizer = new wxBoxSizer(wxVERTICAL);
  m_stopAfterText = new wxStaticText(this, wxID_STATIC,
				     "Number of equilibria to find");
  stopAfterSizer->Add(m_stopAfterText, 0, wxALL, 5);
  m_stopAfterCount = new wxTextCtrl(this, -1, "",
				    wxDefaultPosition, wxDefaultSize, 0,
				    gIntegerValidator(&m_stopAfterValue, 1),
				    "Stop After");
  m_stopAfterCount->SetValue(m_stopAfterValue);
  stopAfterSizer->Add(m_stopAfterCount, 0, wxALL | wxCENTER, 5);
  topSizer->Add(stopAfterSizer, 0, wxALL | wxCENTER, 0);

  SetSizer(topSizer);
  topSizer->Fit(this);
  topSizer->SetSizeHints(this);
  Layout();

  m_stopAfterText->Show(m_stopAfter->GetSelection() == 2);
  m_stopAfterCount->Show(m_stopAfter->GetSelection() == 2);
}

void panelEnumPure::OnStopAfter(wxCommandEvent &)
{
  m_stopAfterText->Show(m_stopAfter->GetSelection() == 2);
  m_stopAfterCount->Show(m_stopAfter->GetSelection() == 2);
}

int panelEnumPure::StopAfter(void) const
{
  switch (m_stopAfter->GetSelection()) {
  case 0:
    return 1;
  case 1:
    return 2;
  case 2:
    return atoi(m_stopAfterCount->GetValue());
  case 3:
  default:
    return 0;
  }
}

bool panelEnumPure::Validate(void)
{
  return (m_stopAfter->GetSelection() != 2 || wxWindow::Validate());
}

class dialogEnumPure : public wxDialog {
private:
  wxNotebook *m_notebook;
  panelEnumPure *m_algorithm;

  // Event handlers
  void OnOK(wxCommandEvent &);

public:
  dialogEnumPure(wxWindow *p_parent);
  bool SelectSolutions(void) const { return true; }
  int StopAfter(void) const { return m_algorithm->StopAfter(); }

  DECLARE_EVENT_TABLE()
};

BEGIN_EVENT_TABLE(dialogEnumPure, wxDialog)
  EVT_BUTTON(wxID_OK, dialogEnumPure::OnOK)
END_EVENT_TABLE()

dialogEnumPure::dialogEnumPure(wxWindow *p_parent)
  : wxDialog(p_parent, -1, "EnumPureSolve Parameters")
{
  SetAutoLayout(true);

  m_notebook = new wxNotebook(this, -1);
  m_algorithm = new panelEnumPure(m_notebook);
  m_notebook->AddPage(m_algorithm, "Algorithm");
  wxPanel *tracePanel = new wxPanel(m_notebook, -1);
  m_notebook->AddPage(tracePanel, "Trace");

  wxNotebookSizer *notebookSizer = new wxNotebookSizer(m_notebook);

  wxButton *okButton = new wxButton(this, wxID_OK, "OK");
  okButton->SetDefault();
  wxButton *cancelButton = new wxButton(this, wxID_CANCEL, "Cancel");
  wxButton *helpButton = new wxButton(this, wxID_HELP, "Help");

  wxBoxSizer *buttonSizer = new wxBoxSizer(wxHORIZONTAL);
  buttonSizer->Add(okButton, 0, wxALL, 5);
  buttonSizer->Add(cancelButton, 0, wxALL, 5);
  buttonSizer->Add(helpButton, 0, wxALL, 5);

  wxBoxSizer *topSizer = new wxBoxSizer(wxVERTICAL);
  topSizer->Add(notebookSizer, 1, wxALL | wxEXPAND, 5);
  topSizer->Add(buttonSizer, 0, wxALL | wxCENTER, 5);
  SetSizer(topSizer);
  topSizer->Fit(this);
  topSizer->SetSizeHints(this);
  Layout();
}

void dialogEnumPure::OnOK(wxCommandEvent &)
{
  if (!m_algorithm->Validate()) {
    m_notebook->SetSelection(0);
    return;
  }

  EndModal(wxID_OK);
}

//========================================================================
//                         Algorithm classes
//========================================================================

class guiEnumPureEfg : public efgEnumPure {
private:
  wxWindow *m_parent;
  bool m_selectSolutions;

  void SelectSolutions(int, const FullEfg &, gList<BehavSolution> &);

public:
  guiEnumPureEfg(wxWindow *p_parent, int p_stopAfter)
    : efgEnumPure(p_stopAfter), m_parent(p_parent) { }
  virtual ~guiEnumPureEfg() { }

  void SetSelectSolutions(bool p_selectSolutions)
    { m_selectSolutions = p_selectSolutions; }
};

void guiEnumPureEfg::SelectSolutions(int, const FullEfg &p_efg,
				     gList<BehavSolution> &p_solutions)
{
  if (m_selectSolutions) {
    wxMessageBox("Someday, you'll be able to select solutions.\n"
		 "For now, you just get them all!");
  }
}

class guiEnumPureNfg : public efgEnumPureNfgSolve {
private:
  wxWindow *m_parent;
  bool m_selectSolutions;

  void SelectSolutions(int, const FullEfg &, gList<BehavSolution> &);

public:
  guiEnumPureNfg(wxWindow *p_parent, int p_stopAfter)
    : efgEnumPureNfgSolve(p_stopAfter), m_parent(p_parent) { }
  virtual ~guiEnumPureNfg() { }

  void SetSelectSolutions(bool p_selectSolutions)
    { m_selectSolutions = p_selectSolutions; }
};

void guiEnumPureNfg::SelectSolutions(int, const FullEfg &p_efg,
				     gList<BehavSolution> &p_solutions)
{
  if (m_selectSolutions) {
    wxMessageBox("Someday, you'll be able to select solutions.\n"
		 "For now, you just get them all!");
  }
}

//========================================================================
//                         Top-level functions
//========================================================================

bool EnumPureEfg(wxWindow *p_parent,
		 const EFSupport &p_support, gList<BehavSolution> &p_solutions)
{
  dialogEnumPure dialog(p_parent);
  if (dialog.ShowModal() == wxID_OK) {
    guiEnumPureEfg algorithm(p_parent, dialog.StopAfter());
    algorithm.SetSelectSolutions(dialog.SelectSolutions());
    wxStatus status(p_parent, "EnumPureSolve Progress");
    try {
      p_solutions = algorithm.Solve(p_support, status);
    }
    catch (gSignalBreak &) { }
    return true;
  }
  return false;
}

bool EnumPureNfg(wxWindow *p_parent,
		 const EFSupport &p_support, gList<BehavSolution> &p_solutions)
{
  dialogEnumPure dialog(p_parent);
  if (dialog.ShowModal() == wxID_OK) {
    guiEnumPureNfg algorithm(p_parent, dialog.StopAfter());
    algorithm.SetSelectSolutions(dialog.SelectSolutions());
    wxStatus status(p_parent, "EnumPureSolve Progress");
    try {
      p_solutions = algorithm.Solve(p_support, status);
    }
    catch (gSignalBreak &) { }
    return true;
  }
  return false;
}

bool EnumPureNfg(wxWindow *p_parent,
		 const NFSupport &p_support, gList<MixedSolution> &p_solutions)
{
  dialogEnumPure dialog(p_parent);
  if (dialog.ShowModal() == wxID_OK) {
    nfgEnumPure algorithm;
    algorithm.SetStopAfter(dialog.StopAfter());
    wxStatus status(p_parent, "EnumPureSolve Progress");
    try {
      algorithm.Solve(p_support, status, p_solutions);
    }
    catch (gSignalBreak &) { }
    return true;
  }
  return false;
}

