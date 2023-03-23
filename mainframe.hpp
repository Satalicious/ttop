#ifndef MAINFRAME_HPP
#define MAINFRAME_HPP

#include <wx/wx.h>

class MainFrame : public wxFrame {
public:
  MainFrame(const wxString& title);
private:
  wxTextCtrl* textField = nullptr;
  void on_button_clear_clicked(wxCommandEvent& evt);
  void clear_last_char(wxCommandEvent& evt);
  void add_char(wxCommandEvent& evt, const wxString& character);
  void calculate(wxCommandEvent& evt);
  void on_textField_change(wxCommandEvent& evt);
  void MakeHttpRequest();
  void OnPaint(wxPaintEvent& event);

};

#endif  // MAINFRAME_HPP
