#include "mainframe.hpp"

#include <wx/wx.h>
#include <wx/protocol/http.h>
#include <wx/url.h>
#include <wx/wx.h>
#include <curl/curl.h>
#include <curl/easy.h>
#include <wx/generic/grid.h>
#include <wx/frame.h>
#include <wx/wx.h>

#include <jsoncpp/json/json.h>
#include <locale>
#include <codecvt>



static size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* userp) {
    size_t realsize = size * nmemb;
    if (realsize > 0) {
        const char* charContents = static_cast<char*>(contents);
        if (charContents[0] != '\0') { // Check if the first character is not null
            userp->append(charContents, realsize);
        }
    }
    return realsize;
}

void MainFrame::OnChoiceSelected(wxCommandEvent& event) {
    regionsDropDownSelection = regionsDropDown->GetSelection();
    fetchCounties();
}


void MainFrame::fetchRegions() {
    CURL* curl = curl_easy_init();
    CURLcode res;
    std::string readBuffer;

    curl_global_init(CURL_GLOBAL_DEFAULT);

    curl = curl_easy_init();
    if(curl) {
        curl_easy_setopt(curl, CURLOPT_URL, "https://api.e-control.at/sprit/1.0/regions?includeCities=false");
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        curl_easy_setopt(curl, CURLOPT_USERAGENT, "libcurl-agent/1.0");

        // Set custom headers
        struct curl_slist* headers = NULL;
        headers = curl_slist_append(headers, "accept: application/json");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        res = curl_easy_perform(curl);


        if(res != CURLE_OK) {
            wxString errMsg(curl_easy_strerror(res));
            wxMessageBox(errMsg, "API Request Error", wxOK | wxICON_ERROR, this);
        } else {
            long response_code;
            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);

            if (response_code == 200) {
              Json::CharReaderBuilder readerBuilder;
              Json::Value jsonResponse;
              std::string parseErrors;

              std::istringstream readBufferStream(readBuffer);
                if (Json::parseFromStream(readerBuilder, readBufferStream, &jsonResponse, &parseErrors)) {
                    if (jsonResponse.isArray()) {
                        for (const auto& region : jsonResponse) {
                            std::string name = region["name"].asString();

                            // Convert the name to wxString with UTF-8 encoding
                            wxString wxName(name.c_str(), wxConvUTF8);

                            // Log the values using std::cout
                            std::cout << "Name: " << wxName.ToUTF8().data() << std::endl;
                            regions->Add(wxName);
                        }
                    }
                } else {
                  wxString errMsg = wxString::Format("Failed to parse JSON: %s", parseErrors.c_str());
                  wxMessageBox(errMsg, "JSON Parse Error", wxOK | wxICON_ERROR, this);
              } } else {
                wxString errMsg = wxString::Format("Server responded with code: %ld", response_code);
                wxMessageBox(errMsg, "API Response Error", wxOK | wxICON_ERROR, this);
            }
        }
        curl_easy_cleanup(curl);
        if (headers) {
            curl_slist_free_all(headers);
        }
    }

    curl_global_cleanup();
}


void MainFrame::fetchWelcomingText() {
    CURL* curl = curl_easy_init();
  std::string response;
  //std::vector res;
  // https://api.e-control.at/sprit/1.0/regions/units
  if (curl) {
      curl_easy_setopt(curl, CURLOPT_URL, "https://api.e-control.at/sprit/1.0/ping");
      curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
      curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
      CURLcode res = curl_easy_perform(curl);
      if (res != CURLE_OK) {
        // handle error
        wxLogStatus("curl_easy_perform() failed: %s\n",curl_easy_strerror(res));
      }

      curl_easy_cleanup(curl);
  }
  wxLogStatus("%s", response.c_str());
}


