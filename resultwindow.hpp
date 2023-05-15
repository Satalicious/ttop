#include <wx/generic/grid.h>
#include <wx/wx.h>
class ResultWindow : public wxFrame {
public:
    ResultWindow(wxWindow* parent, const wxString& title, const wxChoice& fuelsDropDown, wxCheckBox& nowOpenBox, std::string cityCode, std::string longitude, std::string latitude);

private:
  wxGrid *grid;
  void FetchResult(std::string fetchURL);
  wxButton *saveButton;
  //   void OnButtonClick(wxGridEvent &event);
  void OnButtonClick(wxCommandEvent &event);
};
