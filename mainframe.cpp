#include "mainframe.hpp"
#include "resultwindow.hpp"

#include <curl/curl.h>
#include <curl/easy.h>
#include <sqlite3.h>
#include <wx/generic/grid.h>
#include <wx/protocol/http.h>
#include <wx/url.h>
#include <wx/wx.h>

#include <codecvt>
#include <jsoncpp/json/json.h>
#include <locale>

#include <wx/string.h>

void MainFrame::PopulateGridFromDatabase(wxGrid *grid) {
  // Open the SQLite database connection
  sqlite3 *db;
  int rc = sqlite3_open("sqlite.db", &db);
  if (rc != SQLITE_OK) {
    std::cerr << "Cannot open database: " << sqlite3_errmsg(db) << std::endl;
    sqlite3_close(db);
    return;
  }

  std::string sql =
      "SELECT name, address, opening_hours, price FROM gasStations;";

  sqlite3_stmt *stmt;
  rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
  if (rc != SQLITE_OK) {
    std::cerr << "SQL error: " << sqlite3_errmsg(db) << std::endl;
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    return;
  }

  int row = 0;
  while (sqlite3_step(stmt) == SQLITE_ROW) {
    const unsigned char *name = sqlite3_column_text(stmt, 0);
    const unsigned char *address = sqlite3_column_text(stmt, 1);
    const unsigned char *openingHours = sqlite3_column_text(stmt, 2);
    double price = sqlite3_column_double(stmt, 3);

    // Convert the retrieved data to wxString
    wxString wxName(reinterpret_cast<const char *>(name), wxConvUTF8);
    wxString wxAddress(reinterpret_cast<const char *>(address), wxConvUTF8);
    wxString wxOpeningHours(reinterpret_cast<const char *>(openingHours),
                            wxConvUTF8);
    wxString wxPrice = wxString::Format("%.2f", price);

    // Set the values in the grid
    grid->SetCellValue(row, 0, wxName);
    grid->SetCellValue(row, 1, wxAddress);
    grid->SetCellValue(row, 2, wxOpeningHours);
    grid->SetCellValue(row, 3, wxPrice);

    row++;
  }

  // Finalize the prepared statement and close the database connection
  sqlite3_finalize(stmt);
  sqlite3_close(db);
}

static size_t WriteCallback(void *contents, size_t size, size_t nmemb,
                            std::string *userp) {
  size_t realsize = size * nmemb;
  if (realsize > 0) {
    const char *charContents = static_cast<char *>(contents);
    if (charContents[0] != '\0') { // Check if the first character is not null
      userp->append(charContents, realsize);
    }
  }
  return realsize;
}

std::string getGeolocation() {
  CURL *curl;
  CURLcode res;
  std::string readBuffer;

  curl_global_init(CURL_GLOBAL_DEFAULT);
  curl = curl_easy_init();
  if (curl) {
    curl_easy_setopt(curl, CURLOPT_URL, "https://ipinfo.io/geo");
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
    res = curl_easy_perform(curl);

    if (res != CURLE_OK) {
      fprintf(stderr, "curl_easy_perform() failed: %s\n",
              curl_easy_strerror(res));
    } else {
      return readBuffer;
    }
    curl_easy_cleanup(curl);
  }
  curl_global_cleanup();
  return "";
}

void MainFrame::OnGoButtonClick(wxCommandEvent &event) {
  // render 2nd window here
  longitude = latitude = "";
  ResultWindow *resultWindow =
      new ResultWindow(this, "Result", *fuelsDropDown, *nowOpenBox, cityCode,
                       longitude, latitude);
  resultWindow->Show();
}

void MainFrame::OnGoLocationButtonClick(wxCommandEvent &event) {
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
  ResultWindow *resultWindow =
      new ResultWindow(this, "Result", *fuelsDropDown, *nowOpenBox, cityCode,
                       longitude, latitude);
  resultWindow->Show();
}
void MainFrame::OnRegionSelected(wxCommandEvent &event) {
  regionsDropDownSelection = regionsDropDown->GetSelection();
  if (regionsDropDownSelection > 0) {
    fetchCities();
    cityCode = "";
    citiesDropDown->SetSelection(0);
    wxString selectedCity = citiesDropDown->GetStringSelection();
    std::string selectedCityStr = selectedCity.ToStdString(); // convert wxString to std::string
    int cityCodeInt = getRegionCodeByCity(selectedCityStr);
    cityCode = wxString::Format(wxT("%d"), cityCodeInt);
    goButton = new wxButton(panel, wxID_ANY, "GO", wxPoint(850, 50), wxSize(100, 100));
    goButton->Bind(wxEVT_BUTTON, &MainFrame::OnGoButtonClick, this);
    goButton->Enable();
    citiesDropDown->Enable();
  } else {
    goButton->Disable();
    citiesDropDown->Disable();
  }
}