void MainFrame::fetchCounties() {
    CURL* curl = curl_easy_init();
    CURLcode res;
    std::string readBuffer;

    curl_global_init(CURL_GLOBAL_DEFAULT);

    curl = curl_easy_init();
    std::string fetchString = "https://api.e-control.at/sprit/1.0/search/gas-stations/by-region?code=";

    // Append the integer value to the fetchString
    fetchString += std::to_string(regionsDropDownSelection);
    fetchString += "&type=BL&fuelType=DIE&includeClosed=true";

    if(curl) {
        curl_easy_setopt(curl, CURLOPT_URL, fetchString.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        curl_easy_setopt(curl, CURLOPT_USERAGENT, "libcurl-agent/1.0");

        // Set custom headers
        struct curl_slist* headers = NULL;
        headers = curl_slist_append(headers, "accept: application/json");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        res = curl_easy_perform(curl);


        if(res != CURLE_OK) {
            wxString errMsg(curl_easy_strerror(res));
            wxMessageBox(errMsg, "API Request Error", wxOK | wxICON_ERROR, this);
        } else {
            long response_code;
            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);

            if (response_code == 200) {
              Json::CharReaderBuilder readerBuilder;
              Json::Value jsonResponse;
              std::string parseErrors;

              std::istringstream readBufferStream(readBuffer);
                if (Json::parseFromStream(readerBuilder, readBufferStream, &jsonResponse, &parseErrors)) {
                    if (jsonResponse.isArray()) {
                        for (const auto& region : jsonResponse) {
                            std::string name = region["name"].asString();

                            // Convert the name to wxString with UTF-8 encoding
                            wxString wxName(name.c_str(), wxConvUTF8);

                            // Log the values using std::cout
                            std::cout << "Name: " << wxName.ToUTF8().data() << std::endl;
                            regions->Add(wxName);
                        }
                    }
                } else {
                  wxString errMsg = wxString::Format("Failed to parse JSON: %s", parseErrors.c_str());
                  wxMessageBox(errMsg, "JSON Parse Error", wxOK | wxICON_ERROR, this);
              } } else {
                wxString errMsg = wxString::Format("Server responded with code: %ld", response_code);
                wxMessageBox(errMsg, "API Response Error", wxOK | wxICON_ERROR, this);
            }
        }
        curl_easy_cleanup(curl);
        if (headers) {
            curl_slist_free_all(headers);
        }
    }
    curl_global_cleanup();
}

void MainFrame::OnPaint(wxPaintEvent& event) {
    wxPaintDC dc(this);
    dc.DrawLine(0, 350, 1000, 350);
    dc.DrawLine(0, 400, 1000, 400);
}

MainFrame::MainFrame(const wxString& title)
  : wxFrame(nullptr, wxID_ANY, title) {
  wxPanel* panel = new wxPanel(this);

  wxStaticText* resultLabel = new wxStaticText(panel, wxID_ANY, "Results",
                                        wxPoint(450,308), wxSize(400,70));

  wxCheckBox* nowOpenBox = new wxCheckBox(panel, wxID_ANY, "NOW OPEN",
                                    wxPoint(520,50), wxSize(200,100));

  wxCheckBox* locationBox = new wxCheckBox(panel, wxID_ANY, "USE LOCATION",
                                      wxPoint(520,110), wxSize(200,100));

  wxButton* goButton = new wxButton(panel, wxID_ANY, "GO", wxPoint(850, 150),
                                  wxSize(100, 100));

  regions = new wxArrayString;
  regions->Add("REGION");

  wxArrayString county;
  county.Add("COUNTY");
  county.Add("Graz");
  wxChoice* countiesDropDown = new wxChoice(panel, wxID_ANY,wxPoint(100,200),wxSize(280,-1), county);
  countiesDropDown->Select(0);

  wxArrayString fuelType;
  fuelType.Add("FUEL TYPE");
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
  Bind(wxEVT_PAINT, &MainFrame::OnPaint, this);




  fetchRegions();
  fetchWelcomingText();
  regionsDropDown = new wxChoice(panel, wxID_ANY,wxPoint(100,110),wxSize(280,-1), *regions);
  regionsDropDown->Bind(wxEVT_CHOICE, &MainFrame::OnChoiceSelected, this);
  regionsDropDown->Select(0);

}







