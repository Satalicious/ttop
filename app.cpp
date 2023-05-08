#include "app.hpp"
#include <wx/wx.h>
#include "mainframe.hpp"

bool App::OnInit() {
  MainFrame* main_frame = new MainFrame("TTOP");
  main_frame->SetClientSize(1000, 1200);
  main_frame->Center();
  main_frame->Show();
  return true;
}

wxIMPLEMENT_APP(App);
