#include "mainframe.hpp"
#include "resultwindow.hpp"

#include <wx/wx.h>
#include <wx/protocol/http.h>
#include <wx/url.h>
#include <curl/curl.h>
#include <curl/easy.h>
#include <wx/generic/grid.h>

#include <jsoncpp/json/json.h>
#include <locale>
#include <codecvt>

#include <wx/string.h>

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

std::string getGeolocation() {
    CURL* curl;
    CURLcode res;
    std::string readBuffer;

    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();
    if(curl) {
        curl_easy_setopt(curl, CURLOPT_URL, "https://ipinfo.io/geo");
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        res = curl_easy_perform(curl);

        if(res != CURLE_OK) {
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        } else {
            return readBuffer;
        }
        curl_easy_cleanup(curl);
    }
    curl_global_cleanup();
    return "";
}

void MainFrame::OnPostalCodeEntrySetFocus(wxFocusEvent& event) {
    wxString currentValue = postalCodeEntry->GetValue();
    if (currentValue == "Enter postal code") {
        postalCodeEntry->SetValue("");  // Clear the default value
    }
    event.Skip();  // Allow other handlers to process the event
}

void MainFrame::OnPostalCodeEntryKillFocus(wxFocusEvent& event) {
    wxString currentValue = postalCodeEntry->GetValue();
    if (currentValue.IsEmpty()) {
        postalCodeEntry->SetValue("Enter postal code");  // Restore the default value
    }
    event.Skip();  // Allow other handlers to process the event
}

void MainFrame::OnGoButtonClick(wxCommandEvent& event) {
    // render 2nd window here
    ResultWindow* resultWindow = new ResultWindow(this, "Result", *fuelsDropDown, *nowOpenBox, *regionCode, longitude, latitude);
    resultWindow->Show();
}

void MainFrame::OnGoLocationButtonClick(wxCommandEvent& event) {
      std::string response = getGeolocation();
  Json::Value jsonData;
  Json::Reader jsonReader;
  if (jsonReader.parse(response, jsonData)) {
    std::string loc = jsonData["loc"].asString();
    size_t commaPos = loc.find(',');
    latitude = loc.substr(0, commaPos);
    longitude = loc.substr(commaPos + 1);
    std::cout << "Latitude: " << latitude << "\n";
    std::cout << "Longitude: " << longitude << "\n";
} else {
    std::cout << "Unable to get Location!" << std::endl;
}
    ResultWindow* resultWindow = new ResultWindow(this, "Result", *fuelsDropDown, *nowOpenBox, *regionCode, longitude, latitude);
    resultWindow->Show();
}

void MainFrame::OnRegionSelected(wxCommandEvent& event) {
    regionsDropDownSelection = regionsDropDown->GetSelection();
    if(regionsDropDownSelection > 0) {
        fetchPostals();
        int intValue = regionsDropDownSelection;
        regionCode = new wxString(wxString::Format(wxT("%d"), intValue));
    } else {
        postalDropDown->Clear();
    }
    // Create a text field for the user to enter a postal code
    postalCodeEntry = new wxTextCtrl(panel, wxID_ANY, "Enter postal code", wxPoint(100, 270), wxSize(280, 60));
    // Add event handlers for the postal code entry text field
    postalCodeEntry->Bind(wxEVT_SET_FOCUS, &MainFrame::OnPostalCodeEntrySetFocus, this);
    postalCodeEntry->Bind(wxEVT_KILL_FOCUS, &MainFrame::OnPostalCodeEntryKillFocus, this);
    
    // Create a static text to display "OR"
    wxStaticText* orText = new wxStaticText(panel, wxID_ANY, "OR", wxPoint(220, 215), wxSize(200,200));
    
    goButton = new wxButton(panel, wxID_ANY, "GO", wxPoint(850, 50), wxSize(100, 100));
    goButton->Bind(wxEVT_BUTTON, &MainFrame::OnGoButtonClick, this);
}

void MainFrame::OnPaint(wxPaintEvent& event) {
    wxPaintDC dc(this);
    dc.DrawLine(0, 360, 1000, 360);
    dc.DrawLine(0, 410, 1000, 410);
}

