#include "app.hpp"
#include "mainframe.hpp"
#include <wx/wx.h>

bool App::OnInit() {
  MainFrame *main_frame =
      new MainFrame("TTOP - find the cheapest gas stations in your area! ");
  main_frame->SetClientSize(1000, 1200);
  main_frame->Center();
  main_frame->Show();
  return true;
}

wxIMPLEMENT_APP(App);
