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

#include <wx/string.h>
#include <iomanip>  // for std::setprecision and std::fixed

// TODO:
// use the one from mainframe.cpp
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

void ResultWindow::fetchResult(std::string fetchURL) {
    CURL* curl = curl_easy_init();
    CURLcode res;
    std::string readBuffer;

    curl_global_init(CURL_GLOBAL_DEFAULT);

    curl = curl_easy_init();
    if(curl) {
        curl_easy_setopt(curl, CURLOPT_URL, fetchURL.c_str());
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
                        this->grid->CreateGrid(jsonResponse.size(), 4);
                        int row = 0;
                        for (const auto& station : jsonResponse) {
                            std::string name = station["name"].asString();
                            std::string address = station["location"]["address"].asString();
                            std::string open = station["openingHours"][0]["from"].asString() + "-" + station["openingHours"][0]["to"].asString();
                            float price = station["prices"][0]["amount"].asFloat();

                            // Adjust the precision of the price
                            std::stringstream stream;
                            stream << std::fixed << std::setprecision(4) << price;
                            std::string priceStr = stream.str();

                            // Convert each std::string to wxString
                            wxString wxName(name.c_str(), wxConvUTF8);
                            wxString wxAddress(address.c_str(), wxConvUTF8);
                            wxString wxOpen(open.c_str(), wxConvUTF8);
                            wxString wxPrice(priceStr.c_str(), wxConvUTF8);

                            // Set each cell value
                            this->grid->SetCellValue(row, 0, wxName);
                            this->grid->SetCellValue(row, 1, wxAddress);
                            this->grid->SetCellValue(row, 2, wxOpen);
                            this->grid->SetCellValue(row, 3, wxPrice);

                            row++;
                        }
                    }
                } else {
                    wxString errMsg = wxString::Format("Failed to parse JSON: %s", parseErrors.c_str());
                    wxMessageBox(errMsg, "JSON Parse Error", wxOK | wxICON_ERROR, this);
                }
            } else {
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
    // change the text size of grid
    wxFont font(16, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL); 
    this->grid->SetDefaultCellFont(font);
    // Set column labels
    this->grid->SetColLabelValue(0,"Name");
    this->grid->SetColLabelValue(1,"Address");
    this->grid->SetColLabelValue(2,"Open");
    this->grid->SetColLabelValue(3,"Price");
    this->grid->SetColSize(0, 400); // Increase size for "Name" column
    this->grid->SetColSize(1, 250); // Increase size for "Name" column
    this->grid->SetColSize(2, 250 / 2); // Halve size for "Open" column
    this->grid->SetColSize(3, 250 / 2); // Halve size for "Price" column
}

ResultWindow::ResultWindow(wxWindow* parent, const wxString& title, const wxChoice& fuelsDropDown, wxCheckBox& nowOpenBox, wxString& regionCode)
    : wxFrame(parent, wxID_ANY, title, wxDefaultPosition, wxDefaultSize, wxDEFAULT_FRAME_STYLE & ~(wxRESIZE_BORDER | wxMAXIMIZE_BOX)) {
    
    wxString fuelType;
    wxString includeClosed;
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
    includeClosed = nowOpenBox.IsChecked() ? "false" : "true";
    std::string fetchURL = "https://api.e-control.at/sprit/1.0/search/gas-stations/by-region?code=";
    fetchURL += regionCode.c_str() + "&type=BL&fuelType=" + fuelType.ToStdString() + "&includeClosed=" + includeClosed;
    std::cout << fetchURL << std::endl;

    SetClientSize(1000, 1250);
    Center();
    this->grid = new wxGrid(this, -1, wxPoint(0, 415), wxSize(1200, 800));
    grid->SetDefaultRowSize(100,50);
    grid->SetDefaultColSize(230,100);
    fetchResult(fetchURL);
}
