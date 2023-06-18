#ifndef MAINFRAME_HPP
#define MAINFRAME_HPP
#include <wx/generic/grid.h>
#include <wx/wx.h>

class MainFrame : public wxFrame {
public:
  MainFrame(const wxString &title);
  void PopulateGridFromDatabase(wxGrid *grid);
  std::string FetchWelcomingText();

private:
  int regionsDropDownSelection;
  wxPanel *panel;
  wxArrayString *regions;
  wxArrayString *cities;
  wxArrayString fuelType;
  wxChoice *regionsDropDown;
  wxChoice *citiesDropDown;
  wxChoice *fuelsDropDown;
  wxButton *goButton;
  wxButton *goLocationButton;
  wxStaticText *favoritesLabel;
  wxGrid *grid;
  wxCheckBox *nowOpenBox;
  std::string cityCode;
  std::string latitude;
  std::string longitude;
  wxButton *refreshButton;
  void OnGoButtonClick(wxCommandEvent &event);
  void OnGoLocationButtonClick(wxCommandEvent &event);
  void On_button_clear_clicked(wxCommandEvent &evt);
  void On_textField_change(wxCommandEvent &evt);
  void OnPaint(wxPaintEvent &event);
  void FetchRegions();
  void FetchCities();
  void OnRegionSelected(wxCommandEvent &event);
  void OnCitySelected(wxCommandEvent &event);
  int GetRegionCodeByCity(std::string cityName);
};

#endif // MAINFRAME_HPP