void MainFrame::OnCitySelected(wxCommandEvent &event) {
  wxString selectedCity = citiesDropDown->GetStringSelection();
  std::string selectedCityStr = selectedCity.ToStdString(); // convert wxString to std::string
  int cityCodeInt = getRegionCodeByCity(selectedCityStr);
  cityCode = wxString::Format(wxT("%d"), cityCodeInt);
}

int MainFrame::getRegionCodeByCity(std::string cityName) {
  CURL *curl = curl_easy_init();
  CURLcode res;
  std::string readBuffer;

  curl_global_init(CURL_GLOBAL_DEFAULT);

  curl = curl_easy_init();
  if (curl) {
    curl_easy_setopt(
        curl, CURLOPT_URL,
        "https://api.e-control.at/sprit/1.0/regions?includeCities=true");
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "libcurl-agent/1.0");

    // Set custom headers
    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, "accept: application/json");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    res = curl_easy_perform(curl);

    if (res != CURLE_OK) {
      wxString errMsg(curl_easy_strerror(res));
      wxMessageBox(errMsg, "API Request Error", wxOK | wxICON_ERROR, this);
      return -1;
    } else {
      long response_code;
      curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);

      if (response_code == 200) {
        Json::CharReaderBuilder readerBuilder;
        Json::Value jsonResponse;
        std::string parseErrors;

        std::istringstream readBufferStream(readBuffer);
        if (Json::parseFromStream(readerBuilder, readBufferStream,
                                  &jsonResponse, &parseErrors)) {
          if (jsonResponse.isArray()) {
            for (const auto &region : jsonResponse) {
              const auto &subRegions = region["subRegions"];
              if (subRegions.isArray()) {
                for (const auto &subRegion : subRegions) {
                  if (subRegion["name"].asString() == cityName) {
                    return subRegion["code"].asInt();
                  }
                }
              }
            }
          }
        } else {
          wxString errMsg =
              wxString::Format("Failed to parse JSON: %s", parseErrors.c_str());
          wxMessageBox(errMsg, "JSON Parse Error", wxOK | wxICON_ERROR, this);
          return -1;
        }
      } else {
        wxString errMsg =
            wxString::Format("Server responded with code: %ld", response_code);
        wxMessageBox(errMsg, "API Response Error", wxOK | wxICON_ERROR, this);
        return -1;
      }
    }
    curl_easy_cleanup(curl);
    if (headers) {
      curl_slist_free_all(headers);
    }
  }

  curl_global_cleanup();

  return -1; // Return -1 if the city was not found
}

void MainFrame::OnPaint(wxPaintEvent &event) {
  wxPaintDC dc(this);
  dc.DrawLine(0, 360, 1000, 360);
  dc.DrawLine(0, 410, 1000, 410);
}

