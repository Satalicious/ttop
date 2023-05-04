#ifndef MAINFRAME_HPP
#define MAINFRAME_HPP
#include <wx/generic/grid.h>

#include <wx/wx.h>

class MainFrame : public wxFrame {
public:
  MainFrame(const wxString& title);
private:
  int regionsDropDownSelection;
  wxPanel* panel;
  wxArrayString* regions;
  wxArrayString* postals;
  wxArrayString fuelType;
  wxChoice* regionsDropDown;
  wxChoice* postalDropDown;
  wxChoice* fuelsDropDown;
  wxTextCtrl* postalCodeEntry;
  wxButton* goButton;
  wxStaticText* resultLabel;
  
  wxGrid* grid;

  void OnFuelTypeSelected(wxCommandEvent& event);
  void OnGoButtonClick(wxCommandEvent& event);
  
  void on_button_clear_clicked(wxCommandEvent& evt);
  void on_textField_change(wxCommandEvent& evt);
  void OnPaint(wxPaintEvent& event);

  void fetchWelcomingText();
  void fetchRegions();
  void fetchPostals();
  void OnChoiceSelected(wxCommandEvent& event);

  wxCheckBox* nowOpenBox;
  void nowOpenBox_Changed(wxCommandEvent& event);

  wxCheckBox* locationBox;
  void locationBox_Changed(wxCommandEvent& event);

  void OnPostalCodeEntrySetFocus(wxFocusEvent& event);
  void OnPostalCodeEntryKillFocus(wxFocusEvent& event);
};

#endif  // MAINFRAME_HPP
