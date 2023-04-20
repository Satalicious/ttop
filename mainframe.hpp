#ifndef MAINFRAME_HPP
#define MAINFRAME_HPP

#include <wx/wx.h>

class MainFrame : public wxFrame {
public:
  MainFrame(const wxString& title);
private:
  wxArrayString* regions;
  wxArrayString* postals;
  wxChoice* regionsDropDown;
  wxChoice* postalDropDown;
  int regionsDropDownSelection;

  void on_button_clear_clicked(wxCommandEvent& evt);
  void clear_last_char(wxCommandEvent& evt);
  void add_char(wxCommandEvent& evt, const wxString& character);
  void calculate(wxCommandEvent& evt);
  void on_textField_change(wxCommandEvent& evt);
  void OnPaint(wxPaintEvent& event);

  void fetchWelcomingText();
  void fetchRegions();
  void fetchPostals();
  void OnChoiceSelected(wxCommandEvent& event);

};

#endif  // MAINFRAME_HPP