void MainFrame::fetchWelcomingText() {
  CURL *curl = curl_easy_init();
  std::string response;
  if (curl) {
    curl_easy_setopt(curl, CURLOPT_URL,
                     "https://api.e-control.at/sprit/1.0/ping");
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
      // handle error
      wxLogStatus("curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
    }
    curl_easy_cleanup(curl);
  }

  // Create a temporary scope to modify and restore the font
  {
    wxStatusBar *statusBar = GetStatusBar();
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
  CURL *curl = curl_easy_init();
  CURLcode res;
  std::string readBuffer;

  curl_global_init(CURL_GLOBAL_DEFAULT);

  curl = curl_easy_init();
  if (curl) {
    curl_easy_setopt(
        curl, CURLOPT_URL,
        "https://api.e-control.at/sprit/1.0/regions?includeCities=false");
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "libcurl-agent/1.0");
    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, "accept: application/json");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    res = curl_easy_perform(curl);

    if (res != CURLE_OK) {
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
        if (Json::parseFromStream(readerBuilder, readBufferStream,
                                  &jsonResponse, &parseErrors)) {
          if (jsonResponse.isArray()) {
            for (const auto &region : jsonResponse) {
              std::string name = region["name"].asString();
              // Convert the name to wxString with UTF-8 encoding
              wxString wxName(name.c_str(), wxConvUTF8);
              regions->Add(wxName);
            }
          }
        } else {
          wxString errMsg =
              wxString::Format("Failed to parse JSON: %s", parseErrors.c_str());
          wxMessageBox(errMsg, "JSON Parse Error", wxOK | wxICON_ERROR, this);
        }
      } else {
        wxString errMsg =
            wxString::Format("Server responded with code: %ld", response_code);
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

void MainFrame::fetchCities() {
  citiesDropDown->Clear();
  cities->Clear();

  CURL *curl = curl_easy_init();
  CURLcode res;
  std::string readBuffer;

  curl_global_init(CURL_GLOBAL_DEFAULT);

  curl = curl_easy_init();
  if (curl) {
    curl_easy_setopt(
        curl, CURLOPT_URL,
        "https://api.e-control.at/sprit/1.0/regions?includeCities=false");
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "libcurl-agent/1.0");

    // Set custom headers
    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, "accept: application/json");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    res = curl_easy_perform(curl);

    if (res != CURLE_OK) {
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
        if (Json::parseFromStream(readerBuilder, readBufferStream,
                                  &jsonResponse, &parseErrors)) {
          if (jsonResponse.isArray()) {
            const auto &subRegions =
                jsonResponse[regionsDropDownSelection - 1]["subRegions"];

            if (subRegions.isArray()) {
              for (const auto &subRegion : subRegions) {
                std::string cityName = subRegion["name"].asString();

                // Convert the city name to wxString with UTF-8 encoding
                wxString wxCityName(cityName.c_str(), wxConvUTF8);

                cities->Add(wxCityName);
              }
              citiesDropDown->Clear();
              citiesDropDown->Set(*cities);
              citiesDropDown->Select(0);
            }
          }
        } else {
          wxString errMsg =
              wxString::Format("Failed to parse JSON: %s", parseErrors.c_str());
          wxMessageBox(errMsg, "JSON Parse Error", wxOK | wxICON_ERROR, this);
        }
      } else {
        wxString errMsg =
            wxString::Format("Server responded with code: %ld", response_code);
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

MainFrame::MainFrame(const wxString &title)
    : wxFrame(nullptr, wxID_ANY, title, wxDefaultPosition, wxDefaultSize,
              wxDEFAULT_FRAME_STYLE & ~wxRESIZE_BORDER) {
  panel = new wxPanel(this);

  CreateStatusBar();
  Bind(wxEVT_PAINT, &MainFrame::OnPaint, this);
  fetchWelcomingText();
  // TODO: Add a refresh button to refresh the content of the grid

  cities = new wxArrayString;
  regions = new wxArrayString;
  regions->Add("Select Region");
  // fetching regions and adding them to 'regions'
  fetchRegions();
  regionsDropDown = new wxChoice(panel, wxID_ANY, wxPoint(100, 70),
                                 wxSize(300, 70), *regions);
  // when a region gets selected, we fetch the postals
  regionsDropDown->Bind(wxEVT_CHOICE, &MainFrame::OnRegionSelected, this);
  regionsDropDown->Select(0);

  fuelType = *new wxArrayString;
  fuelType.Add("Diesel");
  fuelType.Add("Super");
  fuelType.Add("Gas");
  fuelsDropDown = new wxChoice(panel, wxID_ANY, wxPoint(520, 200),
                               wxSize(300, 70), fuelType);
  fuelsDropDown->SetSelection(0);

  citiesDropDown =
      new wxChoice(panel, wxID_ANY, wxPoint(100, 200), wxSize(300, 70));
  citiesDropDown->Bind(wxEVT_CHOICE, &MainFrame::OnCitySelected, this);

  nowOpenBox = new wxCheckBox(panel, wxID_ANY, "Now Open", wxPoint(520, 0),
                              wxSize(200, 200));

  goLocationButton = new wxButton(panel, wxID_ANY, "Location",
                                  wxPoint(850, 200), wxSize(100, 100));
  goLocationButton->Bind(wxEVT_BUTTON, &MainFrame::OnGoLocationButtonClick,
                         this);

  favoritesLabel = new wxStaticText(panel, wxID_ANY, "Favorites",
                                    wxPoint(450, 370), wxSize(400, 70));

  wxGrid *grid = new wxGrid(panel, -1, wxPoint(0, 415), wxSize(1200, 800));

  refreshButton = new wxButton(panel, wxID_ANY, "Refresh", wxPoint(150, 300),
                               wxSize(100, 50));

  // (100 rows and 10 columns in this example)
  grid->CreateGrid(15, 4);
  // We can set the sizes of individual rows and columns
  // in pixels
  // grid->SetRowSize( 0, 60 );
  // grid->SetColSize( 0, 200 );
  grid->SetDefaultRowSize(100, 50);
  grid->SetDefaultColSize(230, 100);
  grid->SetColLabelValue(0, "Name");
  grid->SetColLabelValue(1, "Address");
  grid->SetColLabelValue(2, "Open");
  grid->SetColLabelValue(3, "Price");
  refreshButton->Bind(wxEVT_BUTTON, [=](wxCommandEvent &event) {
    PopulateGridFromDatabase(grid);
    grid->ForceRefresh();
  });

  // Open the SQLite database connection

  // Refresh the grid to reflect the changes
  PopulateGridFromDatabase(grid);

  grid->ForceRefresh();
}
