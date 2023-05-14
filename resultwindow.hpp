#include <wx/generic/grid.h>
#include <wx/wx.h>
class ResultWindow : public wxFrame {
public:
    ResultWindow(wxWindow* parent, const wxString& title, const wxChoice& fuelsDropDown, wxCheckBox& nowOpenBox, wxString& regionCode, std::string longitude, std::string latitude);

private:
  wxGrid *grid;
  void fetchResult(std::string fetchURL);
  wxButton *saveButton;
  //   void OnButtonClick(wxGridEvent &event);
  void OnButtonClick(wxCommandEvent &event);
};