void MainFrame::fetchWelcomingText() {
    CURL* curl = curl_easy_init();
    std::string response;
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

    // Create a temporary scope to modify and restore the font
    {
        wxStatusBar* statusBar = GetStatusBar();
        if (statusBar) {
            wxFont originalFont = statusBar->GetFont();
            int fontSize = 16;
            wxFont newFont = originalFont;
            newFont.SetPointSize(fontSize);
            statusBar->SetFont(newFont);
            wxLogStatus("%s\t\t  Timothy & sata", response.c_str());
        } else {
            wxLogStatus("Connection to API failed :(");
        }
    }
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

void MainFrame::fetchPostals() {
    postalDropDown->Clear();
    postals->Clear();

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
                        const auto& subRegions = jsonResponse[regionsDropDownSelection - 1]["subRegions"];

                        if (subRegions.isArray()) {
                            for (const auto& subRegion : subRegions) {
                                const auto& postalCodes = subRegion["postalCodes"];

                                if (postalCodes.isArray()) {
                                    for (const auto& postalCode : postalCodes) {
                                        std::string postalCodeStr = postalCode.asString();

                                        // Convert the postal code to wxString with UTF-8 encoding
                                        wxString wxPostalCode(postalCodeStr.c_str(), wxConvUTF8);

                                        std::cout << wxPostalCode.ToUTF8().data() << std::endl;

                                        postals->Add(wxPostalCode);

                                    }
                                }
                            }
                            postalDropDown->Clear();
                            postalDropDown->Set(*postals);
                            postalDropDown->Select(0);
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

MainFrame::MainFrame(const wxString& title)
  : wxFrame(nullptr, wxID_ANY, title, wxDefaultPosition, wxDefaultSize, wxDEFAULT_FRAME_STYLE & ~wxRESIZE_BORDER) {
  panel = new wxPanel(this);

  CreateStatusBar();
  Bind(wxEVT_PAINT, &MainFrame::OnPaint, this);
  fetchWelcomingText();

  postals = new wxArrayString;
  regions = new wxArrayString;
  regions->Add("Select Region");
  // fetching regions and adding them to 'regions'
  fetchRegions();
  regionsDropDown = new wxChoice(panel, wxID_ANY,wxPoint(100,50),wxSize(280,60), *regions);
  // when a region gets selected, we fetch the postals
  regionsDropDown->Bind(wxEVT_CHOICE, &MainFrame::OnRegionSelected, this);
  regionsDropDown->Select(0);

  fuelType = *new wxArrayString;
  fuelType.Add("Diesel");
  fuelType.Add("Super");
  fuelType.Add("Gas");
  fuelsDropDown = new wxChoice(panel, wxID_ANY, wxPoint(520, 270), wxSize(280, 60), fuelType);
  fuelsDropDown->SetSelection(0);

  postalDropDown = new wxChoice(panel, wxID_ANY,wxPoint(100,140),wxSize(280,60));

  nowOpenBox = new wxCheckBox(panel, wxID_ANY, "Now Open", wxPoint(520,120), wxSize(200,100));

  favoritesLabel = new wxStaticText(panel, wxID_ANY, "Favorites", wxPoint(450,370), wxSize(400,70));
  
  wxGrid* grid = new wxGrid( panel,-1, wxPoint( 0, 415 ), wxSize( 1200, 800 ) );
  // (100 rows and 10 columns in this example)
  grid->CreateGrid( 15, 4 );
  // We can set the sizes of individual rows and columns
  // in pixels
  //grid->SetRowSize( 0, 60 );
  //grid->SetColSize( 0, 200 );
  grid->SetDefaultRowSize(100,50);
  grid->SetDefaultColSize(230,100);
  grid->SetColLabelValue(0,"Name");
  grid->SetColLabelValue(1,"Address");
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
  
  
goLocationButton = new wxButton(panel, wxID_ANY, "Location", wxPoint(850, 200), wxSize(100, 100));
goLocationButton->Bind(wxEVT_BUTTON, &MainFrame::OnGoLocationButtonClick, this);

}

