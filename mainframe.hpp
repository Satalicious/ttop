#ifndef MAINFRAME_HPP
#define MAINFRAME_HPP
#include <wx/generic/grid.h>

#include <wx/wx.h>

class MainFrame : public wxFrame {
public:
  MainFrame(const wxString& title);
private:
  wxArrayString* regions;
  wxChoice* regionsDropDown;
  int regionsDropDownSelection;

  void on_button_clear_clicked(wxCommandEvent& evt);
  void clear_last_char(wxCommandEvent& evt);
  void add_char(wxCommandEvent& evt, const wxString& character);
  void calculate(wxCommandEvent& evt);
  void on_textField_change(wxCommandEvent& evt);
  void OnPaint(wxPaintEvent& event);

  wxStaticText* resultLabel;
  wxPanel* panel;
  void fetchWelcomingText();
  void fetchRegions();
  void fetchCounties();
  void OnChoiceSelected(wxCommandEvent& event);
  wxCheckBox* nowOpenBox;
  wxCheckBox* locationBox;
  wxButton* goButton;
  wxChoice* fuelsDropDown;
  wxGrid* grid;
  wxChoice* countiesDropDown;

};

#endif  // MAINFRAME_HPP
