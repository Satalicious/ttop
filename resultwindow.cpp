#include "resultwindow.hpp"
#include <codecvt>
#include <curl/curl.h>
#include <curl/easy.h>
#include <json/json.h>
#include <locale>
#include <sqlite3.h>
#include <wx/generic/grid.h>
#include <wx/protocol/http.h>
#include <wx/url.h>
#include <wx/wx.h>

#include <iomanip> // for std::setprecision and std::fixed
#include <wx/string.h>

class CustomButton : public wxButton {
public:
  CustomButton(wxWindow *parent, wxWindowID id, const wxString &label,
               const wxPoint &pos = wxDefaultPosition,
               const wxSize &size = wxDefaultSize, long style = 0,
               const wxValidator &validator = wxDefaultValidator,
               const wxString &name = wxButtonNameStr)
      : wxButton(parent, id, label, pos, size, style, validator, name) {}

  wxGrid *m_grid;

private:
  void OnButtonClicked(wxCommandEvent &event);

  DECLARE_EVENT_TABLE()
};

void CustomButton::OnButtonClicked(wxCommandEvent &event) {
  std::cout << "Clicked Me" << std::endl;
  if (m_grid) {
    int row = m_grid->GetGridCursorRow();
    int numCols = m_grid->GetNumberCols();

    // Open the SQLite database connection
    sqlite3 *db;
    int rc = sqlite3_open("sqlite.db", &db);
    if (rc != SQLITE_OK) {
      // Handle the error if the connection fails
      std::cerr << "Cannot open database: " << sqlite3_errmsg(db) << std::endl;
      sqlite3_close(db);
      return;
    }

    // Create the gasStations table if it doesn't exist
    std::string createTableQuery =
        "CREATE TABLE IF NOT EXISTS gasStations (name TEXT, address TEXT, "
        "opening_hours TEXT, price TEXT);";
    rc = sqlite3_exec(db, createTableQuery.c_str(), 0, 0, 0);
    if (rc != SQLITE_OK) {
      // Handle the error if the execution fails
      std::cerr << "SQL error: " << sqlite3_errmsg(db) << std::endl;
      sqlite3_close(db);
      return;
    }

    // Prepare the SQL statement
    std::string sql =
        "INSERT INTO gasStations (name, address, opening_hours, price) VALUES ";
    sql += "(";

    for (int col = 0; col < numCols - 1; ++col) {
      wxString cellValue = m_grid->GetCellValue(row, col);
      sql += "'" + std::string(cellValue.ToStdString()) + "'";

      if (col < numCols - 2) {
        sql += ", ";
      }
    }

    sql += ");";

    // Execute the SQL statement
    rc = sqlite3_exec(db, sql.c_str(), 0, 0, 0);
    if (rc != SQLITE_OK) {
      // Handle the error if the execution fails
      std::cerr << "SQL error: " << sqlite3_errmsg(db) << std::endl;
    }

    // Close the database connection
    sqlite3_close(db);
  }
}

BEGIN_EVENT_TABLE(CustomButton, wxButton)
EVT_BUTTON(wxID_ANY, CustomButton::OnButtonClicked)
END_EVENT_TABLE()

class ButtonCellRenderer : public wxGridCellRenderer {
public:
  virtual void Draw(wxGrid &grid, wxGridCellAttr &attr, wxDC &dc,
                    const wxRect &rect, int row, int col, bool isSelected);
  virtual wxSize GetBestSize(wxGrid &grid, wxGridCellAttr &attr, wxDC &dc,
                             int row, int col);
  virtual wxGridCellRenderer *Clone() const;
};

void ButtonCellRenderer::Draw(wxGrid &grid, wxGridCellAttr &attr, wxDC &dc,
                              const wxRect &rect, int row, int col,
                              bool isSelected) {
  dc.SetBrush(*wxLIGHT_GREY_BRUSH);
  dc.SetPen(*wxTRANSPARENT_PEN);
  dc.DrawRectangle(rect);

  wxString label = wxT("Press");
  wxCoord width, height;
  dc.GetTextExtent(label, &width, &height);
  int x = rect.x + (rect.width - width) / 2;
  int y = rect.y + (rect.height - height) / 2;
  dc.DrawText(label, x, y);
}

wxSize ButtonCellRenderer::GetBestSize(wxGrid &grid, wxGridCellAttr &attr,
                                       wxDC &dc, int row, int col) {
  return wxSize(80, 25);
}

wxGridCellRenderer *ButtonCellRenderer::Clone() const {
  return new ButtonCellRenderer;
}

