#include "resultwindow.hpp"

ResultWindow::ResultWindow(wxWindow* parent, const wxString& title, const wxArrayString& fuelTypes)
    : wxFrame(parent, wxID_ANY, title, wxDefaultPosition, wxDefaultSize, wxDEFAULT_FRAME_STYLE & ~(wxRESIZE_BORDER | wxMAXIMIZE_BOX)) {
        
    SetClientSize(1000, 1200);
    Center();
    wxListBox* listBox = new wxListBox(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, fuelTypes);
}
