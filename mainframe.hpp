#ifndef MAINFRAME_HPP
#define MAINFRAME_HPP
#include <wx/generic/grid.h>

#include <wx/wx.h>

class MainFrame : public wxFrame {
public:
  MainFrame(const wxString& title);
private:
  wxArrayString* regions;
  wxArrayString* postals;
  wxArrayString fuelType;
  wxChoice* regionsDropDown;
  wxChoice* postalDropDown;
  wxChoice* fuelsDropDown;
  void OnFuelTypeSelected(wxCommandEvent& event);
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
  void fetchPostals();
  void OnChoiceSelected(wxCommandEvent& event);

  wxCheckBox* nowOpenBox;
  void nowOpenBox_Changed(wxCommandEvent& event);

  wxCheckBox* locationBox;
  void locationBox_Changed(wxCommandEvent& event);

  wxButton* goButton;
  wxGrid* grid;

};

#endif  // MAINFRAME_HPP
