#ifndef MAINFRAME_HPP
#define MAINFRAME_HPP
#include <wx/generic/grid.h>
#include <wx/wx.h>

class MainFrame : public wxFrame {
public:
  MainFrame(const wxString &title);

private:
  int regionsDropDownSelection;
  wxPanel* panel;
  wxArrayString* regions;
  wxString* regionCode;
  wxArrayString* cities;
  wxArrayString fuelType;
  wxChoice* regionsDropDown;
  wxChoice* citiesDropDown;
  wxChoice* fuelsDropDown;
  wxButton* goButton;
  wxButton* goLocationButton;
  wxButton* refreshButton;
  wxStaticText* favoritesLabel;
  wxGrid* grid;
  wxCheckBox* nowOpenBox;
  std::string latitude;
  std::string longitude;
  void PopulateGridFromDatabase(wxGrid *grid);
  void OnGoButtonClick(wxCommandEvent& event);
  void OnGoLocationButtonClick(wxCommandEvent& event);
  void on_button_clear_clicked(wxCommandEvent& evt);
  void on_textField_change(wxCommandEvent& evt);
  void OnPaint(wxPaintEvent& event);
  void fetchWelcomingText();
  void fetchRegions();
  void fetchCities();
  void OnRegionSelected(wxCommandEvent& event);
  int getRegionCodeByCity(std::string cityName);
};

#endif // MAINFRAME_HPP
