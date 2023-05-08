#include <wx/wx.h>
#include <wx/generic/grid.h>
class ResultWindow : public wxFrame // Change wxDialog to wxFrame
{
public:
    ResultWindow(wxWindow* parent, const wxString& title, const wxChoice& fuelsDropDown, wxCheckBox& nowOpenBox, wxString& regionCode);

private:
    wxGrid* grid;
    void fetchResult();
    
};
