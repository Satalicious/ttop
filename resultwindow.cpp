#include "resultwindow.hpp"
#include <wx/generic/grid.h>
ResultWindow::ResultWindow(wxWindow* parent, const wxString& title, const wxArrayString& fuelTypes)
    : wxFrame(parent, wxID_ANY, title, wxDefaultPosition, wxDefaultSize, wxDEFAULT_FRAME_STYLE & ~(wxRESIZE_BORDER | wxMAXIMIZE_BOX)) {
        
    SetClientSize(1000, 1250);
    Center();
    
      wxGrid* grid = new wxGrid( this,-1, wxPoint( 0, 415 ), wxSize( 1200, 800 ) );
  // (100 rows and 10 columns in this example)
  grid->CreateGrid( 15, 4 );
  // We can set the sizes of individual rows and columns
  // in pixels
  //grid->SetRowSize( 0, 60 );
  //grid->SetColSize( 0, 200 );
  grid->SetDefaultRowSize(100,50);
  grid->SetDefaultColSize(230,100);
  grid->SetColLabelValue(0,"Name");
  grid->SetColLabelValue(1,"Distance");
  grid->SetColLabelValue(2,"Open");
  grid->SetColLabelValue(3,"Price");
  // And set grid cell contents as strings
  grid->SetCellValue( 0, 0, "wxGrid is good" );
  // We can specify that some cells are read->only
  grid->SetCellValue( 0, 3, "This is read->only" );
  grid->SetReadOnly( 0, 3 );
  // Colours can be specified for grid cell contents
  grid->SetCellValue(3, 3, "green on grey");
  grid->SetCellTextColour(3, 3, *wxGREEN);
  grid->SetCellBackgroundColour(3, 3, *wxLIGHT_GREY);
  // We can specify the some cells will store numeric
  // values rather than strings. Here we set grid column 5
  // to hold floating point values displayed with width of 6
  // and precision of 2
  grid->SetColFormatFloat(3, 2, 2);
  grid->SetCellValue(0, 3, "3.1415");
}
