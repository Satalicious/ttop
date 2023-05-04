#include "mainframe.hpp"

#include <wx/wx.h>
#include <wx/protocol/http.h>
#include <wx/url.h>
#include <curl/curl.h>
#include <curl/easy.h>
#include <wx/generic/grid.h>
#include <wx/frame.h>

#include <jsoncpp/json/json.h>
#include <locale>
#include <codecvt>

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

void MainFrame::nowOpenBox_Changed(wxCommandEvent& event) {
  if (nowOpenBox->IsChecked()) {
    std::cout << "nowopen: checked" << std::endl;
  } else {
    std::cout << "nowopen: unchecked" << std::endl;
  }
}

void MainFrame::locationBox_Changed(wxCommandEvent& event) {
    // we wont need this function in future but the IsChecked()
    if(locationBox->IsChecked()) {
        std::cout << "locationbox: checked" << std::endl;
    } else {
        std::cout << "locationbox: unchecked" << std::endl;
    }
}

void MainFrame::OnFuelTypeSelected(wxCommandEvent& event) {
    int selectedFuelTypeIndex = fuelsDropDown->GetSelection();

    switch (selectedFuelTypeIndex) {
        case 1:
            std::cout << "DIE is selected" << std::endl;
            break;
        case 2:
            std::cout << "SUP is selected" << std::endl;
            break;
        case 3:
            std::cout << "GAS is selected" << std::endl;
            break;
        default:
            std::cout << "No fuel type selected" << std::endl;
            break;
    }
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
            wxLogStatus("%s\t\t\t  Djon & sata", response.c_str());
        } else {
            wxLogStatus("Connection to API failed :(");
        }
    }
}

void MainFrame::OnChoiceSelected(wxCommandEvent& event) {
    regionsDropDownSelection = regionsDropDown->GetSelection();
    if(regionsDropDownSelection > 0) {
        fetchPostals();
    } else {
        postalDropDown->Clear();
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

void MainFrame::OnPaint(wxPaintEvent& event) {
    wxPaintDC dc(this);
    dc.DrawLine(0, 550, 1000, 550);
    dc.DrawLine(0, 600, 1000, 600);
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
  regionsDropDown->Bind(wxEVT_CHOICE, &MainFrame::OnChoiceSelected, this);
  regionsDropDown->Select(0);

  fuelType = *new wxArrayString;
  fuelType.Add("Select Fuel");
  fuelType.Add("Diesel");
  fuelType.Add("Super");
  fuelType.Add("Gas");
  fuelsDropDown = new wxChoice(panel, wxID_ANY, wxPoint(520, 270), wxSize(280, 60), fuelType);
  fuelsDropDown->Bind(wxEVT_CHOICE, &MainFrame::OnFuelTypeSelected, this);
  fuelsDropDown->SetSelection(0);

  postalDropDown = new wxChoice(panel, wxID_ANY,wxPoint(100,140),wxSize(280,60));
  
  // Create a static text to display "OR"
  wxStaticText* orText = new wxStaticText(panel, wxID_ANY, "OR", wxPoint(220, 220), wxSize(200,200));
  // Create a text field for the user to enter a postal code
  postalCodeEntry = new wxTextCtrl(panel, wxID_ANY, "Enter postal code", wxPoint(100, 270), wxSize(280, 60));
  // Add event handlers for the postal code entry text field
  postalCodeEntry->Bind(wxEVT_SET_FOCUS, &MainFrame::OnPostalCodeEntrySetFocus, this);
  postalCodeEntry->Bind(wxEVT_KILL_FOCUS, &MainFrame::OnPostalCodeEntryKillFocus, this);

  nowOpenBox = new wxCheckBox(panel, wxID_ANY, "NOW OPEN", wxPoint(520,30), wxSize(200,100));
  nowOpenBox->Bind(wxEVT_CHECKBOX, &MainFrame::nowOpenBox_Changed, this);

  locationBox = new wxCheckBox(panel, wxID_ANY, "USE LOCATION", wxPoint(520,120), wxSize(200,100));
  locationBox->Bind(wxEVT_CHECKBOX, &MainFrame::locationBox_Changed, this);

  goButton = new wxButton(panel, wxID_ANY, "GO", wxPoint(850, 130), wxSize(100, 100));
  resultLabel = new wxStaticText(panel, wxID_ANY, "Results", wxPoint(450,508), wxSize(400,70));
}