class ButtonCellEditor : public wxGridCellEditor {
public:
  virtual void Create(wxWindow *parent, wxWindowID id,
                      wxEvtHandler *evtHandler);
  virtual void SetSize(const wxRect &rect);
  virtual void BeginEdit(int row, int col, wxGrid *grid);
  virtual bool EndEdit(int row, int col, const wxGrid *grid,
                       const wxString &oldval, wxString *newval);
  virtual void ApplyEdit(int row, int col, wxGrid *grid);
  virtual void Reset();
  virtual wxGridCellEditor *Clone() const;
  virtual wxString GetValue() const;

  virtual void Show(bool show, wxGridCellAttr *attr = NULL) override;
};

void ButtonCellEditor::Create(wxWindow *parent, wxWindowID id,
                              wxEvtHandler *evtHandler) {
  m_control = new CustomButton(parent, id, wxT("Press"));
  wxGridCellEditor::Create(parent, id, evtHandler);
}

void ButtonCellEditor::SetSize(const wxRect &rect) { m_control->SetSize(rect); }

void ButtonCellEditor::BeginEdit(int row, int col, wxGrid *grid) {
  // No edit needed for a button.
  CustomButton *button = static_cast<CustomButton *>(m_control);
  button->m_grid = grid;
}

bool ButtonCellEditor::EndEdit(int row, int col, const wxGrid *grid,
                               const wxString &oldval, wxString *newval) {
  // No edit needed for a button.
  return false;
}

void ButtonCellEditor::ApplyEdit(int row, int col, wxGrid *grid) {
  // No edit needed for a button.
}

void ButtonCellEditor::Reset() {
  // No edit needed for a button.
}
void ButtonCellEditor::Show(bool show, wxGridCellAttr *attr) {
  wxGridCellEditor::Show(show, attr);
  if (show) {
    m_control->SetFocus();
  }
}

wxGridCellEditor *ButtonCellEditor::Clone() const {
  return new ButtonCellEditor;
}

wxString ButtonCellEditor::GetValue() const { return wxEmptyString; }

// TODO:
// use the one from mainframe.cpp
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

void ResultWindow::fetchResult(std::string fetchURL) {
  CURL *curl = curl_easy_init();
  CURLcode res;
  std::string readBuffer;

  curl_global_init(CURL_GLOBAL_DEFAULT);

  curl = curl_easy_init();
  if (curl) {
    curl_easy_setopt(curl, CURLOPT_URL, fetchURL.c_str());
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
            this->grid->CreateGrid(jsonResponse.size(), 5);
            int row = 0;
            for (const auto &station : jsonResponse) {
              std::string name = station["name"].asString();
              std::string address = station["location"]["address"].asString();
              std::string open = station["openingHours"][0]["from"].asString() +
                                 "-" +
                                 station["openingHours"][0]["to"].asString();
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
              this->grid->SetCellValue(row, 4, "button");
              this->grid->SetCellRenderer(row, 4, new ButtonCellRenderer());
              this->grid->SetCellEditor(row, 4, new ButtonCellEditor());

              row++;
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
  // change the text size of grid
  wxFont font(16, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL,
              wxFONTWEIGHT_NORMAL);
  this->grid->SetDefaultCellFont(font);
  // Set column labels
  this->grid->SetColLabelValue(0, "Name");
  this->grid->SetColLabelValue(1, "Address");
  this->grid->SetColLabelValue(2, "Open");
  this->grid->SetColLabelValue(3, "Price");
  this->grid->SetColLabelValue(4, "Save");
  this->grid->SetColSize(0, 300);     // Increase size for "Name" column
  this->grid->SetColSize(1, 250);     // Increase size for "Name" column
  this->grid->SetColSize(2, 250 / 2); // Halve size for "Open" column
  this->grid->SetColSize(3, 250 / 2); // Halve size for "Price" column
  this->grid->SetColSize(4, 100);     // Halve size for "Price" column
}

ResultWindow::ResultWindow(wxWindow *parent, const wxString &title,
                           const wxChoice &fuelsDropDown,
                           wxCheckBox &nowOpenBox, wxString &regionCode)
    : wxFrame(parent, wxID_ANY, title, wxDefaultPosition, wxDefaultSize,
              wxDEFAULT_FRAME_STYLE & ~(wxRESIZE_BORDER | wxMAXIMIZE_BOX)) {

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
  std::string fetchURL =
      "https://api.e-control.at/sprit/1.0/search/gas-stations/by-region?code=";
  fetchURL += regionCode.c_str() +
              "&type=BL&fuelType=" + fuelType.ToStdString() +
              "&includeClosed=" + includeClosed;
  std::cout << fetchURL << std::endl;

  SetClientSize(1000, 1250);
  Center();
  this->grid = new wxGrid(this, -1, wxPoint(0, 415), wxSize(1200, 800));
  grid->SetDefaultRowSize(100, 50);
  grid->SetDefaultColSize(230, 100);

  fetchResult(fetchURL);
}
