#include "resultwindow.hpp"
#include <wx/generic/grid.h>
#include <wx/wx.h>
#include <wx/protocol/http.h>
#include <wx/url.h>
#include <curl/curl.h>
#include <curl/easy.h>
#include <wx/generic/grid.h>

#include <jsoncpp/json/json.h>
#include <locale>
#include <codecvt>

/* void ResultWindow::fetchResult() {
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
} */


ResultWindow::ResultWindow(wxWindow* parent, const wxString& title, const wxChoice& fuelsDropDown, wxCheckBox& nowOpenBox, wxString& regionCode)
    : wxFrame(parent, wxID_ANY, title, wxDefaultPosition, wxDefaultSize, wxDEFAULT_FRAME_STYLE & ~(wxRESIZE_BORDER | wxMAXIMIZE_BOX)) {
    
    wxString fuelType;
    wxString isOpen;
    int selectedIndex = fuelsDropDown.GetCurrentSelection();

    switch (selectedIndex) {
        case 0:
            fuelType = "DIE";
            break;
        case 1:
            fuelType = "SUP";
            break;
        case 2:
            fuelType = "GAS";
            break;
        default:
            // if user selects none, search for diesel
            fuelType = "DIE";
            break;
    }
    //std::cout << fuelType.ToStdString() << std::endl;

    isOpen = nowOpenBox.IsChecked() ? "true" : "false";

    std::cout << regionCode.c_str() << std::endl;

    //fuelsDropDown.GetSelection();
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
}
