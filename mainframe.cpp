#include "mainframe.hpp"

#include <wx/wx.h>
#include <wx/protocol/http.h>
#include <wx/url.h>
#include <wx/wx.h>
#include <curl/curl.h>
#include <wx/generic/grid.h>
#include <wx/frame.h>

static size_t writeCallback(void *contents, size_t size, size_t nmemb, void *userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

void MainFrame::OnPaint(wxPaintEvent& event)
{
    wxPaintDC dc(this);
    dc.DrawLine(0, 350, 1000, 350);
    dc.DrawLine(0, 400, 1000, 400);
}


MainFrame::MainFrame(const wxString& title)
  : wxFrame(nullptr, wxID_ANY, title) {
  wxPanel* panel = new wxPanel(this);


  wxStaticText* resultLabel = new wxStaticText(panel, wxID_ANY, "Results",
                                        wxPoint(450,308), wxSize(400,70));

  wxCheckBox* nowOpenBox = new wxCheckBox(panel, wxID_ANY, "Now Open",
                                    wxPoint(520,40), wxSize(200,100));

  wxCheckBox* locationBox = new wxCheckBox(panel, wxID_ANY, "Use Location",
                                      wxPoint(520,110), wxSize(200,100));

  wxButton* goButton = new wxButton(panel, wxID_ANY, "GO", wxPoint(850, 150),
                                  wxSize(100, 100));



  wxArrayString regions;
  regions.Add("Region");
  wxChoice* regionsDropDown = new wxChoice(panel, wxID_ANY,wxPoint(100,110),wxSize(280,-1), regions);
  regionsDropDown->Select(0);

  wxArrayString county;
  county.Add("County");
  wxChoice* countiesDropDown = new wxChoice(panel, wxID_ANY,wxPoint(100,200),wxSize(280,-1), county);
  countiesDropDown->Select(0);

  wxArrayString fuelType;
  fuelType.Add("Fuel Type");
  wxChoice* fuelsDropDown = new wxChoice(panel, wxID_ANY,wxPoint(520,200),wxSize(280,-1), fuelType);
  fuelsDropDown->Select(0);

  wxGrid* grid = new wxGrid( panel,-1, wxPoint( 0, 354 ), wxSize( 1200, 800 ) );
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




  CreateStatusBar();
  wxLogStatus("2023, Djon & Schuh");



CURL* curl = curl_easy_init();
std::string response;

if (curl) {
    curl_easy_setopt(curl, CURLOPT_URL, "https://api.e-control.at/sprit/1.0/ping");
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        // handle error
    }


    curl_easy_cleanup(curl);
}

wxLogStatus("%s", response.c_str());


Bind(wxEVT_PAINT, &MainFrame::OnPaint, this);


}





